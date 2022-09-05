#ifndef _NET_UDP_HPP_
#define _NET_UDP_HPP_

#include "socket.hpp"

namespace net::udp {

    class server  {
    public:
        server(socket_address addr) : sock_{addr, DATAGRAM} { sock_.bind(); }
        constexpr auto pollfd(short events = poll::in) const noexcept { return sock_.pollfd(events); }
        void send(out_buffer buf) const { sock_.sendto(buf.buf, buf.len); }
        auto recv(in_buffer buf) { return sock_.recvfrom(buf.buf, buf.len); }
        auto & operator<<(std::string_view sv) const { return send(sv), *this; }
        constexpr socket_address peer() const { return sock_; }
        void connect() { sock_.connect(); }
        void connect(socket_address addr) {
            sock_.reset(addr);
            connect();
        }

    private:
        net::socket sock_;
    };

    class connection  {
    public:
        connection(socket_address addr) : sock_{addr, DATAGRAM} { sock_.connect(); }
        constexpr auto pollfd(short events = poll::in) const noexcept { return sock_.pollfd(events); }
        void send(out_buffer buf) const { sock_.send(buf.buf, buf.len); }
        auto recv(in_buffer buf) const { return sock_.recv(buf.buf, buf.len); }
        auto & operator<<(std::string_view sv) const { return send(sv), *this; }
    private:
        net::socket sock_;
    };

}

#endif // _NET_UDP_HPP_
