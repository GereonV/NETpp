#ifndef _NET_SOCKET_HPP_
#define _NET_SOCKET_HPP_

#include <bit>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h> // close()
#endif

namespace net {

    inline constexpr int ANY = AF_UNSPEC,
                         IP4 = AF_INET,
                         IP6 = AF_INET6,
                         STREAM = SOCK_STREAM,
                         DATAGRAM = SOCK_DGRAM;

    class context {
    public:
#ifdef _WIN32
    using socket_t = SOCKET;
    using len_t = int;
        context() { if(WSAStartup(MAKEWORD(1, 1), &wsa_data_)) throw error("WSAStartup"); }
        ~context() { WSACleanup(); }
        static constexpr socket_t invalid() noexcept { return INVALID_SOCKET; }
        static void close(socket_t sock) { if(closesocket(sock)) throw error("closesocket"); }
        static std::runtime_error error(char const * fun, std::string err = std::to_string(GetLastError())) {
            static std::string s{"() failed ("};
            return std::runtime_error{fun + s + err + ')'};
        }

        static auto gai_error(int ecode) { return error("getaddrinfo", std::to_string(ecode)); }
    private:
        WSADATA wsa_data_;
#else
    using socket_t = int;
    using len_t = socklen_t;
        static constexpr socket_t invalid() noexcept { return -1; }
        static void close(socket_t sock) { if(::close(sock)) throw error("close"); }
        static std::runtime_error error(char const * fun, std::string err = strerror(errno)) {
            static std::string s{"() failed: "};
            return std::runtime_error{fun + s + err};
        }

        static auto gai_error(int ecode) { return error("getaddrinfo", gai_strerror(ecode)); }
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
            .sin_family = AF_INET,
            .sin_port = endian(port),
            .sin_addr = {endian(ip_addr)},
            .sin_zero = {}
        };
    }

    std::unique_ptr<addrinfo, decltype([](addrinfo * ai) { freeaddrinfo(ai); })> endpoints(char const * name, char const * service, int family = ANY, int socktype = STREAM) {
        addrinfo hints{
            .ai_flags = AI_PASSIVE, // loopback address if !name
            .ai_family = family,
            .ai_socktype = socktype,
            .ai_protocol = 0, // automatic based on type
            .ai_addrlen = 0,
            .ai_addr = nullptr,
            .ai_canonname = nullptr,
            .ai_next = nullptr
        }, * res;
        if(auto ecode = getaddrinfo(name, service, &hints, &res); ecode)
            throw context::gai_error(ecode);
        return {res, {}};
    }

    class socket {
    private:
        socket() = default;
    public:
        socket(void * addr, context::len_t addr_size, int socktype = STREAM, int protocol = 0)
            : addr_size_{addr_size} {
            std::memcpy(&addr_, addr, addr_size_);
            sock_ = ::socket(addr_.ss_family, socktype, protocol);
            if(sock_ == context::invalid())
                throw context::error("socket");
        }

        operator sockaddr *() noexcept { return reinterpret_cast<sockaddr *>(&addr_); }
        ~socket() { try { context::close(sock_); } catch(std::exception & e) {} }
        void close() { context::close(sock_); }
        void bind() { if(::bind(sock_, *this, addr_size_)) throw context::error("bind"); }
        void listen(int backlog) { if(::listen(sock_, backlog)) throw context::error("listen"); }
        socket accept() {
            socket s;
            s.addr_size_ = sizeof(sockaddr_storage);
            s.sock_ = ::accept(sock_, s, &s.addr_size_);
            if(s.sock_ == context::invalid())
                throw context::error("accept");
            return s;
        }

        void connect() { if(::connect(sock_, *this, addr_size_)) throw context::error("connect"); }
        auto send(const void * msg, int len, int flags = 0) {
            if(auto sent = ::send(sock_, msg, len, flags); sent != -1)
                return sent;
            throw context::error("send");
        }

        auto recv(void * buf, int len, int flags = 0) {
            if(auto received = ::recv(sock_, buf, len, flags); received != -1)
                return received;
            throw context::error("recv");
        }

    private:
        sockaddr_storage addr_;
        context::len_t addr_size_;
        context::socket_t sock_;
    };

};

#endif // _NET_SOCKET_HPP_