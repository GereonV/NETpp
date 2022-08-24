#include <iostream>
#include "udp.hpp"

int main() {
	try {
		net::context c;
		auto ep = net::endpoint(0, 3636);
		net::socket sock{{&ep, sizeof(ep)}, net::DATAGRAM};
		sock.bind();
		char buf[256];
		for(;;) {
            std::cout.write(buf, sock.recvfrom(buf, sizeof(buf)));
            std::cout << std::endl;
            sock.sendto("HELLO! :)", 9);
		}
	} catch(std::exception & e) {
		std::cerr << e.what() << '\n';
		return -1;
	}
}
