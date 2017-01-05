#include "compressed_pair.h"

#include <iostream>
#include <sstream>
#include <vector>

struct empty_type {};
struct empty_type2 {};
struct non_empty_type { empty_type e; };

struct copy_move_type {
	copy_move_type() {
		std::cout << "default constructor\n";
	}
	copy_move_type(const copy_move_type&) {
		std::cout << "copy constructor\n";
	}
	copy_move_type& operator=(const copy_move_type&) {
		std::cout << "copy assignment\n";
		return *this;
	}
	copy_move_type(copy_move_type&&) {
		std::cout << "move constructor\n";
	}
	copy_move_type& operator=(copy_move_type&&) {
		std::cout << "move assignment\n";
		return *this;
	}
	int non_empty;
};

std::ostream& operator <<(std::ostream& stream, const empty_type& e) {
	return stream << "[empty type]";
}

std::ostream& operator <<(std::ostream& stream, const copy_move_type& c) {
	return stream << "[copy move type]";
}

template<typename T, typename U>
std::string to_string(const compressed_pair<T, U>& pair) {
	std::stringstream stream;
	stream << pair.first() << ", " << pair.second();
	return stream.str();
}

#define RUNTIME_ASSERT(condition) do { if (!(condition)) throw; } while ( false )

void tests() {
	// sanity checks
	static_assert(std::is_empty<empty_type>{} && std::is_empty<empty_type2>{}, "");
	static_assert(!std::is_empty<int>{}, "");
	static_assert(!std::is_empty<non_empty_type>{}, "");
	static_assert(sizeof(non_empty_type) == 1, "");
	
	// size
	static_assert(sizeof(compressed_pair<int, int>) == 2*sizeof(int), "");
	static_assert(sizeof(compressed_pair<int, empty_type>) == sizeof(int), "");
	static_assert(sizeof(compressed_pair<empty_type, int>) == sizeof(int), "");
	static_assert(sizeof(compressed_pair<empty_type, empty_type2>) == 1, "");
	static_assert(sizeof(compressed_pair<empty_type, empty_type>) == sizeof(compressed_pair<empty_type2, empty_type2>), "");
	
	{
		// two-argument constructor
		auto test = compressed_pair<double, double>{0.0, 1};
		RUNTIME_ASSERT(to_string(test) == "0, 1");
	}
	{
		// single-argument constructor, single type
		auto test = compressed_pair<char, char>{'x'};
		RUNTIME_ASSERT(to_string(test) == "x, x");
	}
	{
		auto test = compressed_pair<char, empty_type>{'x'};
		RUNTIME_ASSERT(to_string(test) == "x, [empty type]");
	}
	{
		auto test = compressed_pair<empty_type, unsigned>{0u};
		RUNTIME_ASSERT(to_string(test) == "[empty type], 0");
	}
	{
		// empty type
		auto test = compressed_pair<int, empty_type>(2, empty_type{});
		RUNTIME_ASSERT(to_string(test) == "2, [empty type]");
	}
	{
		// default construction
		compressed_pair<empty_type, std::string> test;
		RUNTIME_ASSERT(to_string(test) == "[empty type], ");
	}
	{
		// copy and move
		copy_move_type testCMT;
		copy_move_type testCMT2;
		compressed_pair<copy_move_type, copy_move_type> test{testCMT, std::move(testCMT2)};
		RUNTIME_ASSERT(to_string(test) == "[copy move type], [copy move type]");
		
		auto copy = test;
		auto&& move = std::move(copy);
	}
}

int main(int argc, const char * argv[]) {
	tests();
    return 0;
}
