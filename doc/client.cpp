#include <iostream>
#include "tcp.hpp"

int main(int argc, char const ** argv) try {
	if(argc < 3)
		throw std::invalid_argument{"Provide name and port"};
	NET_INIT()
	net::tcp::connection conn{net::endpoints(argv[1], argv[2]).get()};
	char buf[4096];
	for(auto rcv = conn.recv(buf); rcv; rcv = conn.recv(buf))
		std::cout.write(buf, rcv);
} catch(std::exception & e) {
	std::cerr << e.what() << '\n';
	return -1;
}
