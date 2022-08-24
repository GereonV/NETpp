#include <iostream>
#include "udp.hpp"

int main(int argc, char const ** argv) {
	try {
		if(argc < 3)
			throw std::invalid_argument{"Provide name and port"};
		net::context c;
		net::udp::connection conn{*net::endpoints(argv[1], argv[2])};
        conn.send("LOL", 3);
        char buf[256];
        auto len = conn.recv(buf);
        std::cout.write(buf, len);
	} catch(std::exception & e) {
		std::cerr << e.what() << '\n';
		return -1;
	}
}

