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

        template<std::size_t N>
        auto operator<<(char const (&arr)[N]) {
            auto remaining = N;
            while(remaining)
                remaining -= sock_.send(arr + N - remaining, remaining);
            return *this;
        }

        template<std::size_t N>
        auto operator>>(char (&arr)[N]) {
            if(sock_.recv(arr, N) != N)
                throw std::runtime_error{"Not everything read"};
            return *this;
        }

    private:
        net::socket sock_;
    };

}

#endif