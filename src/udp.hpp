#ifndef _NET_UDP_HPP_
#define _NET_UDP_HPP_

#include "socket.hpp"

namespace net::udp {

    class connection : public socket_io<connection> {
    friend socket_io<connection>;
    public:
        connection(socket_address addr) : sock_{addr, DATAGRAM} { sock_.connect(); }
        using socket_io<connection>::send, socket_io<connection>::recv;
        auto send(char const * buf, context::len_t len) const { return sock_.sendto(buf, len); }
        auto recv(char * buf, context::len_t len) { return sock_.recvfrom(buf, len); }
    private:
        net::socket sock_;
    };

}

#endif // _NET_UDP_HPP_
