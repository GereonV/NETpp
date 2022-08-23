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
        connection(addrinfo_list info) : connection{info.get()} {}
        connection(addrinfo const * info) : sock_{info->ai_addr, info->ai_addrlen} {
            auto family = info->ai_addr->sa_family;
            for(info = info->ai_next; info; info = info->ai_next) {
                try {
                    sock_.connect();
                    return;
                } catch(std::runtime_error & e) {
                    sock_.reset(info->ai_addr, info->ai_addrlen);
                    auto new_fam = info->ai_addr->sa_family;
                    if(new_fam == family)
                        continue;
                    sock_.close();
                    sock_.resock();
                    family = new_fam;
                }
            }
            sock_.connect();
        }

        auto & operator<<(auto && msg) const { return send(msg.data(), msg.size()), *this; }
        auto & operator<<(char const * msg) const { return *this << std::string_view{msg}; }
        template<std::size_t N> void send(char const (&buf)[N]) const { send(buf, N); }
        void send(char const * buf, auto len) const {
            for(auto end = buf + len; len;)
                len -= sock_.send(end - len, len);
        }

        template<std::size_t N> auto recv(char (&buf)[N]) const { return recv(buf, N); }
        auto recv(char * buf, auto len) const { return sock_.recv(buf, len); }
    private:
        net::socket sock_;
    };

}

#endif // _NET_TCP_HPP_
