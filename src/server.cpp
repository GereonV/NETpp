#include <ctime>
#include <iostream>
#include <string>
#include "socket.hpp"

std::string daytime() noexcept {
	std::time_t now = time(nullptr);
	return std::ctime(&now);
}

int main() {
}

