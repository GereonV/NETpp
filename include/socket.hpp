#ifndef _NET_SOCKET_HPP_
#define _NET_SOCKET_HPP_

#include <bit>
#include <cstdint>
#include <cstring>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h> // close()
#endif

namespace net {

    inline constexpr int ANY = AF_UNSPEC,
                         IP4 = AF_INET,
                         IP6 = AF_INET6,
                         STREAM = SOCK_STREAM,
                         DATAGRAM = SOCK_DGRAM;

    struct poll : private pollfd {
    friend class context;
        static constexpr auto in = POLLIN,
                              out = POLLOUT,
                              pri = POLLPRI,
                              err = POLLERR,
                              hup = POLLHUP,
                              nval = POLLNVAL;
        constexpr poll(auto sock, short events) : pollfd{sock, events, 0} {}
        constexpr void set(short events) noexcept { this->events = events; }
        constexpr short get() noexcept { return revents; }
    };

    class context {
    public:
#ifdef _WIN32
    #define NET_INIT() net::context _c;
    using socket_t = SOCKET;
    using len_t = int;
    using fam_t = ADDRESS_FAMILY;
        context() { if(WSAStartup(MAKEWORD(1, 1), &wsa_data_)) throw error("WSAStartup"); }
        ~context() { WSACleanup(); }
        static constexpr socket_t invalid() noexcept { return INVALID_SOCKET; }
        static void close(socket_t sock) { if(closesocket(sock)) throw error("closesocket"); }
        static std::runtime_error error(char const * fun, std::string err = std::to_string(GetLastError())) {
            static std::string s{"() failed ("};
            return std::runtime_error{fun + s + err + ')'};
        }

        static auto gai_error(int ecode) { return error("getaddrinfo", std::to_string(ecode)); }
        static constexpr in_addr addr(std::uint32_t addr) noexcept { return {{.S_addr = addr}}; }
        static int poll(std::span<poll> fds, int timeout) {
            if(auto num = WSAPoll(fds.data(), fds.size(), timeout); num != SOCKET_ERROR)
                return num;
            throw error("WSAPoll");
        }

    private:
        WSADATA wsa_data_;
#else
    #define NET_INIT()
    using socket_t = int;
    using len_t = std::size_t;
    using fam_t = std::uint16_t;
        static constexpr socket_t invalid() noexcept { return -1; }
        static void close(socket_t sock) { if(::close(sock)) throw error("close"); }
        static std::runtime_error error(char const * fun, std::string err = strerror(errno)) {
            static std::string s{"() failed: "};
            return std::runtime_error{fun + s + err};
        }

        static auto gai_error(int ecode) { return error("getaddrinfo", gai_strerror(ecode)); }
        static constexpr in_addr addr(std::uint32_t addr) noexcept { return {addr}; }
        static int poll(std::span<poll> fds, int timeout) {
            if(auto num = ::poll(fds.data(), fds.size(), timeout); num != -1)
                return num;
            throw error("poll");
        }
#endif
    };


    constexpr std::uint32_t ip4(char const * addr) noexcept {
        std::uint32_t ip{};
        for(;; ++addr) {
            unsigned char octet{};
            while(*addr && *addr != '.')
                octet = static_cast<unsigned char>(10 * octet + *addr++ - '0');
            ip = ip << 8 | octet;
            if(!*addr)
                break;
        }
        return ip;
    }

    constexpr std::uint32_t ip4(std::string_view addr) noexcept {
        std::uint32_t ip{};
        for(auto begin = addr.begin(), end = addr.end();; ++begin) {
            unsigned char octet{};
            while(begin != end && *begin != '.')
                octet = static_cast<unsigned char>(10 * octet + *begin++ - '0');
            ip = ip << 8 | octet;
            if(begin == end)
                break;
        }
        return ip;
    }

    template<typename T>
    constexpr T endian(T const & val) noexcept {
        if constexpr(std::endian::native == std::endian::big)
            return val;
        auto max_shift = 8 * sizeof(T) - 8;
        auto n = static_cast<T>(val >> max_shift & 0xFF);
        for(std::size_t shift{8}; shift <= max_shift; shift += 8)
            n |= (val >> (max_shift - shift) & 0xFF) << shift;
        return n;
    }

    constexpr sockaddr_in endpoint(std::uint32_t ip_addr, std::uint16_t port) noexcept {
        return {
            .sin_family = IP4,
            .sin_port = endian(port),
            .sin_addr = context::addr(endian(ip_addr)),
            .sin_zero = {}
        };
    }

    struct socket_address {
    public:
        constexpr socket_address(void const * addr, socklen_t addr_size) noexcept : addr{addr, addr_size}, soo{} {}
        constexpr socket_address(addrinfo const & info) noexcept : socket_address{info.ai_addr, static_cast<socklen_t>(info.ai_addrlen)} {}
        constexpr socket_address(sockaddr_in const & addr) noexcept : so{addr.sin_family, addr.sin_port, addr.sin_addr}, soo{true} {}
        socklen_t copy_to(sockaddr_storage & dst) const noexcept {
            if(soo) {
                std::memcpy(&dst, &so, sizeof(so));
                std::memset(reinterpret_cast<char *>(&dst) + sizeof(so), 0, 8);
                return sizeof(sockaddr_in);
            } else {
                std::memcpy(&dst, addr.data, addr.size);
                return addr.size;
            }
        }

