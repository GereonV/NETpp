#include <ctime>
#include <iostream>
#include <string>
#include <asio.hpp>

std::string daytime() noexcept {
	std::time_t now = time(nullptr);
	return std::ctime(&now);
}

int main() {
	try {
		asio::io_context io;
		asio::ip::tcp::acceptor acc{io, asio::ip::tcp::endpoint{asio::ip::tcp::v4(), 13}};
		asio::error_code err;
		while(true) {
			asio::ip::tcp::socket sock{io};
			acc.accept(sock);
			auto message = daytime();
			asio::write(sock, asio::buffer(message), err);
		}
	} catch(std::exception & e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}
}

