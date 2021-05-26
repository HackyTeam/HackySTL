#pragma once

#include "IntegerSequence.hpp"
#include "Limits.hpp"
#include "Concepts.hpp"
#include <cmath>

/// The implementations are NOT original, they are, in fact
/// A C++20 adaptation of a C++11 codebase thanks to the
/// compile-time math library: https://github.com/kthohr/gcem

namespace hsd
{
    namespace math
    {
        namespace constants
        {
            static constexpr f128 pi = 3.14159265358979323846264338327950288l;
            static constexpr f128 euler = 2.71828182845904523536028747135266249l;
            static constexpr f128 log_sqrt_pi = 0.91893853320467274178032973640561763l;
            static constexpr f128 ln_table[9] = {
                0.6931471805599453094172321214581765680755l,
                1.0986122886681096913952452369225257046475l,
                1.3862943611198906188344642429163531361510l,
                1.6094379124341003746007593332261876395256l,
                1.7917594692280550008124773583807022727230l,
                1.9459101490553133051053527434431797296371l,
                2.0794415416798359282516963643745297042265l,
                2.1972245773362193827904904738450514092950l,
                2.3025850929940456840179914546843642076011l
            };
        } // namespace constants
        
        template <typename T>
        concept Middleable = is_number<T>::value ||
            is_same<remove_cv_t<T>, T>::value ||
            negation<is_same<T, bool>>::value;

        template <Middleable T>
        constexpr auto midpoint(T a, T b);
        template <IsObject T>
        constexpr auto midpoint(T* a, T* b);
        template <IsNumber T>
        static constexpr auto abs(const T& value);
        template <IsNumber T>
        static constexpr auto atan(T value);
        template <IsNumber T>
        static constexpr auto sqrt(T value);

        namespace constexpr_math
        {
            template <IsNumber T>
            static constexpr auto factorial(usize size);
            template <IsFloat T>
            static constexpr auto floor(const T& value);
            template <IsNumber T>
            static constexpr auto exp(T value);
            template <IsNumber T>
            static constexpr auto log(T value);
            template <IsNumber T>
            static constexpr auto lgamma(const T& value);

            namespace ctmath_detail
            {
                template <IsNumber T>
                static constexpr auto sgn(const T& value)
                {
                    if(value != 0)
                    {
                        return value < 0 ? -1 : 1;
                    }
                    else
                    {
                        return 0;
                    }
                }

                template <IsFloat T>
                static constexpr bool sign_bit(const T& value)
                {
                    if constexpr(IsSame<T, f32>)
                    {
                        return bit_cast<u32>(value) >> 31;
                    }
                    else if constexpr(IsSame<T, f64>)
                    {
                        return bit_cast<u64>(value) >> 63;
                    }
                    else
                    {
                        struct upair
                        {
                            hsd::u64 first;
                            hsd::u64 second;
                        };

                        return bit_cast<upair>(value).second >> 15;
                    }
                }

                template <IsFloat T>
                static constexpr auto find_fraction(const T& value)
                {
                    if(math::abs(value - constexpr_math::floor(value)) >= static_cast<T>(0.5))
                    {
                        return value - constexpr_math::floor(value) - sgn(value);
                    }
                    else
                    {
                        return value - constexpr_math::floor(value);
                    }
                }

                template <IsFloat T>
                static constexpr auto find_whole(const T& value)
                {
                    if(math::abs(value - constexpr_math::floor(value)) >= static_cast<T>(0.5))
                    {
                        return static_cast<i64>(constexpr_math::floor(value) + sgn(value));
                    }
                    else
                    { 
                        return static_cast<i64>(constexpr_math::floor(value));
                    }
                }

                template < IsNumber T1, IsIntegral T2 >
                static constexpr T1 pow_int_compute_rec(
                    const T1& base, const T1& val, const T2& exp_term)
                {
                    if(exp_term > static_cast<T2>(1))
                    {
                        if((exp_term & 1U) != 0)
                        {
                            return pow_int_compute_rec(base * base, val * base, exp_term / 2);
                        }
                        else
                        {
                            return pow_int_compute_rec(base * base, val, exp_term / 2);
                        }
                    }
                    else
                    {
                        return exp_term == static_cast<T2>(1) ? val * base : val;
                    }         
                }

                template < IsNumber T1, IsIntegral T2 >
                static constexpr auto pow_int_sgn_check(const T1& base, const T2& exp_term)
                {
                    if(exp_term < 0)
                    {
                        return 1 / pow_int_compute_rec(base, static_cast<T1>(1), -exp_term);
                    }
                    else
                    {
                        return pow_int_compute_rec(base, static_cast<T1>(1), exp_term);
                    }
                }

                template < IsNumber T1, IsIntegral T2 >
                static constexpr auto pow_int_compute(const T1& base, const T2& exp_term)
                {
                    if(exp_term == 3)
                    {
                        return base * base * base;
                    }
                    else if(exp_term == 2)
                    {
                        return base * base;
                    }
                    else if(exp_term == 1)
                    {
                        return base;
                    }
                    else if(exp_term == 0)
                    {
                        return static_cast<T1>(1);
                    }
                    else if(exp_term == limits<T2>::min)
                    {
                        return static_cast<T1>(0);
                    }
                    else if(exp_term == limits<T2>::max)
                    {
                        return limits<T1>::infinity;
                    }
                    else 
                    {
                        return pow_int_sgn_check(base, exp_term);
                    }
                }

