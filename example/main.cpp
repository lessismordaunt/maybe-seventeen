#include <iostream>
#include "maybe.hpp"

struct MyComplexType {
	int index;
	std::string message;

	// Note that aggregate initialisation is not possible when using Maybe.
	MyComplexType(int i, std::string m) : index(i), message(std::move(m)) {}
};

int main() {
	using namespace maybe;
	
	Maybe<MyComplexType, std::string> result1 { 42, "Hello, World!" };
	Maybe<MyComplexType, std::string> result2 = maybe::unexpected("Error occurred");

	if (result1) {
		std::cout << "Result 1: " << result1.value().message << std::endl;
	}

	if (!result2) {
		std::cout << "Result 2 error: " << result2.error() << std::endl;
	}

	return 0;
}
