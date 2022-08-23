#ifndef _NET_TCP_HPP_
#define _NET_TCP_HPP_

#include "socket.hpp"

namespace net::tcp {

    class server {
    friend class connection;
    public:
        server(void * addr, context::len_t addr_size, int backlog) : sock_{addr, addr_size} {
            sock_.bind();
            sock_.listen(backlog);
        }

    private:
        net::socket sock_;
    };

    class connection {
    public:
        connection(server const & server) : sock_{server.sock_.accept()} {}
        connection(void * addr, context::len_t addr_size) : sock_{addr, addr_size} { sock_.connect(); }
        void send(char const * msg, auto len) { for(auto end = msg + len; len;) len -= sock_.send(end - len, len); }
        template<std::size_t N> auto recv(char (&buf)[N]) { return sock_.recv(buf, N); }

        template<std::size_t N>
        auto operator<<(char const (&msg)[N]) {
            send(msg, N);
            return *this;
        }

        auto operator<<(auto msg) {
            send(msg.data(), msg.size());
            return *this;
        }

    private:
        net::socket sock_;
    };

}

#endif