                template < IsNumber T1, IsIntegral T2 >
                static constexpr auto pow_int_type_check(const T1& base, const T2& exp_term)
                {
                    return pow_int_compute(base, exp_term);
                }

                template < IsNumber T1, IsFloat T2 >
                static constexpr auto pow_int_type_check(const T1& base, const T2& exp_term)
                {
                    if(limits<T2>::epsilon > mod(exp_term, 1))
                    {
                        return pow_int_compute(base, static_cast<i64>(exp_term));
                    }
                    else
                    {
                        return pow_int_compute(base, static_cast<i64>(exp_term)) *
                            pow_int_compute(base, static_cast<i64>(1 / mod(exp_term, 1)));
                    }
                }

                template <IsNumber T1, IsIntegral T2>
                static constexpr auto pow_integral(const T1& base, const T2& exp)
                {
                    return pow_int_type_check(base, exp);                
                }

                template < usize Depth, usize MaxDepth, IsNumber T >
                static constexpr auto exp_cf_rec(const T& value)
                {
                    if constexpr(Depth < MaxDepth)
                    {
                        if constexpr(Depth == 1)
                        {
                            return static_cast<T>(1) - value / 
                                exp_cf_rec< Depth + 1, MaxDepth >(value);
                        }
                        else
                        {
                            return static_cast<T>(1) + value / static_cast<T>(Depth - 1) - 
                                value / Depth / exp_cf_rec< Depth + 1, MaxDepth >(value);
                        }
                    }
                    else
                    {
                        return static_cast<T>(1);
                    }
                }

                template <IsNumber T>
                static constexpr auto exp_cf(const T value)
                {
                    return static_cast<T>(1) / exp_cf_rec<1, 25>(value);
                }

                template <IsNumber T>
                static constexpr auto exp_split(const T value)
                {
                    return pow_integral(
                        constants::euler, find_whole(value)
                    ) * exp_cf(find_fraction(value));
                }

                template <IsNumber T>
                static constexpr T mantissa(const T& value)
                {
                    if(value < static_cast<T>(1))
                    {
                        return mantissa(value * static_cast<T>(10));
                    }
                    else if(value > static_cast<T>(10))
                    {
                        return mantissa(value / static_cast<T>(10));
                    }
                    else
                    {
                        return value;
                    }
                }

                template <IsNumber T>
                static constexpr i64 find_exponent(const T value, const i64& exponent)
                {
                    if(value < static_cast<T>(1))
                    {
                        return find_exponent(value * static_cast<T>(10), exponent - static_cast<i64>(1));
                    }
                    else if(value > static_cast<T>(10))
                    {
                        return find_exponent(value / static_cast<T>(10) , exponent + static_cast<i64>(1));
                    }
                    else
                    {
                        return exponent;
                    }
                }

                template <IsNumber T>
                static constexpr auto tan_series_exp_long(const T& value)
                {   
                    return (
                        -1 / value + (value / 3 + (pow_integral(value, 3) / 45 + 
                        (2 * pow_integral(value, 5) / 945 + pow_integral(value, 7) / 4725)))
                    );
                }

                template <IsNumber T>
                static constexpr auto tan_series_exp(const T& value)
                {
                    using constants::pi;
                    if(limits<T>::epsilon > math::abs(value - static_cast<T>(pi / 2)))
                    {
                        return static_cast<T>(pi / 2);
                    }
                    else 
                    {
                        return tan_series_exp_long(value - static_cast<T>(pi / 2));
                    }
                }

                template < usize Depth, usize MaxDepth, IsNumber T >
                static constexpr auto tan_cf_rec(const T& value)
                {
                    if constexpr(Depth < MaxDepth)
                    {
                        return static_cast<T>(2 * Depth - 1) - value / 
                            tan_cf_rec< Depth + 1, MaxDepth >(value);
                    }
                    else
                    {
                        return static_cast<T>(2 * MaxDepth - 1);
                    }
                }

                template <IsNumber T>
                static constexpr auto tan_cf_main(const T& value)
                {
                    if(value > static_cast<T>(1.55) && value < static_cast<T>(1.60))
                    {
                        return tan_series_exp(value);
                    }
                    else if(value > static_cast<T>(1.4))
                    {
                        return value / tan_cf_rec<1, 45>(value * value);
                    }
                    else if(value > static_cast<T>(1))
                    {
                        return value / tan_cf_rec<1, 35>(value * value);
                    }
                    else
                    {
                        return value / tan_cf_rec<1, 25>(value * value);
                    }
                }

                template <IsNumber T>
                static constexpr auto tan_begin(T value)
                {
                    using constants::pi;

                    if(value > static_cast<T>(pi))
                    {
                        value -= static_cast<T>(pi) * floor(value / static_cast<T>(pi));

                        if(value > static_cast<T>(pi))
                        {
                            return limits<T>::nan;     
                        }
                        else
                        {
                            return tan_cf_main(value);
                        }        
                    }
                    else
                    {
                        return tan_cf_main(value);
                    }
                }

                template <IsFloat T>
                static constexpr bool invlaid_case(const T& value)
                {
                    return (
                        value == limits<T>::infinity || 
                        value == -limits<T>::infinity || 
                        value == limits<T>::nan
                    );
                }

