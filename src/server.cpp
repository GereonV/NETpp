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
		auto ep = net::endpoint(net::ip4("127.0.0.1"), 13);
		net::tcp::server server{&ep, sizeof ep, 10};
		for(;;) {
			net::tcp::connection serv{server};
			serv << daytime();
		}
	} catch(std::exception & e) {
		std::cerr << e.what() << '\n';
		return -1;
	}
}

