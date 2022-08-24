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

    class connection : public socket_io<connection> {
    friend socket_io<connection>;
    public:
        connection(server const & server) : sock_{server.sock_.accept()} {}
        connection(socket_address addr) : sock_{addr} { sock_.connect(); }
        connection(addrinfo const * info) : sock_{*info} {
            auto fam = sock_.family();
            while((info = info->ai_next)) {
                try {
                    sock_.connect();
                    return;
                } catch(std::runtime_error & e) {
                    sock_.reset(*info);
                    auto new_fam = sock_.family();
                    if(new_fam == fam)
                        continue;
                    sock_.close();
                    sock_.resock();
                    fam = new_fam;
                }
            }
            sock_.connect();
        }

    private:
        net::socket sock_;
    };

}

#endif // _NET_TCP_HPP_
