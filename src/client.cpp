#include <iostream>
#include "tcp.hpp"

int main(int argc, char const ** argv) {
	try {
		if(argc < 2)
			throw std::invalid_argument{"No IP address provided"};
		net::context c;
		auto ep = net::endpoints(argv[1], "13", net::STREAM);
		char buf[256];
		std::cout.write(buf, net::tcp::connection{ep->ai_addr, ep->ai_addrlen}.recv(buf));
	} catch(std::exception & e) {
		std::cerr << e.what() << '\n';
		return -1;
	}
}
