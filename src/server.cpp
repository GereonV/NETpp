#include <ctime>
#include <iostream>
#include <string>
#include "socket.hpp"

std::string daytime() noexcept {
	std::time_t now = time(nullptr);
	return std::ctime(&now);
}

int main() {
	try {
		net::context c;
		auto ep = net::endpoints(nullptr, "daytime", net::STREAM);
		net::socket sock{ep->ai_addr, ep->ai_addrlen, ep->ai_socktype, ep->ai_protocol};
		sock.bind();
		sock.listen(10);
		for(;;) {
			auto conn = sock.accept();
			auto str = daytime();
			conn.send(str.data(), str.size());
		}
	} catch(std::exception & e) {
		std::cerr << e.what() << '\n';
		return -1;
	}
}

