#ifndef _NET_UDP_HPP_
#define _NET_UDP_HPP_

#include "socket.hpp"

namespace net::udp {

    template<std::size_t N>
    class connection {
    public:
        connection(void * addr, context::len_t addr_size) : sock_{addr, addr_size, DATAGRAM} { sock_.connect(); }
        connection(void * addr, context::len_t addr_size, char * buf, std::size_t & len) : sock_{addr, addr_size, DATAGRAM} {
            sock_.bind();
            len = recvfrom(buf); // overwrites addr
            sock_.connect();
        }

        void send(char const * buf, auto len = N) const { while(sock_.send(buf, len) != len); }
        auto recvfrom(char * buf) { return sock_.recvfrom(buf, N); }
        auto recv(char * buf) const { return sock_.recv(buf, N); }
        void recv(char * buf, auto len) const {
            for(auto end = buf + len; len;)
                len -= sock_.recv(buf - len, len);
        }

        void reset(void * addr, context::len_t addr_size) {
            auto old_fam = sock_.family()
            sock_.reset(addr, addr_size);
            if(sock_.family() != old_fam) {
                sock_.close();
                sock_.resock(DATAGRAM);
            }
            sock_.connect();
        }

    private:
        net::socket sock_;
    };

}

#endif // _NET_UDP_HPP_