                template <IsNumber T>
                static constexpr auto sin_compute(const T& value)
                {
                    return static_cast<T>(2) * value / (static_cast<T>(1) + value * value);
                }

                template <IsNumber T>
                static constexpr auto cos_compute(const T& value)
                {
                    return (static_cast<T>(1) - value * value) / (static_cast<T>(1) + value * value);
                }

                template < usize Depth, usize MaxDepth, IsNumber T >
                static constexpr auto log_cf_main(const T& value)
                {
                    if constexpr(Depth < MaxDepth)
                    {
                        return static_cast<T>(2 * Depth - 1) - 
                        static_cast<T>(Depth * Depth) * value / 
                        log_cf_main< Depth + 1, MaxDepth >(value);
                    }
                    else
                    {
                        return static_cast<T>(2 * Depth - 1);
                    }
                }

                template <IsNumber T>
                static constexpr auto log_cf_begin(const T& value)
                { 
                    return static_cast<T>(2) * value / log_cf_main<1, 25>(value * value);
                }

                template <IsNumber T>
                static constexpr auto log_main(const T& value)
                { 
                    return log_cf_begin((value - static_cast<T>(1)) / (value + static_cast<T>(1)));
                }

                static constexpr f128 log_mantissa_integer(const i32& value)
                {
                    if(value > 1 && value < 11)
                    {
                        return constants::ln_table[value - 2];
                    }
                    else
                    {
                        return 0.l;
                    }
                }

                template <IsNumber T>
                static constexpr auto log_mantissa(const T& value)
                {   
                    return log_main(value / static_cast<T>(static_cast<i32>(value))) + 
                        static_cast<T>(log_mantissa_integer(static_cast<i32>(value)));
                }

                template <IsNumber T>
                static constexpr auto log_breakup(const T& value)
                {   
                    return log_mantissa(mantissa(value)) + 
                        static_cast<T>(constants::ln_table[8]) * 
                        static_cast<T>(find_exponent(value, 0));
                }

                template <IsNumber T>
                static constexpr auto pow_dbl(const T& base, const T& exp_term)
                {
                    return constexpr_math::exp(exp_term * constexpr_math::log(base));
                }

                template <IsNumber T1, IsFloat T2>
                static constexpr auto pow_type_check(const T1& base, const T2& exp_term)
                {
                    using ReturnType = std::common_type_t<T1, T2>;

                    if(base < static_cast<T1>(0))
                    {
                        return limits<ReturnType>::nan;
                    }
                    else
                    {
                        return pow_dbl(
                            static_cast<ReturnType>(base), 
                            static_cast<ReturnType>(exp_term)
                        );
                    }
                }

                template <IsNumber T1, IsIntegral T2>
                static constexpr auto pow_type_check(const T1& base, const T2& exp_term)
                    -> std::common_type_t<T1, T2>
                {
                    return pow_integral(base, exp_term);
                }

                template <usize Order, IsNumber T>
                static constexpr auto atan_series_order_calc(const T& value, const T& value_pow)
                {
                    return ( 
                        static_cast<T>(1) / (static_cast<T>((Order - 1) * 4 - 1) * value_pow ) - 
                        static_cast<T>(1) / (static_cast<T>((Order - 1) * 4 + 1) * value_pow * value)
                    );
                }
                
                template <usize Order, usize MaxOrder, IsNumber T>
                static constexpr auto atan_series_order(const T& value, const T& value_pow)
                {
                    if constexpr(Order == 1)
                    {
                        return (
                            static_cast<T>(math::constants::pi / 2) - static_cast<T>(1) / value + 
                            atan_series_order<Order + 1, MaxOrder>(value * value, pow(value, 3))
                        );
                    }
                    else if constexpr(Order < MaxOrder)
                    {
                        return (
                            atan_series_order_calc<Order>(value, value_pow) + 
                            atan_series_order<Order + 1, MaxOrder>(value, value_pow * value * value)
                        );
                    }
                    else if constexpr(Order == MaxOrder)
                    {
                        return atan_series_order_calc<Order>(value, value_pow);
                    }
                    else
                    {
                        return limits<T>::nan;
                    }
                }
                
                template <IsNumber T>
                static constexpr auto atan_series_main(const T& value)
                {
                    if(value < static_cast<T>(3))          return atan_series_order<1, 10>(value, value);
                    else if(value < static_cast<T>(4))     return atan_series_order<1, 9>(value, value);
                    else if(value < static_cast<T>(5))     return atan_series_order<1, 8>(value, value);
                    else if(value < static_cast<T>(7))     return atan_series_order<1, 7>(value, value);
                    else if(value < static_cast<T>(11))    return atan_series_order<1, 6>(value, value);
                    else if(value < static_cast<T>(25))    return atan_series_order<1, 5>(value, value);
                    else if(value < static_cast<T>(100))   return atan_series_order<1, 4>(value, value);
                    else if(value < static_cast<T>(1000))  return atan_series_order<1, 3>(value, value);
                    else                                   return atan_series_order<1, 2>(value, value);
                }
                
                template <usize Depth, usize MaxDepth, IsNumber T>
                static constexpr auto atan_cf_recur(const T& value)
                {
                    if constexpr(Depth < MaxDepth)
                    {
                        return static_cast<T>(2 * Depth - 1) + Depth * Depth * 
                        value / atan_cf_recur<Depth + 1, MaxDepth>(value);
                    }
                    else
                    {
                        return static_cast<T>(2 * Depth - 1);
                    }           
                }
                
