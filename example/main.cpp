#include <iostream>
#include "maybe.hpp"

struct MyComplexType {
	int index;
	std::string message;

	// Note that aggregate initialisation is not possible when using Maybe.
	MyComplexType(int i, std::string m) : index(i), message(std::move(m)) {}
};

template<class T>
using StringMaybe = maybe::Maybe<T, std::string>;

int main() {
	StringMaybe<MyComplexType> maybe_value { 42, "Hello, World!" };
	StringMaybe<MyComplexType> maybe_error = maybe::unexpected("Error occurred");

	// Conversion to underlying value type is possible,
	// with compile-time checks that the value is valid.
	MyComplexType valid = maybe_value.value();

	MyComplexType error = maybe_error.value();

	return 0;
}
