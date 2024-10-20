#include <iostream>
#include "maybe.hpp"

int main() {
	Maybe<int, std::string> result1 = 42;
	Maybe<int, std::string> result2 = unexpected("Error occurred");

	if (result1) {
		std::cout << "Result 1: " << result1.value() << std::endl;
	}

	if (!result2) {
		std::cout << "Result 2 error: " << result2.error() << std::endl;
	}

	return 0;
}