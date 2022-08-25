#include <ctime>
#include <iostream>
#include <string>
#include "tcp.hpp"

std::string daytime() noexcept {
	std::time_t now = time(nullptr);
	return std::ctime(&now);
}

int main() {
	try {
		net::context c;
		net::tcp::server server{net::endpoint(0, 13), 10};
		for(;;) {
			net::tcp::connection serv{server};
			serv << daytime();
		}
	} catch(std::exception & e) {
		std::cerr << e.what() << '\n';
		return -1;
	}
}