                template <IsNumber T>
                static constexpr auto atan_cf_main(const T& value)
                {
                    if(value < static_cast<T>(0.5))         return value / atan_cf_recur<1, 15>(value * value); 
                    else if(value < static_cast<T>(1))      return value / atan_cf_recur<1, 25>(value * value); 
                    else if(value < static_cast<T>(1.5))    return value / atan_cf_recur<1, 35>(value * value); 
                    else if(value < static_cast<T>(2))      return value / atan_cf_recur<1, 45>(value * value); 
                    else                                    return value / atan_cf_recur<1, 52>(value * value);
                }
                
                template <IsNumber T>
                static constexpr auto atan_begin(const T& value)
                {
                    if(value > static_cast<T>(2.5))
                    {
                        return static_cast<T>(atan_series_main(value));
                    }
                    else
                    {
                        return static_cast<T>(atan_cf_main(value));
                    }
                }
                
                template <IsFloat T>
                static constexpr auto atan_check(const T& value)
                {
                    if(value == limits<T>::nan)
                    {
                        return value;
                    }
                    else if(limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(0);
                    }
                    else if(value < static_cast<T>(0))
                    {
                        return -atan_begin(-value);
                    }
                    else
                    {
                        return atan_begin(value);
                    }
                }

                template <IsFloat T>
                static constexpr auto asin_compute(const T& value)
                {
                    if(value > static_cast<T>(1))
                    {
                        return limits<T>::nan;
                    }
                    else if(limits<T>::epsilon > abs(value - static_cast<T>(1)))
                    {
                        return static_cast<T>(math::constants::pi / 2);
                    }
                    else if(limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(0);
                    }
                    else
                    {
                        return atan(value / sqrt(static_cast<T>(1) - value * value));
                    }
                }

                template <IsFloat T>
                static constexpr auto asin_check(const T& value)
                {
                    if(value == limits<T>::nan)
                    {
                        return value;
                    }
                    else if(value < static_cast<T>(0))
                    {
                        return -asin_compute(-value);
                    }
                    else
                    {
                        return asin_compute(value);
                    }
                }

                template <IsFloat T>
                static constexpr auto acos_compute(const T& value)
                {
                    if (abs(value) > static_cast<T>(1))
                    {
                        return limits<T>::nan;
                    }
                    else if(limits<T>::epsilon > abs(value - static_cast<T>(1)))
                    {
                        return static_cast<T>(0);
                    }
                    else if(limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(math::constants::pi / 2);
                    }
                    else
                    {
                        return atan(sqrt(static_cast<T>(1) - value * value) / value);
                    }
                }

                template <IsFloat T>
                static constexpr auto acos_check(const T& value)
                {
                    using math::constants::pi;

                    if(value == limits<T>::nan)
                    {
                        return value;
                    }
                    else if(value > static_cast<T>(0))
                    {
                        return acos_compute(value);
                    }
                    else
                    {
                        return static_cast<T>(pi / 2) - 
                            acos_compute(-value);
                    }
                }

                template <IsFloat T>
                static constexpr auto atan2_compute(const T& y, const T& x)
                {
                    using math::constants::pi;

                    if(y == limits<T>::nan || x == limits<T>::nan)
                    {
                        return limits<T>::nan;
                    }
                    else if(limits<T>::epsilon > abs(x))
                    {
                        if(limits<T>::epsilon > abs(y))
                        {
                            if(y == static_cast<T>(0) && sign_bit(y))
                            {
                                if(x == static_cast<T>(0) && sign_bit(x))
                                {
                                    return -static_cast<T>(pi / 2);
                                }
                                else
                                {
                                    return -static_cast<T>(0);
                                }
                            }
                            else if(x == static_cast<T>(0) && sign_bit(x))
                            {
                                return static_cast<T>(pi / 2);
                            }
                            else
                            {
                                return static_cast<T>(0);
                            }
                        }
                        else if(y > static_cast<T>(0))
                        {
                            return static_cast<T>(pi / 2);
                        }
                        else
                        {
                            return -static_cast<T>(pi / 2);
                        }
                    }
                    else if(x < static_cast<T>(0))
                    {
                        if(y < static_cast<T>(0))
                        {
                            return atan(y / x) - static_cast<T>(pi);
                        }
                        else
                        {
                            return atan(y / x) + static_cast<T>(pi);
                        }
                    }
                    else
                    {
                        return atan(y / x);
                    }
                }

                template <IsFloat T1, IsFloat T2>
                static constexpr auto atan2_type_check(const T1& y, const T2& x)
                {
                    using RetType = std::common_type_t<T1, T2>;

                    return atan2_compute(
                        static_cast<RetType>(y), 
                        static_cast<RetType>(x)
                    );
                }

                template <IsFloat T>
                static constexpr auto sinh_check(const T& value)
                {
                    if(value == limits<T>::nan)
                    {
                        return value;
                    }
                    else if(limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(0);
                    }
                    else
                    {
                        return (exp(value) - exp(-value)) / static_cast<T>(2);
                    }
                }

