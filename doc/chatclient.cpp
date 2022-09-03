#include <iostream>
#include <mutex>
#include <thread>
#include <string>
#include "tcp.hpp"

int main(int argc, char const * const * argv) {
	try {
		if(argc < 2)
			throw std::invalid_argument{"Provide name"};
        NET_INIT()
        net::tcp::connection conn{net::endpoints(argv[1], "42069").get()};
        bool open{true};
        std::mutex mutex;
        std::thread read_thread{[&]() {
            char buf[4096];
            for(auto rcv = conn.recv(buf); rcv; rcv = conn.recv(buf))
                std::cout.write(buf, rcv).flush();
            std::scoped_lock open_lock{mutex};
            open = false;
        }};
        std::string in;
        while(std::scoped_lock open_lock{mutex}, open) {
            std::getline(std::cin, in);
            conn << (in + '\n');
        }
	} catch(std::exception & e) {
		std::cerr << e.what() << '\n';
		return -1;
	}
}
