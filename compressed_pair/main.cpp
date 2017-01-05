#include "compressed_pair.h"

#include <iostream>
#include <sstream>
#include <memory>

struct empty_type {};
struct empty_type2 {};
struct non_empty_type { empty_type e; };

struct copy_move_type {
	copy_move_type() : copies{0}, moves{0} {}
	copy_move_type(const copy_move_type&) { ++copies; }
	copy_move_type& operator=(const copy_move_type&) { ++copies; return *this; }
	copy_move_type(copy_move_type&&) { ++moves; }
	copy_move_type& operator=(copy_move_type&&) { ++moves; return *this; }
	unsigned copies, moves;
};

struct empty_non_default_constructible {
	empty_non_default_constructible() = delete;
	empty_non_default_constructible(int) {}
};

struct non_empty_non_default_constructible {
	non_empty_non_default_constructible() = delete;
	non_empty_non_default_constructible(int) {}
	int x;
};

struct empty_final final {};
struct non_empty_final final { empty_final e; };

std::ostream& operator<<(std::ostream& stream, const empty_type& e) {
	return stream << "[empty type]";
}

std::ostream& operator<<(std::ostream& stream, const copy_move_type& c) {
	return stream << "[copy move type]";
}

template<typename T, typename U>
std::string to_string(const compressed_pair<T, U>& pair) {
	std::stringstream stream;
	stream << pair.first() << ", " << pair.second();
	return stream.str();
}

#define RUNTIME_ASSERT(condition) do { if ( !(condition) ) throw; } while(false)

void tests() {
	// sanity checks
	static_assert(std::is_empty<empty_type>{} && std::is_empty<empty_type2>{} && std::is_empty<empty_non_default_constructible>{} && std::is_empty<empty_final>{}, "");
	static_assert(!std::is_empty<int>{} && !std::is_empty<non_empty_type>{} && !std::is_empty<non_empty_non_default_constructible>{} && !std::is_empty<non_empty_final>{}, "");
	
	// size
	static_assert(sizeof(compressed_pair<int, int>) == 2*sizeof(int), "");
	static_assert(sizeof(compressed_pair<int, empty_type>) == sizeof(int), "");
	static_assert(sizeof(compressed_pair<empty_type, int>) == sizeof(int), "");
	static_assert(sizeof(compressed_pair<empty_type, empty_type>) == 2, "");
	static_assert(sizeof(compressed_pair<empty_type, empty_type2>) == 1, "");
	static_assert(sizeof(compressed_pair<empty_type, empty_type>) == sizeof(compressed_pair<empty_type2, empty_type2>), "");
	
	// construction
	static_assert(!std::is_default_constructible<compressed_pair<non_empty_non_default_constructible, int>>::value, "");
	static_assert(!std::is_default_constructible<compressed_pair<int, empty_non_default_constructible>>::value, "");
	static_assert( std::is_default_constructible<compressed_pair<int, char>>::value, "");
	
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
		// single-argument constructor, first type
		auto test = compressed_pair<char, empty_type>{'x'};
		RUNTIME_ASSERT(to_string(test) == "x, [empty type]");
	}
	{
		// single-argument constructor, second type
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
		RUNTIME_ASSERT(to_string(move) == to_string(test));
	}
	{
		// crazy stuff
		compressed_pair<non_empty_non_default_constructible, std::string> t1{42};
		compressed_pair<empty_non_default_constructible, empty_type> t2{42};
		compressed_pair<non_empty_non_default_constructible, empty_non_default_constructible> t3{42, 42};
		compressed_pair<empty_final, int> t4;
		compressed_pair<empty_final, non_empty_final> t5;
		compressed_pair<int, std::unique_ptr<empty_non_default_constructible>> t6(std::make_unique<empty_non_default_constructible>(42));
		static_assert(!std::is_copy_constructible<compressed_pair<int, std::unique_ptr<empty_non_default_constructible>>>{}, "");
		auto t7 = std::move(t6);
	}
}

int main(int argc, const char * argv[]) {
	tests();
    return 0;
}
