#include <iostream>
#include "socket.hpp"

int main(int argc, char const ** argv) {
	try {
		net::context c;
		auto ep = net::endpoints(nullptr, "daytime");
		net::socket sock{ep->ai_addr, ep->ai_addrlen, ep->ai_socktype, ep->ai_protocol};
		sock.connect();
		char buf[256];
		auto size = sock.recv(buf, 256);
		std::cout.write(buf, size);
	} catch(std::exception & e) {
		std::cerr << e.what() << '\n';
		return -1;
	}
}
