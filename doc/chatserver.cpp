#include <iostream>
#include <mutex>
#include <thread>
#include <string>
#include "tcp.hpp"

struct connection_block {
    net::tcp::connection conn;
    connection_block * next, * prev{};
};

int main() {
	try {
		net::context c;
		net::tcp::server server{net::endpoint(0, 42069), 10};
        connection_block * connections{};
        std::mutex conn_mutex;
        for(;;) {
            auto block = new connection_block{{server}, connections};
            std::scoped_lock create_lock{conn_mutex};
            if(connections)
                connections->prev = block;
            connections = block;
            std::thread{[block, &connections, &conn_mutex]() {
                char buf[4096];
                for(;;) {
                    auto rcv = block->conn.recv(buf);
                    if(!rcv)
                        break;
                    std::scoped_lock send_lock{conn_mutex};
                    for(auto ptr = connections; ptr; ptr = ptr->next)
                        if(ptr != block)
                            ptr->conn.sendall({buf, rcv});
                }
                std::scoped_lock delete_lock{conn_mutex};
                if(block->prev)
                    block->prev->next = block->next;
                if(block->next)
                    block->next->prev = block->prev;
                if(connections == block)
                    connections = block->next;
                delete block;
            }}.detach();
        }
	} catch(std::exception & e) {
		std::cerr << e.what() << '\n';
		return -1;
	}
}
