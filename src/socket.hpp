#ifndef _NET_SOCKET_HPP_
#define _NET_SOCKET_HPP_

#include <bit>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>
#ifdef _WIN32
// #include <winsock2.h>
// #include <Ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h> // getaddrinfo() & freeaddrinfo()
#include <arpa/inet.h>
#include <unistd.h> // close()
#endif

namespace net {

    inline constexpr int ANY = AF_UNSPEC, IP4 = AF_INET, IP6 = AF_INET6, STREAM = SOCK_STREAM, DATAGRAM = SOCK_DGRAM;

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
        if(getaddrinfo(name, service, &hints, &res))
            throw std::runtime_error{"getaddrinfo() failed"};
        return {res, {}};
    }

#ifdef _WIN32
    using sd_t = SOCKET;
    using len_t = int;
    inline constexpr sd_t invalid = INVALID_SOCKET;
    inline constexpr int error = SOCKET_ERROR;
    inline constexpr auto close_socket = closesocket;
#else
    using sd_t = int;
    using len_t = socklen_t;
    inline constexpr sd_t invalid = -1;
    inline constexpr int error = -1;
    inline constexpr auto close_socket = close;
#endif

    class socket {
    private:
        socket() = default;
    public:
        socket(void * addr, len_t addr_size, int socktype = STREAM, int protocol = 0) : addr_size_{addr_size} {
            std::memcpy(&addr_, addr, addr_size_);
            sock_ = ::socket(addr_.ss_family, socktype, protocol);
            if(sock_ == invalid)
                throw std::runtime_error{"socket() failed"};
        }
        ~socket() { close_socket(sock_); }
        void close() { if(close_socket(sock_)) throw std::runtime_error{"closing failed"}; }
        void bind() { if(::bind(sock_, reinterpret_cast<sockaddr const *>(&addr_), addr_size_)) throw std::runtime_error{"bind() failed"}; }
        void listen(int backlog) { if(::listen(sock_, backlog)) throw std::runtime_error{"listen() failed"}; }
        socket accept() {
            socket s;
            s.addr_size_ = sizeof(sockaddr_storage);
            s.sock_ = ::accept(sock_, reinterpret_cast<sockaddr *>(&s.addr_), &s.addr_size_);
            if(s.sock_ == invalid)
                throw std::runtime_error{"accept() failed"};
            return s;
        }
        void connect() { if(::connect(sock_, reinterpret_cast<sockaddr const *>(&addr_), addr_size_)) throw std::runtime_error{"connect() failed"}; }
        auto send(const void * msg, int len, int flags = 0) {
            if(auto sent = ::send(sock_, msg, len, flags); sent != error)
                return sent;
            throw std::runtime_error{"send() failed"};
        }

        auto recv(void * buf, int len, int flags = 0) {
            if(auto received = ::recv(sock_, buf, len, flags); received != error)
                return received;
            throw std::runtime_error{"recv() failed"};
        }

    private:
        sockaddr_storage addr_;
        len_t addr_size_;
        sd_t sock_;
    };

    class context {
#ifdef _WIN32
    public:
        context() {
            using std::literals;
            // https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
            if(WSAStartup(MAKEWORD(1, 1), &wsa_data_))
                throw std::runtime_error{"WSAStartup() failed"};
        }

        ~context() {
            WSACleanup(); // 0 or SOCKET_ERROR + WSAGetLastError()
        }

    private:
        WSADATA wsa_data_;
#endif
    };

};

#endif // _NET_SOCKET_HPP_