    private:
        union {
            struct {
                void const * data;
                socklen_t size;
            } addr;
            struct {
                context::fam_t family;
                std::uint16_t port;
                in_addr addr;
            } so;
        };
        bool soo;
    };

    struct socket_properies {
        constexpr socket_properies(int socktype = STREAM, int protocol = 0) noexcept : socktype{socktype}, protocol{protocol} {}
        int socktype;
        int protocol;
    };

    using addrinfo_list = std::unique_ptr<addrinfo, decltype([](addrinfo * ai) { freeaddrinfo(ai); })>;

    inline addrinfo_list endpoints(char const * name, char const * service, socket_properies prop = {}, int family = ANY) {
        addrinfo hints, * res;
        hints.ai_flags = AI_PASSIVE; // loopback address if !name
        hints.ai_family = family;
        hints.ai_socktype = prop.socktype;
        hints.ai_protocol = prop.protocol;
        hints.ai_addrlen = 0;
        hints.ai_addr = nullptr;
        hints.ai_canonname = nullptr;
        hints.ai_next = nullptr;
        if(auto ecode = getaddrinfo(name, service, &hints, &res); ecode)
            throw context::gai_error(ecode);
        return {res, {}};
    }

    class socket {
    private:
        operator auto()       noexcept { return reinterpret_cast<sockaddr       *>(&addr_); }
        operator auto() const noexcept { return reinterpret_cast<sockaddr const *>(&addr_); }
        socket(socket const &) = delete;
        socket(context::socket_t listener) : addr_size_{sizeof(sockaddr_storage)} { if((sock_ = ::accept(listener, *this, &addr_size_)) == context::invalid()) throw context::error("accept"); }
    public:
        void reset(socket_address addr) { addr_size_ = addr.copy_to(addr_); }
        void resock(socket_properies prop = {}) { if((sock_ = ::socket(addr_.ss_family, prop.socktype, prop.protocol)) == context::invalid()) throw context::error("accept"); }
        socket(socket_address addr, socket_properies prop = {}) { reset(addr); resock(prop); }
        ~socket() { try { close(); } catch(std::exception &) {} }
        constexpr auto family() const noexcept { return addr_.ss_family; }
        constexpr operator socket_address() const noexcept { return {&addr_, addr_size_}; }
        void shutdown(int how = 2) { if(::shutdown(sock_, how)) throw context::error("shutdown"); }
        void close() { context::close(sock_); }
        void connect() { if(::connect(sock_, *this, addr_size_)) throw context::error("connect"); }
        void bind() { if(::bind(sock_, *this, addr_size_)) throw context::error("bind"); }
        void listen(int backlog) { if(::listen(sock_, backlog)) throw context::error("listen"); }
        socket accept() const { return sock_; }
        constexpr poll pollfd(short events) const noexcept { return {sock_, events}; }
        auto send(char const * msg, context::len_t len, int flags = 0) const {
            if(auto sent = ::send(sock_, msg, len, flags); sent != -1)
                return sent;
            throw context::error("send");
        }

        auto recv(char * buf, context::len_t len, int flags = 0) const {
            if(auto received = ::recv(sock_, buf, len, flags); received != -1)
                return received;
            throw context::error("recv");
        }

        auto sendto(char const * msg, context::len_t len, int flags = 0) const {
            if(auto sent = ::sendto(sock_, msg, len, flags, *this, addr_size_); sent != -1)
                return sent;
            throw context::error("sendto");
        }

        auto recvfrom(char * buf, context::len_t len, int flags = 0) {
            addr_size_ = sizeof(sockaddr_storage);
            if(auto received = ::recvfrom(sock_, buf, len, flags, *this, &addr_size_); received != -1)
                return received;
            throw context::error("recvfrom");
        }

    private:
        sockaddr_storage addr_;
        context::socket_t sock_;
        socklen_t addr_size_;
    };

    template<typename T>
    struct buffer {
        constexpr buffer(T * buf, auto len) noexcept : buf{buf}, len{static_cast<context::len_t>(len)} {}
        constexpr buffer(std::string_view sv) noexcept : buf{sv.data()}, len{static_cast<context::len_t>(sv.size())} {}
        template<std::size_t N> constexpr buffer(T (&buf)[N]) noexcept : buf{buf}, len{N} {}
        constexpr auto begin() const noexcept { return buf; }
        constexpr auto end() const noexcept { return buf + len; }
        T * buf;
        context::len_t len;
    };

    using in_buffer = buffer<char>;
    using out_buffer = buffer<char const>;

};

#endif // _NET_SOCKET_HPP_