                template <IsFloat T>
                static constexpr auto cosh_check(const T& value)
                {
                    if(value == limits<T>::nan)
                    {
                        return value;
                    }
                    else if(limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(1);
                    }
                    else
                    {
                        return (exp(value) + exp(-value)) / static_cast<T>(2);
                    }
                }

                template <usize Depth, IsFloat T>
                static constexpr auto tanh_cf(const T value)
                {
                    if constexpr(Depth < 25)
                    {
                        return (2 * Depth - 1) + value / tanh_cf<Depth + 1>(value);
                    }
                    else
                    {
                        return static_cast<T>(2 * Depth - 1);
                    }
                }

                template <IsFloat T>
                static constexpr auto tanh_begin(const T& value)
                {
                    return value / tanh_cf<1>(value * value);
                }

                template <IsFloat T>
                static constexpr auto tanh_check(const T& value)
                {
                    if(value == limits<T>::nan)
                    {
                        return value;
                    }
                    else if(limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(0);
                    }
                    else if(value < static_cast<T>(0))
                    {
                        return -tanh_begin(-value);
                    }
                    else
                    {
                        return tanh_begin(value);
                    }
                }

                template <IsFloat T>
                static constexpr auto asinh_compute(const T& value)
                {
                    if(value == limits<T>::nan)
                    {
                        return value;
                    }
                    else if(limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(0);
                    }
                    else
                    {
                        return log(value + sqrt(value * value + static_cast<T>(1)));
                    }
                }

                template <IsFloat T>
                static constexpr auto acosh_compute(const T& value)
                {
                    if(value == limits<T>::nan)
                    {
                        return value;
                    }
                    else if(limits<T>::epsilon > abs(value - static_cast<T>(1)))
                    {
                        return static_cast<T>(0);
                    }
                    else
                    {
                        return log(value + sqrt(value * value - static_cast<T>(1)));
                    }
                }

                
                template <IsFloat T>
                static constexpr auto atanh_compute(const T& value)
                {
                    return log(
                        (static_cast<T>(1) + value) / 
                        (static_cast<T>(1) - value)
                    ) / static_cast<T>(2);
                }

                template <IsFloat T>
                static constexpr auto atanh_check(const T& value)
                {
                    if(value == limits<T>::nan)
                    {
                        return value;
                    }
                    else if(static_cast<T>(1) < abs(value))
                    {
                        return limits<T>::nan;
                    }
                    else if(limits<T>::epsilon > (static_cast<T>(1) - abs(value)))
                    {
                        return sgn(value) * limits<T>::infinity; 
                    }
                    else if(limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(0);
                    }
                    else
                    {
                        return atanh_compute(value);
                    }
                        
                }

                template <IsFloat T>
                static constexpr auto lgamma_coef_term(const T& value)
                {
                    return (     
                        0.99999999999999709182l                     + 
                        57.156235665862923517l      / (value + 1)   -
                        59.597960355475491248l      / (value + 2)   + 
                        14.136097974741747174l      / (value + 3)   -
                        0.49191381609762019978l     / (value + 4)   + 
                        0.33994649984811888699e-4l  / (value + 5)   +
                        0.46523628927048575665e-4l  / (value + 6)   - 
                        0.98374475304879564677e-4l  / (value + 7)   +
                        0.15808870322491248884e-3l  / (value + 8)   - 
                        0.21026444172410488319e-3l  / (value + 9)   +
                        0.21743961811521264320e-3l  / (value + 10)  - 
                        0.16431810653676389022e-3l  / (value + 11)  +
                        0.84418223983852743293e-4l  / (value + 12)  -
                        0.26190838401581408670e-4l  / (value + 13)  +
                        0.36899182659531622704e-5L  / (value + 14) 
                    );  
                }
                
                template <IsFloat T>
                static constexpr auto lgamma_term_2(const T& value)
                {
                    return (
                        static_cast<T>(constants::log_sqrt_pi) + 
                        log(static_cast<T>(lgamma_coef_term(value)))
                    );
                }
                
                template <IsNumber T>
                static constexpr auto lgamma_term_1(const T& value)
                {   
                    constexpr f128 _key_gamma = 5.2421875l;
                    return ( 
                        (value + static_cast<T>(0.5l)) * 
                        log(value + static_cast<T>(_key_gamma)) - 
                        (value + static_cast<T>(_key_gamma))
                    );
                }
                
                template <IsFloat T>
                static constexpr auto lgamma_begin(const T& value)
                {   
                    return lgamma_term_1(value) + lgamma_term_2(value);
                }
                
                template <IsFloat T>
                static constexpr auto lgamma_check(const T& value)
                {
                    if(value == limits<T>::nan)
                    {
                        return value;
                    }
                    else if(limits<T>::epsilon > abs(value - static_cast<T>(1)))
                    {
                        return static_cast<T>(0);
                    }
                    else if(limits<T>::epsilon > value)
                    {
                        return limits<T>::infinity;
                    }
                    else
                    {
                        return lgamma_begin(value - static_cast<T>(1));
                    }
                }

