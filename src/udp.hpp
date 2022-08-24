#ifndef _NET_UDP_HPP_
#define _NET_UDP_HPP_

#include "socket.hpp"

namespace net::udp {

    class connection : public socket_io<connection> {
    friend socket_io<connection>;
    public:
        connection(socket_address addr) : sock_{addr, DATAGRAM} { sock_.connect(); }
    private:
        net::socket sock_;
    };

}

#endif // _NET_UDP_HPP_
