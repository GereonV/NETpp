#include <iostream>
#include "udp.hpp"

int main() try {
	NET_INIT()
	net::udp::server server{net::endpoint(0, 3636)};
	char buf[256];
	for(;;) {
		std::cout.write(buf, server.recv(buf)).flush();
		server << "HELLO! :)\n";
	}
} catch(std::exception & e) {
	std::cerr << e.what() << '\n';
	return -1;
}
