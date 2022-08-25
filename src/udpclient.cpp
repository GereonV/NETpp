#include <iostream>
#include "udp.hpp"

int main(int argc, char const ** argv) {
	try {
		if(argc < 3)
			throw std::invalid_argument{"Provide name and port"};
		net::context c;
		net::udp::connection conn{*net::endpoints(argv[1], argv[2])};
		conn << "LOL";
        char buf[256];
        std::cout.write(buf, conn.recv(buf));
	} catch(std::exception & e) {
		std::cerr << e.what() << '\n';
		return -1;
	}
}