                template <IsFloat T>
                static constexpr auto tgamma_check(const T& value)
                {
                    if(value == limits<T>::nan)
                    {
                        return value;
                    }
                    else if(limits<T>::epsilon > abs(value - static_cast<T>(1)))
                    {
                        return static_cast<T>(1);
                    }
                    else if(limits<T>::epsilon > abs(value))
                    {
                        return limits<T>::infinity;
                    }
                    else if(value < static_cast<T>(0))
                    { 
                        if(limits<T>::epsilon > abs(value - find_whole(value)))
                        {
                            return limits<T>::nan;
                        }
                        else
                        {
                            return tgamma_check(
                                value + static_cast<T>(1)
                            ) / value;
                        }
                    }
                    else
                    {
                        if(static_cast<u64>(value) == value)
                        {
                            return factorial<T>(
                                static_cast<usize>(value) - 1
                            );
                        }
                        else
                        {
                            return exp(lgamma(value));
                        }
                    }
                }
            } // namespace ctmath_detail

            template <IsNumber T>
            static constexpr auto factorial(usize size)
            {
                T _result = 1;

                for(usize _index = size; _index >= 2; _index--)
                    _result *= static_cast<T>(_index);

                return _result;
            }

            template <IsFloat T>
            static constexpr auto mod(const T& value, const T& factor)
            {
                if(ctmath_detail::invlaid_case(value))
                {
                    return value;
                }
                else
                {
                    i64 _rez = static_cast<i64>(value / factor);
                    return value - static_cast<T>(_rez) * factor;
                }
            }

            template <IsFloat T>
            static constexpr auto floor(const T& value)
            {
                if(ctmath_detail::invlaid_case(value))
                {
                    return value;
                }
                else
                {
                    i64 _rez = static_cast<i64>(value);
                    return static_cast<T>(_rez - (value < 0 && 
                        constexpr_math::mod(value, static_cast<T>(1)) != 0)
                    );
                }
            }

            template <IsFloat T>
            static constexpr auto ceil(const T& value)
            {
                if(ctmath_detail::invlaid_case(value))
                {
                    return value;
                }
                else
                {
                    i64 _res = static_cast<i64>(value);

                    return static_cast<T>(_res + (value > 0 && 
                        constexpr_math::mod(value, static_cast<T>(1)) != 0)
                    );
                }
            }

            template <IsFloat T>
            static constexpr auto round(const T& value)
            {
                if(ctmath_detail::invlaid_case(value))
                {
                    return value;
                }
                else
                {
                    return floor(value) + 
                        (value > 0 && constexpr_math::mod(value, static_cast<T>(1)) >= 0.5) + 
                        (value < 0 && constexpr_math::mod(1 + 
                            constexpr_math::mod(value, static_cast<T>(1)), static_cast<T>(1)) >= 0.5
                    );
                }
            }

            template <IsNumber T>
            static constexpr auto tan(T value)
            {
                if(value == limits<T>::nan)
                {
                    return value;
                }
                else if(limits<T>::epsilon > math::abs(value))
                {
                    return static_cast<T>(0);
                }
                else
                {
                    return value < 0 ? -ctmath_detail::tan_begin(-value) : ctmath_detail::tan_begin(value);
                }
            }

            template <IsNumber T>
            static constexpr auto sin(T value)
            {
                using constants::pi;
                if(value == limits<T>::nan)
                {
                    return value;
                }
                else if(limits<T>::epsilon > math::abs(value))
                {
                    return static_cast<T>(0);
                }
                else if(limits<T>::epsilon > math::abs(value - static_cast<T>(pi / 2)))
                {
                    return static_cast<T>(1);
                }
                else if(limits<T>::epsilon > math::abs(value + static_cast<T>(pi / 2)))
                {
                    return -static_cast<T>(1);
                }
                else if(limits<T>::epsilon > math::abs(value - static_cast<T>(pi)))
                {
                    return static_cast<T>(0);
                }
                else if(limits<T>::epsilon > math::abs(value + static_cast<T>(pi)))
                {
                    return -static_cast<T>(0);
                }
                else
                {
                    return ctmath_detail::sin_compute(constexpr_math::tan(value / static_cast<T>(2)));
                }
            }

            template <IsNumber T>
            static constexpr auto cos(T value)
            {
                using constants::pi;
                if(value == limits<T>::nan)
                {
                    return value;
                }
                else if(limits<T>::epsilon > math::abs(value))
                {
                    return static_cast<T>(1);
                }
                else if(limits<T>::epsilon > math::abs(value - static_cast<T>(pi / 2)))
                {
                    return static_cast<T>(0);
                }
                else if(limits<T>::epsilon > math::abs(value + static_cast<T>(pi / 2)))
                {
                    return static_cast<T>(0);
                }
                else if(limits<T>::epsilon > math::abs(value - static_cast<T>(pi)))
                {
                    return -static_cast<T>(1);
                }
                else if(limits<T>::epsilon > math::abs(value + static_cast<T>(pi)))
                {
                    return -static_cast<T>(1);
                }
                else
                {
                    return ctmath_detail::cos_compute(constexpr_math::tan(value / static_cast<T>(2)));
                }
            }

            template <IsNumber T>
            static constexpr auto log(T value)
            {
                if(value == limits<T>::nan || value < static_cast<T>(0))
                {
                    return limits<T>::nan;
                }
                else if(limits<T>::epsilon > value)
                {
                    return -limits<T>::infinity;
                }
                else if(limits<T>::epsilon > math::abs(value - static_cast<T>(1)))
                {
                    return static_cast<T>(0);
                }
                else if(value == limits<T>::infinity)
                {
                    return limits<T>::infinity;
                }
                else
                {
                    if(value < static_cast<T>(0.5) || value > static_cast<T>(1.5))
                    {
                        return ctmath_detail::log_breakup(value);
                    }
                    else
                    {
                        return ctmath_detail::log_main(value);
                    }
                }  
            }

