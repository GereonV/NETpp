#ifndef _NET_TCP_HPP_
#define _NET_TCP_HPP_

#include <ostream>
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
        void send(char const * msg, auto len) const { for(auto end = msg + len; len;) len -= sock_.send(end - len, len); }
        template<std::size_t N> auto recv(char (&buf)[N]) const { return sock_.recv(buf, N); }

        template<std::size_t N>
        auto operator<<(char const (&msg)[N]) const {
            send(msg, N);
            return *this;
        }

        auto operator<<(auto msg) const {
            send(msg.data(), msg.size());
            return *this;
        }

        friend auto & operator<<(std::ostream & os, connection const & conn) {
            char buf[1024];
            for(auto received = conn.recv(buf); received; received = conn.recv(buf))
                os.write(buf, received);
            return os;
        }

        auto operator>>(std::string & str) const {
            str.clear();
            char buf[1024];
            for(auto received = recv(buf); received; received = recv(buf))
                str.append(buf, received);
            return *this;
        }

    private:
        net::socket sock_;
    };

}

#endif // _NET_TCP_HPP_
