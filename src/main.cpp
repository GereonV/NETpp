#include <iostream>
#include <asio.hpp>

int main(int argc, char const ** argv) {
	if(argc < 2)
		return -1;
	try {
		asio::io_context io;
		asio::ip::tcp::socket sock{io};
		asio::connect(sock, asio::ip::tcp::resolver{io}.resolve(argv[1], "daytime"));
		char buf[256];
		std::size_t len{};
		asio::error_code err;
		while(!err) {
			std::cout.write(buf, len);
			len = sock.read_some(asio::buffer(buf), err);
		}
		if(err != asio::error::eof)
			throw asio::system_error{err};
	} catch(std::exception & e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}
}
