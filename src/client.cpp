#include <iostream>
#include "socket.hpp"

int main(int argc, char const ** argv) {
	try {
		if(argc < 2)
			throw std::invalid_argument{"No IP address provided"};
		net::context c;
		auto ep = net::endpoints(argv[1], "daytime");
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
