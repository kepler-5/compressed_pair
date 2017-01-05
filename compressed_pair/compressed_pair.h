#ifndef compressed_pair_h
#define compressed_pair_h

#include <type_traits>
#include <functional>

namespace detail {
	template<typename T, unsigned N, typename Enable = void>
	struct compressed_pair_member;
	
	template<typename T, unsigned N>
	struct compressed_pair_member<T, N, typename std::enable_if_t<std::is_empty<T>::value>> : T {
		using type = T;
		T& get() { return *this; }
		const T& get() const { return *this; }
		
		template<typename Tp = type>
		void set(Tp&& t) {}
	};
	
	template<typename T, unsigned N>
	struct compressed_pair_member<T, N, typename std::enable_if_t<!std::is_empty<T>::value>> {
		using type = T;
		T member;
		T& get() { return member; }
		const T& get() const { return member; }
		
		template<typename Tp = type>
		void set(Tp&& t) {
			member = type((Tp&&)t);
		}
	};
}

template<typename T1, typename T2>
class compressed_pair
: detail::compressed_pair_member<T1, 0>, detail::compressed_pair_member<T2, 1> {
private:
	using first_member_type = detail::compressed_pair_member<T1, 0>;
	using second_member_type = detail::compressed_pair_member<T2, 1>;
	struct nat{};
public:
	using first_type = T1;
	using second_type = T2;
	using first_reference = T1&;
	using second_reference = T2&;
	using first_const_reference = const T1&;
	using second_const_reference = const T2&;
	
	// POD types are not zero-initialized
	compressed_pair() {}
	
	template<typename F = T1, typename S = T2>
	compressed_pair(F&& x, S&& y) {
		first_member_type::set((F&&)x);
		second_member_type::set((S&&)y);
	}
	
	template<typename F = T1, typename S = T2>
	explicit compressed_pair(T1&& x, std::enable_if_t<!std::is_same<std::remove_cv_t<F>, std::remove_cv_t<S>>::value, nat> = nat{}) {
		first_member_type::set((T1&&)x);
	}
	template<typename F = T1, typename S = T2>
	explicit compressed_pair(T2&& y, std::enable_if_t<!std::is_same<std::remove_cv_t<F>, std::remove_cv_t<S>>::value, const nat&> = nat{}) {
		second_member_type::set((T2&&)y);
	}
	template<typename F = T1, typename S = T2>
	explicit compressed_pair(T1&& x, std::enable_if_t<std::is_same<std::remove_cv_t<F>, std::remove_cv_t<S>>::value, nat> = nat{}) {
		first_member_type::set((T1&&)x);
		second_member_type::set((T1&&)(x));
	}
	
	compressed_pair(const compressed_pair& other) {
		first_member_type::set(other.first());
		second_member_type::set(other.second());
	}
	compressed_pair& operator=(const compressed_pair& other) {
		first_member_type::set(other.first());
		second_member_type::set(other.second());
		return *this;
	}
	compressed_pair(compressed_pair&& other) {
		first_member_type::set(std::move(other.first()));
		second_member_type::set(std::move(other.second()));
	}
	compressed_pair& operator=(compressed_pair&& other) {
		first_member_type::set(std::move(other.first()));
		second_member_type::set(std::move(other.second()));
		return *this;
	}
	
	first_reference first() { return first_member_type::get(); }
	first_const_reference first() const { return first_member_type::get(); }
	
	second_reference second() { return second_member_type::get(); }
	second_const_reference second() const { return second_member_type::get(); }
};

#endif