            template <IsNumber T>
            static constexpr auto log2(T value)
            {
                if(value == limits<T>::nan || value < static_cast<T>(0))
                {
                    return limits<T>::nan;
                }
                else if(limits<T>::epsilon > value)
                {
                    return -limits<T>::infinity;
                }
                else if(limits<T>::epsilon > math::abs(value - static_cast<T>(1)))
                {
                    return static_cast<T>(0);
                }
                else if(value == limits<T>::infinity)
                {
                    return limits<T>::infinity;
                }
                else
                {
                    return log(value) / static_cast<T>(constants::ln_table[0]);
                }  
            }

            template <IsNumber T>
            static constexpr auto log10(T value)
            {
                if(value == limits<T>::nan || value < static_cast<T>(0))
                {
                    return limits<T>::nan;
                }
                else if(limits<T>::epsilon > value)
                {
                    return -limits<T>::infinity;
                }
                else if(limits<T>::epsilon > math::abs(value - static_cast<T>(1)))
                {
                    return static_cast<T>(0);
                }
                else if(value == limits<T>::infinity)
                {
                    return limits<T>::infinity;
                }
                else
                {
                    return log(value) / static_cast<T>(constants::ln_table[8]);
                }  
            }

            template <IsNumber T>
            static constexpr auto exp(T value)
            {
                if(value == limits<T>::nan || value == limits<T>::infinity)
                {
                    return value;
                }
                else if(value == -limits<T>::infinity)
                {
                    return static_cast<T>(0);
                }
                else if(limits<T>::epsilon > math::abs(value))
                {
                    return static_cast<T>(1);
                }
                else if(math::abs(value) < static_cast<T>(2))
                {
                    return static_cast<T>(ctmath_detail::exp_cf(value));
                }
                else
                {
                    return static_cast<T>(ctmath_detail::exp_split(value));
                }
            }

            template <IsNumber T1, IsNumber T2>
            static constexpr auto pow(T1 base, T2 exp_term)
            {
                return ctmath_detail::pow_type_check(base, exp_term);
            }

            template <IsFloat T>
            static constexpr auto sqrt(const T& value)
            {
                auto mid_val = static_cast<T>(1.0);

                if(value <= static_cast<T>(-0.0))
                {
                    return limits<T>::nan;
                }
                for(usize count = 0; count < 10; count++)
                {
                    auto euler = value / mid_val;
                    mid_val = midpoint(mid_val, euler);
                }

                return mid_val;
            }

            template <IsNumber T>
            static constexpr auto atan(T value)
            {
                return ctmath_detail::atan_check(value);
            }

            template <IsNumber T>
            static constexpr auto asin(T value)
            {
                return ctmath_detail::asin_check(value);
            }

            template <IsNumber T>
            static constexpr auto acos(T value)
            {
                return ctmath_detail::acos_check(value);
            }

            template <IsNumber T1, IsNumber T2>
            static constexpr auto atan2(const T1& y, const T2& x)
            {
                return ctmath_detail::atan2_type_check(y, x);
            }

            template <IsNumber T>
            static constexpr auto sinh(const T& value)
            {
                return ctmath_detail::sinh_check(value);
            }

            template <IsNumber T>
            static constexpr auto cosh(const T& value)
            {
                return ctmath_detail::cosh_check(value);
            }

            template <IsNumber T>
            static constexpr auto tanh(const T& value)
            {
                return ctmath_detail::tanh_check(value);
            }

            template <IsNumber T>
            static constexpr auto asinh(const T& value)
            {
                return ctmath_detail::asinh_compute(value);
            }

            template <IsNumber T>
            static constexpr auto acosh(const T& value)
            {
                return ctmath_detail::acosh_compute(value);
            }

            template <IsNumber T>
            static constexpr auto atanh(const T& value)
            {
                return ctmath_detail::atanh_check(value);
            }

            template <IsNumber T>
            static constexpr auto lgamma(const T& value)
            {
                return ctmath_detail::lgamma_check(value);
            }

            template <IsNumber T>
            static constexpr auto tgamma(const T& value)
            {
                return ctmath_detail::tgamma_check(value);
            }

            template <IsNumber T1, IsNumber T2>
            static constexpr auto lbeta(const T1& a, const T2& b)
            {
                using RetType = std::common_type_t<T1, T2>;
                return static_cast<RetType>(
                    (lgamma(a) + lgamma(b)) - lgamma(a + b)
                );
            }

            template <IsNumber T1, IsNumber T2>
            static constexpr auto beta(const T1& a, const T2& b)
            {
                return exp(lbeta(a, b));
            }
        } // namespace constexpr_math

        template <Middleable T>
        constexpr auto midpoint(T a, T b)
        {
            if constexpr(hsd::is_integral<T>::value)
            {
                if(a > b)
                {
                    return a - 1 * static_cast<T>(static_cast<usize>(a - b) / 2);
                }
                else
                {
                    return a + 1 * static_cast<T>(static_cast<usize>(b - a) / 2);
                }
            }
            else
            {
                constexpr auto _low_limit = limits<T>::min * 2;
                constexpr auto _high_limit = limits<T>::max / 2;
                const auto _abs_a = math::abs(a);
                const auto _abs_b = math::abs(b);

                if(_abs_a <= _high_limit && _abs_b <= _high_limit)
                {
                    return (a + b) / 2;
                }
                else if(_abs_a < _low_limit)
                {
                    return a + b / 2;
                }
                else if(_abs_b < _low_limit)
                {
                    return a / 2 + b;
                }
                else
                {
                    return a / 2 + b / 2;
                }
            }
        }

