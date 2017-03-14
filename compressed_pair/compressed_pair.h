#ifndef compressed_pair_h
#define compressed_pair_h

#include <type_traits>

namespace detail {
	template<typename T>
	constexpr bool is_EBO_eligible = std::is_empty<T>::value && !std::is_final<T>::value;
	
	template<typename T, unsigned N, typename Enable = void>
	struct compressed_pair_member;
	
	template<typename T, unsigned N>
	struct compressed_pair_member<T, N, typename std::enable_if_t<is_EBO_eligible<T>>> : T {
		using type = T;
		T& get() { return *this; }
		const T& get() const { return *this; }
		
		compressed_pair_member() = default;
		template<typename Tp = type>
		compressed_pair_member(Tp&& t) : T{(Tp&&)t} {}
		
		template<typename Tp = type>
		void set(Tp&& t) {}
	};
	
	template<typename T, unsigned N>
	struct compressed_pair_member<T, N, typename std::enable_if_t<!is_EBO_eligible<T>>> {
		using type = T;
		T member;
		T& get() { return member; }
		const T& get() const { return member; }
		
		compressed_pair_member() = default;
		template<typename Tp = type>
		compressed_pair_member(Tp&& t) : member((Tp&&)t) {}
		
		template<typename Tp = type>
		void set(Tp&& t) {
			member = type((Tp&&)t);
		}
	};
	
	struct cp_nat {};
}

template<typename T1, typename T2>
class compressed_pair
: detail::compressed_pair_member<T1, 0>, detail::compressed_pair_member<T2, 1> {
private:
	using first_member_type = detail::compressed_pair_member<T1, 0>;
	using second_member_type = detail::compressed_pair_member<T2, 1>;
	
	template<typename F, typename S>
	using is_same_as_this_t = std::enable_if_t<std::is_same<F, T1>::value && std::is_same<S, T2>::value>;
public:
	using first_type = T1;
	using second_type = T2;
	using first_reference = T1&;
	using second_reference = T2&;
	using first_rvalue_reference = T1&&;
	using second_rvalue_reference = T2&&;
	using first_const_reference = const T1&;
	using second_const_reference = const T2&;
	
	// POD types are not zero-initialized
	template<typename F = first_member_type, typename S = second_member_type,
		typename = std::enable_if_t<
			std::is_default_constructible<F>::value &&
			std::is_default_constructible<S>::value
		>
	>
	compressed_pair() {}
	
	template<typename F = T1, typename S = T2>
	compressed_pair(F&& x, S&& y)
	: first_member_type{(F&&)x}, second_member_type{(S&&)y} {}
	
	template<typename F = T1, typename S = T2>
	explicit compressed_pair(F&& x, std::enable_if_t<
							 !std::is_same<std::remove_cv_t<F>, std::remove_cv_t<S>>::value &&
							 std::is_constructible<T1, F&&>::value &&
							 !std::is_constructible<T2, F&&>::value, detail::cp_nat> = detail::cp_nat{})
	: first_member_type{(F&&)x} {}
	
	template<typename F = T1, typename S = T2>
	explicit compressed_pair(S&& y, std::enable_if_t<
							 !std::is_same<std::remove_cv_t<F>, std::remove_cv_t<S>>::value &&
							 !std::is_constructible<T1, S&&>::value &&
							 std::is_constructible<T2, S&&>::value, const detail::cp_nat&> = detail::cp_nat{})
	: second_member_type{(S&&)y} {}
	
	template<typename F = T1, typename S = T2>
	explicit compressed_pair(F&& x, std::enable_if_t<
							 std::is_same<std::remove_cv_t<F>, std::remove_cv_t<S>>::value &&
							 std::is_constructible<T1, F&&>::value, detail::cp_nat> = detail::cp_nat{})
	: first_member_type{x}, second_member_type{(F&&)x} {}
	
	first_reference first() & { return first_member_type::get(); }
	first_rvalue_reference first() && { return std::move(first_member_type::get()); }
	first_const_reference first() const& { return first_member_type::get(); }
	
	second_reference second() & { return second_member_type::get(); }
	second_rvalue_reference second() && { return std::move(second_member_type::get()); }
	second_const_reference second() const& { return second_member_type::get(); }
	
	////////////////
	std::declval<<#class _Tp#>>()
#define CP_OPERATOR_RETURNS(...) -> decltype(__VA_ARGS__) {return __VA_ARGS__;}
	
	template<typename F = T1, typename S = T2, typename = is_same_as_this_t<F, S>>
	constexpr auto operator==(const compressed_pair<F, S>& rhs) CP_OPERATOR_RETURNS(first() == rhs.first() && second() == rhs.second())
	template<typename F = T1, typename S = T2, typename = is_same_as_this_t<F, S>>
	constexpr auto operator!=(const compressed_pair<F, S>& rhs) CP_OPERATOR_RETURNS(!(*this == rhs))
	
#undef CP_OPERATOR_RETURNS
};

#endif
