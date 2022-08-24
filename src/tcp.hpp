#ifndef _NET_TCP_HPP_
#define _NET_TCP_HPP_

#include <ostream>
#include "socket.hpp"

namespace net::tcp {

    class server {
    friend class connection;
    public:
        server(socket_address addr, int backlog) : sock_{addr} {
            sock_.bind();
            sock_.listen(backlog);
        }

    private:
        net::socket sock_;
    };

    class connection {
    public:
        connection(server const & server) : sock_{server.sock_.accept()} {}
        connection(socket_address addr) : sock_{addr} { sock_.connect(); }
        connection(addrinfo_list info) : sock_{connect_to_first(info.get())} {}
        connection(addrinfo const * info) : sock_{connect_to_first(info)} {}
        auto & operator<<(auto && msg) const { return sendall(msg.data(), msg.size()), *this; }
        auto & operator<<(char const * msg) const { return *this << std::string_view{msg}; }
        auto send(char const * buf, auto len) const { return sock_.send(buf, len); }
        template<std::size_t N> auto send(char const (&buf)[N]) const { return send(buf, N); }
        void sendall(char const * buf, auto len) const { for(auto end = buf + len; len;) len -= send(end - len, len); }
        template<std::size_t N> void sendall(char const (&buf)[N]) const { sendall(buf, N); }
        auto recv(char * buf, auto len) const { return sock_.recv(buf, len); }
        template<std::size_t N> auto recv(char (&buf)[N]) const { return recv(buf, N); }
    private:
        net::socket sock_;
    };

}

#endif // _NET_TCP_HPP_