        template <IsObject T>
        constexpr T* midpoint(T* a, T* b)
        {
            static_assert(sizeof(T) != 0, "type must be complete");
            return a + (b - a) / 2;
        }

        template <IsFloat T>
        static constexpr auto mod(const T& value, const T& factor)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::mod(value, factor);
            }
            else
            {
                return std::fmod(value, factor);
            }
        }

        template <IsFloat T>
        static constexpr auto floor(const T& value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::floor(value);
            }
            else
            {
                return std::floor(value);
            }
        }

        template <IsFloat T>
        static constexpr auto ceil(const T& value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::ceil(value);
            }
            else
            {
                return std::ceil(value);
            }
        }

        template <IsFloat T>
        static constexpr auto round(const T& value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::round(value);
            }
            else
            {
                return std::round(value);
            }
        }

        template <IsNumber T>
        static constexpr auto abs(const T& value)
        {
            return value < 0 ? -value : value;
        }

        template <IsNumber T1>
        static constexpr T1 min(const T1& first, const T1& second)
        {
            return first < second ? first : second;
        }

        template <IsNumber T1>
        static constexpr T1 max(const T1& first, const T1& second)
        {
            return first > second ? first : second;
        }

        template <IsNumber T>
        static constexpr auto tan(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::tan(value);
            }
            else
            {
                return std::tan(value);
            }
        }

        template <IsNumber T>
        static constexpr auto sin(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::sin(value);
            }
            else
            {
                return std::sin(value);
            }
        }

        template <IsNumber T>
        static constexpr auto cos(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::cos(value);
            }
            else
            {
                return std::cos(value);
            }
        }

        template <IsNumber T>
        static constexpr auto atan(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::atan(value);
            }
            else
            {
                return std::atan(value);
            }
        }

        template <IsNumber T>
        static constexpr auto asin(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::asin(value);
            }
            else
            {
                return std::asin(value);
            }
        }

        template <IsNumber T>
        static constexpr auto acos(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::acos(value);
            }
            else
            {
                return std::acos(value);
            }
        }

        template <IsNumber T1, IsNumber T2>
        static constexpr auto atan2(const T1 y, const T2 x)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::atan2(y, x);
            }
            else
            {
                return std::atan2(y, x);
            }
        }

        template <IsNumber T>
        static constexpr auto log(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::log(value);
            }
            else
            {
                return std::log(value);
            }
        }

        template <IsNumber T>
        static constexpr auto log2(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::log2(value);
            }
            else
            {
                return std::log2(value);
            } 
        }

        template <IsNumber T>
        static constexpr auto log10(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::log10(value);
            }
            else
            {
                return std::log10(value);
            }
        }

        template <IsNumber T>
        static constexpr auto sqrt(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::sqrt(value);
            }
            else
            {
                return std::sqrt(value);
            }
        }

        template <IsNumber T>
        static constexpr auto exp(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::exp(value);
            }
            else
            {
                return std::exp(value);
            }
        }
        
        template <IsNumber T1, IsNumber T2>
        static constexpr auto pow(T1 base, T2 exp_term)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::pow(base, exp_term);
            }
            else
            {
                return std::pow(base, exp_term);
            }
        }

        template <IsNumber T>
        static constexpr auto sinh(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::sinh(value);
            }
            else
            {
                return std::sinh(value);
            }
        }

        template <IsNumber T>
        static constexpr auto cosh(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::cosh(value);
            }
            else
            {
                return std::cosh(value);
            }
        }

        template <IsNumber T>
        static constexpr auto tanh(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::tanh(value);
            }
            else
            {
                return std::tanh(value);
            }
        }

        template <IsNumber T>
        static constexpr auto asinh(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::asinh(value);
            }
            else
            {
                return std::asinh(value);
            }
        }

        template <IsNumber T>
        static constexpr auto acosh(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::acosh(value);
            }
            else
            {
                return std::acosh(value);
            }
        }

        template <IsNumber T>
        static constexpr auto atanh(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::atanh(value);
            }
            else
            {
                return std::atanh(value);
            }
        }

        template <IsNumber T>
        static constexpr auto lgamma(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::lgamma(value);
            }
            else
            {
                return std::lgamma(value);
            }
        }

        template <IsNumber T>
        static constexpr auto tgamma(T value)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::tgamma(value);
            }
            else
            {
                return std::tgamma(value);
            }
        }

        template <IsNumber T1, IsNumber T2>
        static constexpr auto lbeta(const T1& a, const T2& b)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::lbeta(a, b);
            }
            else
            {
                return std::log(std::beta(a, b));
            }
        }

        template <IsNumber T1, IsNumber T2>
        static constexpr auto beta(const T1& a, const T2& b)
        {
            if(std::is_constant_evaluated())
            {
                return constexpr_math::beta(a, b);
            }
            else
            {
                return std::beta(a, b);
            }
        }
    } // namespace math
} // namespace hsd