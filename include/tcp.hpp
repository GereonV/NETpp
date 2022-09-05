#ifndef _NET_TCP_HPP_
#define _NET_TCP_HPP_

#include "socket.hpp"

namespace net::tcp {

    class server {
    friend class connection;
    public:
        server(socket_address addr, int backlog) : sock_{addr} {
            sock_.bind();
            sock_.listen(backlog);
        }

        constexpr auto pollfd() const noexcept { return sock_.pollfd(poll::in); }
    private:
        net::socket sock_;
    };

    class connection {
    public:
        connection(server const & server) : sock_{server.sock_.accept()} {}
        connection(socket_address addr) : sock_{addr} { sock_.connect(); }
        connection(addrinfo const * info) : sock_{*info} {
            auto fam = sock_.family();
            while((info = info->ai_next)) {
                try {
                    sock_.connect();
                    return;
                } catch(std::runtime_error &) {
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

        ~connection() { try { sock_.shutdown(); } catch(std::exception &) {} }
        constexpr auto pollfd(short events = poll::in) const noexcept { return sock_.pollfd(events); }
        auto send(out_buffer buf) const { return sock_.send(buf.buf, buf.len); }
        auto recv(in_buffer buf) const { return sock_.recv(buf.buf, buf.len); }
        void sendall(out_buffer buf) const { for(auto end = buf.end(); buf.len;) buf.len -= send({end - buf.len, buf.len}); }
        void recvall(in_buffer buf) const { for(auto end = buf.end(); buf.len;) buf.len -= recv({end - buf.len, buf.len}); }
        auto & operator<<(std::string_view msg) const { return sendall(msg), *this; }
    private:
        net::socket sock_;
    };

}

#endif // _NET_TCP_HPP_
