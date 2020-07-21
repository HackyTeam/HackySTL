#pragma once

// Requires C++ 17

#include "../Tuple/Tuple.hpp"

struct default_t {};
constexpr default_t default_v{};

template<typename S, typename F>
struct defaultcall_t;

template <typename F, typename Result, typename... Args>
struct defaultcall_t<Result(Args...), F>;

template <size_t Id, typename F, typename Result, typename... Args, typename T>
constexpr auto default_cast(defaultcall_t<Result(Args...), F> const&, T&& x);
template <size_t Id, typename F, typename Result, typename... Args>
constexpr auto default_cast(defaultcall_t<Result(Args...), F> const& c, default_t);

template <typename F, typename Result, typename... Args>
struct defaultcall_t<Result(Args...), F> {
  tuple<Args...> default_args;
  F func;
  constexpr defaultcall_t(F&& func, Args&&... args) : default_args(std::forward<Args>(args)...), func(std::forward<F>(func)) {}
  
  private:
  template <typename... U, size_t... Seq>
  constexpr Result operator()(index_sequence<Seq...>, U&&... args) const {
    return func(default_cast<Seq, F, Result, Args...>(*this, std::forward<U>(args))...);
  }

  public:
  template <typename... U>
  constexpr Result operator()(U&&... args) const {
    return operator()(index_sequence_for<Args...>(), std::forward<U>(args)...);
  }

};

template<typename S, typename F, typename... Args>
defaultcall_t<S, F> make_defaultcall(F&& func, Args&&... args) {
    return defaultcall_t<S, F>(std::forward<F>(func), std::forward<Args>(args)...);
}

template <size_t Id, typename F, typename Result, typename... Args, typename T>
constexpr auto default_cast(defaultcall_t<Result(Args...), F> const&, T&& x) {return std::forward<T>(x); }
template <size_t Id, typename F, typename Result, typename... Args>
constexpr auto default_cast(defaultcall_t<Result(Args...), F> const& c, default_t) {return c.default_args.template get<Id>(); }

// STOLEN <begin>
template< typename T >
struct as_function
    : public as_function< decltype(&T::operator()) >
{};

template< typename R, typename... ArgTypes >
struct as_function< R(ArgTypes...) > 
{
    using type = R(ArgTypes...);
};

template< typename R, typename... ArgTypes >
struct as_function< R(ArgTypes...) noexcept > 
{
    using type = R(ArgTypes...);
};

template< typename U, typename R, typename... ArgTypes >
struct as_function< R(U::*)(ArgTypes...) const > 
{
    using type = R(ArgTypes...);
};
// STOLEN <end>

template <typename F, typename... Args>
defaultcall_t(F&&, Args&&...)
  -> defaultcall_t<typename as_function<
      std::remove_pointer_t<
        std::remove_reference_t<F>
      >
    >::type, F>;
