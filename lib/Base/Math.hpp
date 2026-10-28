#pragma once

/// @file Math.hpp
/// @brief constexpr mathematical functions and scalar helpers.
/// @details Wraps elementary operations such as roots, powers, logarithms,
/// trigonometry, rounding, extrema, and special functions for project types.

#include "Limits.hpp"
#include "Concepts.hpp"
#include <cmath>

namespace hsd
{
    /// @namespace hsd::math
    /// @brief Constexpr-capable mathematical functions mirroring the `<cmath>` API.
    /// @details The implementations are NOT original, they are, in fact
    /// A C++20 adaptation of a C++11 codebase thanks to the
    /// compile-time math library: https://github.com/kthohr/gcem
    namespace math
    {
        /// @namespace hsd::math::constants
        /// @brief Compile-time mathematical and logarithm-table constants shared by the library.
        namespace constants
        {
            /// @brief The ratio of a circle's circumference to its diameter, π.
            static constexpr f128 pi = 3.14159265358979323846264338327950288l;
            /// @brief Euler's number, the base of the natural logarithm.
            static constexpr f128 euler = 2.71828182845904523536028747135266249l;
            /// @brief Precomputed value of ln(sqrt(π)), used by the Lanczos-style lgamma approximation.
            static constexpr f128 log_sqrt_pi = 0.91893853320467274178032973640561763l;
            /// @brief Natural logarithms of the integers 2 through 10.
            /// @details Indexed by `value - 2`, so `ln_table[0] == ln(2)` and `ln_table[8] == ln(10)`;
            /// used to reconstruct mantissa logarithms and to convert `log` results into `log2`/`log10`.
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
        
        /// @brief Constrains the types accepted by `math::midpoint`.
        /// @details Satisfied by numeric types (`is_number`), by any type that is not
        /// cv-qualified, or by any type other than `bool`; effectively admits all
        /// non-cv-qualified scalar types while explicitly ruling out `bool`.
        /// @tparam T The candidate type.
        template <typename T>
        concept Middleable = is_number<T>::value ||
            is_same<remove_cv_t<T>, T>::value ||
            negation<is_same<T, bool>>::value;

        /// @fn constexpr auto hsd::math::midpoint(T a, T b)
        /// @brief Computes the midpoint of two values without overflow/underflow.
        /// @tparam T A `Middleable` numeric type.
        template <Middleable T>
        constexpr auto midpoint(T a, T b);

        /// @overload
        /// @fn constexpr auto hsd::math::midpoint(T* a, T* b)
        /// @brief Computes the midpoint of two pointers into the same array.
        /// @tparam T A complete object type.
        template <ObjectType T>
        constexpr auto midpoint(T* a, T* b);
        /// @brief Computes the arctangent of `value`, in radians.
        /// @details Dispatches to the constexpr implementation during constant evaluation
        /// and to `std::atan` otherwise.
        /// @tparam T A numeric type.
        /// @param value The input value.
        /// @return The arctangent of `value`, in the range [-π/2, π/2].
        template <NumericType T>
        static constexpr auto atan(T value);
        /// @brief Computes the square root of `value`.
        /// @details Dispatches to the constexpr implementation during constant evaluation
        /// and to `std::sqrt` otherwise.
        /// @tparam T A numeric type.
        /// @param value The input value.
        /// @return The non-negative square root of `value`, or NaN if `value` is negative.
        template <NumericType T>
        static constexpr auto sqrt(T value);

        /// @brief Computes the absolute value of `value`.
        /// @tparam T A numeric type.
        /// @param value The input value.
        /// @return `-value` if `value < 0`, otherwise `value`.
        template <NumericType T>
        static constexpr auto abs(const T& value)
        {
            return value < 0 ? -value : value;
        }

        /// @brief Returns the smaller of two values.
        /// @tparam T1 A numeric type.
        /// @param first The first value.
        /// @param second The second value.
        /// @return `first` if `first < second`, otherwise `second`.
        template <NumericType T1>
        static constexpr T1 min(const T1& first, const T1& second)
        {
            return first < second ? first : second;
        }

        /// @brief Returns the larger of two values.
        /// @tparam T1 A numeric type.
        /// @param first The first value.
        /// @param second The second value.
        /// @return `first` if `first > second`, otherwise `second`.
        template <NumericType T1>
        static constexpr T1 max(const T1& first, const T1& second)
        {
            return first > second ? first : second;
        }

        /// @brief Checks whether `val` is NaN (not-a-number).
        /// @details Relies on the IEEE-754 property that NaN never compares equal to itself.
        /// @param val The value to test.
        /// @return `true` if `val != val`, `false` otherwise.
        static constexpr bool is_nan(NumericType auto val)
        {
            return val != val;
        }

        /// @namespace hsd::math::constexpr_math
        /// @brief Constexpr-only reference implementations of the `math` functions.
        /// @details Used during constant evaluation by the dispatching wrappers in `hsd::math`.
        namespace constexpr_math
        {
            /// @brief Computes `size!` (the factorial of `size`).
            /// @tparam T The arithmetic result type.
            /// @param size The input value.
            /// @return The product of the integers from 2 to `size`, or 1 if `size < 2`.
            template <NumericType T>
            static constexpr auto factorial(usize size);
            /// @brief Rounds `value` down to the nearest integer.
            /// @tparam T A floating-point type.
            /// @param value The input value.
            /// @return The largest integral value not greater than `value`; NaN/Inf are passed through.
            template <FloatingPointType T>
            static constexpr auto floor(const T& value);
            /// @brief Computes the base-e exponential of `value`.
            /// @tparam T A numeric type.
            /// @param value The input value.
            /// @return e raised to the power `value`.
            template <NumericType T>
            static constexpr auto exp(T value);
            /// @brief Computes the natural logarithm of `value`.
            /// @tparam T A numeric type.
            /// @param value The input value.
            /// @return The natural logarithm of `value`; NaN for negative inputs, -infinity for zero.
            template <NumericType T>
            static constexpr auto log(T value);
            /// @brief Computes the natural logarithm of the gamma function of `value`.
            /// @tparam T A numeric type.
            /// @param value The input value.
            /// @return ln(Γ(value)).
            template <NumericType T>
            static constexpr auto lgamma(const T& value);

            /// @namespace hsd::math::constexpr_math::ctmath_detail
            /// @brief Private helper routines backing the `constexpr_math` public functions.
            /// @details Not part of the public API; implements series expansions, continued
            /// fractions, and range-reduction tricks used to keep the public functions constexpr.
            namespace ctmath_detail
            {
                /// @brief Returns the sign of `value`.
                /// @param value The input value.
                /// @return -1 if negative, 1 if positive, 0 if zero.
                template <NumericType T>
                static constexpr auto sgn(const T& value)
                {
                    if (value != 0)
                    {
                        return value < 0 ? -1 : 1;
                    }
                    else
                    {
                        return 0;
                    }
                }

                /// @brief Extracts the IEEE-754 sign bit of `value`.
                /// @details Truncates to `f32` before inspecting the sign bit, so this is only
                /// used for distinguishing +0/-0 and the sign of already-small magnitudes.
                /// @param value The input value.
                /// @return `true` if the sign bit is set (negative or -0), `false` otherwise.
                template <FloatingPointType T>
                static constexpr bool sign_bit(const T& value)
                {
                    return bit_cast<u32>(static_cast<f32>(value)) >> 31;
                }

                /// @brief Computes the fractional part of `value`, rounded to nearest.
                /// @details Used by the exponential range-reduction to split `value` into an
                /// integral part (`find_whole`) and a fractional remainder in [-0.5, 0.5].
                /// @param value The input value.
                /// @return `value - floor(value)`, adjusted by ±1 when that remainder is >= 0.5.
                template <FloatingPointType T>
                static constexpr auto find_fraction(const T& value)
                {
                    if (math::abs(value - constexpr_math::floor(value)) >= static_cast<T>(0.5))
                    {
                        return value - constexpr_math::floor(value) - sgn(value);
                    }
                    else
                    {
                        return value - constexpr_math::floor(value);
                    }
                }

                /// @brief Computes the integral part of `value`, rounded to nearest.
                /// @details Counterpart of `find_fraction`; together they satisfy
                /// `value == find_whole(value) + find_fraction(value)`.
                /// @param value The input value.
                /// @return `value` rounded to the nearest integer, as an `i64`.
                template <FloatingPointType T>
                static constexpr auto find_whole(const T& value)
                {
                    if (math::abs(value - constexpr_math::floor(value)) >= static_cast<T>(0.5))
                    {
                        return static_cast<i64>(constexpr_math::floor(value) + sgn(value));
                    }
                    else
                    { 
                        return static_cast<i64>(constexpr_math::floor(value));
                    }
                }

                /// @brief Recursive exponentiation-by-squaring accumulator.
                /// @tparam T1 A numeric base/accumulator type.
                /// @tparam T2 An integral exponent type.
                /// @param base The current squared base.
                /// @param val The accumulated product so far.
                /// @param exp_term The remaining (non-negative) exponent.
                /// @return `base` raised to the original exponent, multiplied into `val`.
                template < NumericType T1, IntegralType T2 >
                static constexpr T1 pow_int_compute_rec(
                    const T1& base, const T1& val, const T2& exp_term)
                {
                    if (exp_term > static_cast<T2>(1))
                    {
                        if ((exp_term & 1U) != 0)
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

                /// @brief Applies the sign of an integral exponent before computing an integer power.
                /// @tparam T1 A numeric base type.
                /// @tparam T2 An integral exponent type.
                /// @param base The base value.
                /// @param exp_term The (possibly negative) exponent.
                /// @return `base^exp_term`; negative exponents are handled via `1 / base^(-exp_term)`.
                template < NumericType T1, IntegralType T2 >
                static constexpr auto pow_int_sgn_check(const T1& base, const T2& exp_term)
                {
                    if (exp_term < 0)
                    {
                        return 1 / pow_int_compute_rec(base, static_cast<T1>(1), -exp_term);
                    }
                    else
                    {
                        return pow_int_compute_rec(base, static_cast<T1>(1), exp_term);
                    }
                }

                /// @brief Computes an integral power of `base`, special-casing small/extreme exponents.
                /// @tparam T1 A numeric base type.
                /// @tparam T2 An integral exponent type.
                /// @param base The base value.
                /// @param exp_term The exponent.
                /// @return `base^exp_term`; fast paths for exponents 0-3, and saturates to 0/infinity
                /// when `exp_term` equals `limits<T2>::min`/`limits<T2>::max`.
                template < NumericType T1, IntegralType T2 >
                static constexpr auto pow_int_compute(const T1& base, const T2& exp_term)
                {
                    if (exp_term == 3)
                    {
                        return base * base * base;
                    }
                    else if (exp_term == 2)
                    {
                        return base * base;
                    }
                    else if (exp_term == 1)
                    {
                        return base;
                    }
                    else if (exp_term == 0)
                    {
                        return static_cast<T1>(1);
                    }
                    else if (exp_term == limits<T2>::min)
                    {
                        return static_cast<T1>(0);
                    }
                    else if (exp_term == limits<T2>::max)
                    {
                        return limits<T1>::infinity;
                    }
                    else 
                    {
                        return pow_int_sgn_check(base, exp_term);
                    }
                }

                /// @brief Integral-exponent overload: forwards directly to `pow_int_compute`.
                /// @tparam T1 A numeric base type.
                /// @tparam T2 An integral exponent type.
                /// @param base The base value.
                /// @param exp_term The exponent.
                /// @return `base^exp_term`.
                template < NumericType T1, IntegralType T2 >
                static constexpr auto pow_int_type_check(const T1& base, const T2& exp_term)
                {
                    return pow_int_compute(base, exp_term);
                }

                /// @brief Floating-point-exponent overload: splits `exp_term` into integral and
                /// fractional parts and multiplies the corresponding integer powers.
                /// @tparam T1 A numeric base type.
                /// @tparam T2 A floating-point exponent type.
                /// @param base The base value.
                /// @param exp_term The exponent.
                /// @return `base^exp_term`, exact when `exp_term` has no fractional part.
                template < NumericType T1, FloatingPointType T2 >
                static constexpr auto pow_int_type_check(const T1& base, const T2& exp_term)
                {
                    if (limits<T2>::epsilon > mod(exp_term, 1))
                    {
                        return pow_int_compute(base, static_cast<i64>(exp_term));
                    }
                    else
                    {
                        return pow_int_compute(base, static_cast<i64>(exp_term)) *
                            pow_int_compute(base, static_cast<i64>(1 / mod(exp_term, 1)));
                    }
                }

                /// @brief Entry point for integer-exponent power computation.
                /// @tparam T1 A numeric base type.
                /// @tparam T2 An integral exponent type.
                /// @param base The base value.
                /// @param exp The exponent.
                /// @return `base^exp` via `pow_int_type_check`.
                template <NumericType T1, IntegralType T2>
                static constexpr auto pow_integral(const T1& base, const T2& exp)
                {
                    return pow_int_type_check(base, exp);                
                }

                /// @brief Recursive continued-fraction term for the exponential series.
                /// @tparam Depth The current recursion depth.
                /// @tparam MaxDepth The recursion depth at which the series is truncated.
                /// @tparam T A numeric type.
                /// @param value The (small-magnitude) exponent argument.
                /// @return The partial continued-fraction denominator at `Depth`.
                template < usize Depth, usize MaxDepth, NumericType T >
                static constexpr auto exp_cf_rec(const T& value)
                {
                    if constexpr (Depth < MaxDepth)
                    {
                        if constexpr (Depth == 1)
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

                /// @brief Evaluates `exp(value)` for small `value` via a 25-term continued fraction.
                /// @param value The (small-magnitude) exponent argument.
                /// @return An approximation of e^value.
                template <NumericType T>
                static constexpr auto exp_cf(const T value)
                {
                    return static_cast<T>(1) / exp_cf_rec<1, 25>(value);
                }

                /// @brief Evaluates `exp(value)` for larger magnitudes via range reduction.
                /// @details Splits `value` into an integral part (via `find_whole`) handled with
                /// integer powers of e, and a fractional remainder handled by `exp_cf`.
                /// @param value The exponent argument.
                /// @return An approximation of e^value.
                template <NumericType T>
                static constexpr auto exp_split(const T value)
                {
                    return pow_integral(
                        constants::euler, find_whole(value)
                    ) * exp_cf(find_fraction(value));
                }

                /// @brief Recursively scales `value` into the [1, 10] mantissa range.
                /// @param value The input value.
                /// @return `value` scaled by a power of 10 so that it lies within [1, 10].
                template <NumericType T>
                static constexpr T mantissa(const T& value)
                {
                    if (value < static_cast<T>(1))
                    {
                        return mantissa(value * static_cast<T>(10));
                    }
                    else if (value > static_cast<T>(10))
                    {
                        return mantissa(value / static_cast<T>(10));
                    }
                    else
                    {
                        return value;
                    }
                }

                /// @brief Recursively computes the base-10 exponent needed to bring `value` into [1, 10].
                /// @param value The input value, updated at each recursive step.
                /// @param exponent The accumulated exponent so far (start at 0).
                /// @return The power of 10 such that `value / 10^exponent` lies within [1, 10].
                template <NumericType T>
                static constexpr i64 find_exponent(const T value, const i64& exponent)
                {
                    if (value < static_cast<T>(1))
                    {
                        return find_exponent(value * static_cast<T>(10), exponent - static_cast<i64>(1));
                    }
                    else if (value > static_cast<T>(10))
                    {
                        return find_exponent(value / static_cast<T>(10) , exponent + static_cast<i64>(1));
                    }
                    else
                    {
                        return exponent;
                    }
                }

                /// @brief Taylor-series approximation of `tan` near π/2, in terms of the offset from π/2.
                /// @param value The argument, offset from π/2 (small magnitude).
                /// @return An approximation of `tan(value + π/2)`.
                template <NumericType T>
                static constexpr auto tan_series_exp_long(const T& value)
                {   
                    return (
                        -1 / value + (value / 3 + (pow_integral(value, 3) / 45 + 
                        (2 * pow_integral(value, 5) / 945 + pow_integral(value, 7) / 4725)))
                    );
                }

                /// @brief Handles `tan` evaluation near the π/2 singularity.
                /// @param value The argument, close to π/2.
                /// @return π/2 if `value` is within epsilon of π/2, otherwise the series
                /// approximation from `tan_series_exp_long`.
                template <NumericType T>
                static constexpr auto tan_series_exp(const T& value)
                {
                    using constants::pi;
                    if (limits<T>::epsilon > math::abs(value - static_cast<T>(pi / 2)))
                    {
                        return static_cast<T>(pi / 2);
                    }
                    else 
                    {
                        return tan_series_exp_long(value - static_cast<T>(pi / 2));
                    }
                }

                /// @brief Recursive continued-fraction term for the tangent series.
                /// @tparam Depth The current recursion depth.
                /// @tparam MaxDepth The recursion depth at which the series is truncated.
                /// @tparam T A numeric type.
                /// @param value The squared argument.
                /// @return The partial continued-fraction denominator at `Depth`.
                template < usize Depth, usize MaxDepth, NumericType T >
                static constexpr auto tan_cf_rec(const T& value)
                {
                    if constexpr (Depth < MaxDepth)
                    {
                        return static_cast<T>(2 * Depth - 1) - value / 
                            tan_cf_rec< Depth + 1, MaxDepth >(value);
                    }
                    else
                    {
                        return static_cast<T>(2 * MaxDepth - 1);
                    }
                }

                /// @brief Computes `tan(value)` for range-reduced arguments, selecting a
                /// continued-fraction depth (or the π/2-singularity series) based on magnitude.
                /// @param value The range-reduced argument, in [0, π].
                /// @return An approximation of `tan(value)`.
                template <NumericType T>
                static constexpr auto tan_cf_main(const T& value)
                {
                    if (value > static_cast<T>(1.55) && value < static_cast<T>(1.60))
                    {
                        return tan_series_exp(value);
                    }
                    else if (value > static_cast<T>(1.4))
                    {
                        return value / tan_cf_rec<1, 45>(value * value);
                    }
                    else if (value > static_cast<T>(1))
                    {
                        return value / tan_cf_rec<1, 35>(value * value);
                    }
                    else
                    {
                        return value / tan_cf_rec<1, 25>(value * value);
                    }
                }

                /// @brief Reduces `value` modulo π, then evaluates the tangent approximation.
                /// @param value The argument, in radians.
                /// @return An approximation of `tan(value)`, or NaN if range reduction fails.
                template <NumericType T>
                static constexpr auto tan_begin(T value)
                {
                    using constants::pi;

                    if (value > static_cast<T>(pi))
                    {
                        value -= static_cast<T>(pi) * floor(value / static_cast<T>(pi));

                        if (value > static_cast<T>(pi))
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

                /// @brief Checks whether `value` is one of the special IEEE-754 cases (±infinity or NaN).
                /// @param value The value to test.
                /// @return `true` if `value` is +infinity, -infinity, or NaN.
                template <FloatingPointType T>
                static constexpr bool invalid_case(const T& value)
                {
                    return (
                        value == limits<T>::infinity || 
                        value == -limits<T>::infinity || 
                        is_nan(value)
                    );
                }

                /// @brief Recovers `sin` from a half-angle tangent value via the tangent half-angle identity.
                /// @param value `tan(x / 2)` for the desired angle `x`.
                /// @return `sin(x)`.
                template <NumericType T>
                static constexpr auto sin_compute(const T& value)
                {
                    return static_cast<T>(2) * value / (static_cast<T>(1) + value * value);
                }

                /// @brief Recovers `cos` from a half-angle tangent value via the tangent half-angle identity.
                /// @param value `tan(x / 2)` for the desired angle `x`.
                /// @return `cos(x)`.
                template <NumericType T>
                static constexpr auto cos_compute(const T& value)
                {
                    return (static_cast<T>(1) - value * value) / (static_cast<T>(1) + value * value);
                }

                /// @brief Recursive continued-fraction term for the natural-logarithm series.
                /// @tparam Depth The current recursion depth.
                /// @tparam MaxDepth The recursion depth at which the series is truncated.
                /// @tparam T A numeric type.
                /// @param value The squared argument to the logarithm series.
                /// @return The partial continued-fraction denominator at `Depth`.
                template < usize Depth, usize MaxDepth, NumericType T >
                static constexpr auto log_cf_main(const T& value)
                {
                    if constexpr (Depth < MaxDepth)
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

                /// @brief Evaluates the logarithm continued fraction for `value = (x-1)/(x+1)`.
                /// @param value The transformed argument, see `log_main`.
                /// @return An approximation of `ln((1+value)/(1-value))`.
                template <NumericType T>
                static constexpr auto log_cf_begin(const T& value)
                { 
                    return static_cast<T>(2) * value / log_cf_main<1, 25>(value * value);
                }

                /// @brief Computes `ln(value)` for `value` near 1 via the identity
                /// `ln(x) = 2 * atanh((x-1)/(x+1))`.
                /// @param value The argument, expected close to 1.
                /// @return An approximation of `ln(value)`.
                template <NumericType T>
                static constexpr auto log_main(const T& value)
                { 
                    return log_cf_begin((value - static_cast<T>(1)) / (value + static_cast<T>(1)));
                }

                /// @brief Looks up `ln(value)` for small positive integers via `constants::ln_table`.
                /// @param value An integer in [2, 10]; any other value yields 0.
                /// @return `ln(value)` for `value` in [2, 10], otherwise 0.
                static constexpr f128 log_mantissa_integer(const i32& value)
                {
                    if (value > 1 && value < 11)
                    {
                        return constants::ln_table[value - 2];
                    }
                    else
                    {
                        return 0.l;
                    }
                }

                /// @brief Computes `ln(value)` for a mantissa in [1, 10] by combining a fractional
                /// logarithm (`log_main`) with a table lookup of the nearest integer's logarithm.
                /// @param value The mantissa value, in [1, 10].
                /// @return An approximation of `ln(value)`.
                template <NumericType T>
                static constexpr auto log_mantissa(const T& value)
                {   
                    return log_main(value / static_cast<T>(static_cast<i32>(value))) + 
                        static_cast<T>(log_mantissa_integer(static_cast<i32>(value)));
                }

                /// @brief Computes `ln(value)` for arbitrary positive magnitudes.
                /// @details Splits `value` into a mantissa in [1, 10] (`mantissa`) and a base-10
                /// exponent (`find_exponent`), then combines `ln(mantissa)` with `exponent * ln(10)`.
                /// @param value The input value.
                /// @return An approximation of `ln(value)`.
                template <NumericType T>
                static constexpr auto log_breakup(const T& value)
                {   
                    return log_mantissa(mantissa(value)) + 
                        static_cast<T>(constants::ln_table[8]) * 
                        static_cast<T>(find_exponent(value, 0));
                }

                /// @brief Computes `base^exp_term` for real exponents via `exp(exp_term * ln(base))`.
                /// @param base The base value.
                /// @param exp_term The exponent.
                /// @return An approximation of `base^exp_term`.
                template <NumericType T>
                static constexpr auto pow_dbl(const T& base, const T& exp_term)
                {
                    return constexpr_math::exp(exp_term * constexpr_math::log(base));
                }

                /// @brief Floating-point-exponent overload of `pow`, using `pow_dbl`.
                /// @tparam T1 A numeric base type.
                /// @tparam T2 A floating-point exponent type.
                /// @param base The base value.
                /// @param exp_term The exponent.
                /// @return `base^exp_term`, or NaN if `base` is negative.
                template <NumericType T1, FloatingPointType T2>
                static constexpr auto pow_type_check(const T1& base, const T2& exp_term)
                {
                    using ReturnType = std::common_type_t<T1, T2>;

                    if (base < static_cast<T1>(0))
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

                /// @brief Integral-exponent overload of `pow`, forwarding to `pow_integral`.
                /// @tparam T1 A numeric base type.
                /// @tparam T2 An integral exponent type.
                /// @param base The base value.
                /// @param exp_term The exponent.
                /// @return `base^exp_term`.
                template <NumericType T1, IntegralType T2>
                static constexpr auto pow_type_check(const T1& base, const T2& exp_term)
                    -> std::common_type_t<T1, T2>
                {
                    return pow_integral(base, exp_term);
                }

                /// @brief Computes a single term of the asymptotic arctangent series (Euler's series).
                /// @tparam Order The 1-based term index.
                /// @tparam T A numeric type.
                /// @param value The squared reciprocal of the arctangent argument.
                /// @param value_pow `value` raised to the power matching `Order`.
                /// @return The contribution of term `Order` to the series.
                template <usize Order, NumericType T>
                static constexpr auto atan_series_order_calc(const T& value, const T& value_pow)
                {
                    return ( 
                        static_cast<T>(1) / (static_cast<T>((Order - 1) * 4 - 1) * value_pow ) - 
                        static_cast<T>(1) / (static_cast<T>((Order - 1) * 4 + 1) * value_pow * value)
                    );
                }
                
                /// @brief Recursively accumulates the asymptotic arctangent series up to `MaxOrder` terms.
                /// @tparam Order The current 1-based term index.
                /// @tparam MaxOrder The number of terms to sum.
                /// @tparam T A numeric type.
                /// @param value For `Order == 1`, the arctangent argument; otherwise its square.
                /// @param value_pow The running power of `value` used by the current term.
                /// @return The partial sum of the series from `Order` to `MaxOrder`.
                template <usize Order, usize MaxOrder, NumericType T>
                static constexpr auto atan_series_order(const T& value, const T& value_pow)
                {
                    if constexpr (Order == 1)
                    {
                        return (
                            static_cast<T>(math::constants::pi / 2) - static_cast<T>(1) / value + 
                            atan_series_order<Order + 1, MaxOrder>(value * value, pow_type_check(value, 3))
                        );
                    }
                    else if constexpr (Order < MaxOrder)
                    {
                        return (
                            atan_series_order_calc<Order>(value, value_pow) + 
                            atan_series_order<Order + 1, MaxOrder>(value, value_pow * value * value)
                        );
                    }
                    else if constexpr (Order == MaxOrder)
                    {
                        return atan_series_order_calc<Order>(value, value_pow);
                    }
                    else
                    {
                        return limits<T>::nan;
                    }
                }
                
                /// @brief Computes `atan(value)` for `value >= 2.5` via the asymptotic series,
                /// choosing a term count that shrinks as `value` grows.
                /// @param value The (large-magnitude) argument.
                /// @return An approximation of `atan(value)`.
                template <NumericType T>
                static constexpr auto atan_series_main(const T& value)
                {
                    if (value < static_cast<T>(3))          return atan_series_order<1, 10>(value, value);
                    else if (value < static_cast<T>(4))     return atan_series_order<1, 9>(value, value);
                    else if (value < static_cast<T>(5))     return atan_series_order<1, 8>(value, value);
                    else if (value < static_cast<T>(7))     return atan_series_order<1, 7>(value, value);
                    else if (value < static_cast<T>(11))    return atan_series_order<1, 6>(value, value);
                    else if (value < static_cast<T>(25))    return atan_series_order<1, 5>(value, value);
                    else if (value < static_cast<T>(100))   return atan_series_order<1, 4>(value, value);
                    else if (value < static_cast<T>(1000))  return atan_series_order<1, 3>(value, value);
                    else                                    return atan_series_order<1, 2>(value, value);
                }
                
                /// @brief Recursive continued-fraction term for the arctangent series.
                /// @tparam Depth The current recursion depth.
                /// @tparam MaxDepth The recursion depth at which the series is truncated.
                /// @tparam T A numeric type.
                /// @param value The squared argument.
                /// @return The partial continued-fraction denominator at `Depth`.
                template <usize Depth, usize MaxDepth, NumericType T>
                static constexpr auto atan_cf_recur(const T& value)
                {
                    if constexpr (Depth < MaxDepth)
                    {
                        return static_cast<T>(2 * Depth - 1) + Depth * Depth * 
                        value / atan_cf_recur<Depth + 1, MaxDepth>(value);
                    }
                    else
                    {
                        return static_cast<T>(2 * Depth - 1);
                    }           
                }
                
                /// @brief Computes `atan(value)` for `value < 2.5` via a continued fraction,
                /// choosing a term count that grows with `value`.
                /// @param value The (small-magnitude) argument.
                /// @return An approximation of `atan(value)`.
                template <NumericType T>
                static constexpr auto atan_cf_main(const T& value)
                {
                    if (value < static_cast<T>(0.5))         return value / atan_cf_recur<1, 15>(value * value); 
                    else if (value < static_cast<T>(1))      return value / atan_cf_recur<1, 25>(value * value); 
                    else if (value < static_cast<T>(1.5))    return value / atan_cf_recur<1, 35>(value * value); 
                    else if (value < static_cast<T>(2))      return value / atan_cf_recur<1, 45>(value * value); 
                    else                                     return value / atan_cf_recur<1, 52>(value * value);
                }
                
                /// @brief Dispatches to the series or continued-fraction arctangent approximation
                /// based on the magnitude of `value`.
                /// @param value The (non-negative) argument.
                /// @return An approximation of `atan(value)`.
                template <NumericType T>
                static constexpr auto atan_begin(const T& value)
                {
                    if (value > static_cast<T>(2.5))
                    {
                        return static_cast<T>(atan_series_main(value));
                    }
                    else
                    {
                        return static_cast<T>(atan_cf_main(value));
                    }
                }
                
                /// @brief Handles special/edge cases of `atan(value)` before delegating to `atan_begin`.
                /// @param value The argument.
                /// @return `value` if NaN; 0 if `value` is within epsilon of 0; the negation of
                /// `atan_begin(-value)` for negative arguments; otherwise `atan_begin(value)`.
                template <FloatingPointType T>
                static constexpr auto atan_check(const T& value)
                {
                    if (is_nan(value) == true)
                    {
                        return value;
                    }
                    else if (limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(0);
                    }
                    else if (value < static_cast<T>(0))
                    {
                        return -atan_begin(-value);
                    }
                    else
                    {
                        return atan_begin(value);
                    }
                }

                /// @brief Computes `asin(value)` for non-negative `value` via `atan(x / sqrt(1-x^2))`.
                /// @param value The argument, expected non-negative and at most 1.
                /// @return NaN if `value > 1`; π/2 if `value` is within epsilon of 1; 0 if within
                /// epsilon of 0; otherwise an approximation of `asin(value)`.
                template <FloatingPointType T>
                static constexpr auto asin_compute(const T& value)
                {
                    if (value > static_cast<T>(1))
                    {
                        return limits<T>::nan;
                    }
                    else if (limits<T>::epsilon > abs(value - static_cast<T>(1)))
                    {
                        return static_cast<T>(math::constants::pi / 2);
                    }
                    else if (limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(0);
                    }
                    else
                    {
                        return atan(value / sqrt(static_cast<T>(1) - value * value));
                    }
                }

                /// @brief Handles the sign of `value` before delegating to `asin_compute`.
                /// @param value The argument.
                /// @return `value` if NaN; the negation of `asin_compute(-value)` for negative
                /// arguments; otherwise `asin_compute(value)`.
                template <FloatingPointType T>
                static constexpr auto asin_check(const T& value)
                {
                    if (is_nan(value) == true)
                    {
                        return value;
                    }
                    else if (value < static_cast<T>(0))
                    {
                        return -asin_compute(-value);
                    }
                    else
                    {
                        return asin_compute(value);
                    }
                }

                /// @brief Computes `acos(value)` via `atan(sqrt(1-x^2) / x)`.
                /// @param value The argument, in [-1, 1].
                /// @return NaN if `|value| > 1`; 0 if `value` is within epsilon of 1; π/2 if
                /// within epsilon of 0; otherwise an approximation of `acos(value)`.
                template <FloatingPointType T>
                static constexpr auto acos_compute(const T& value)
                {
                    if (abs(value) > static_cast<T>(1))
                    {
                        return limits<T>::nan;
                    }
                    else if (limits<T>::epsilon > abs(value - static_cast<T>(1)))
                    {
                        return static_cast<T>(0);
                    }
                    else if (limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(math::constants::pi / 2);
                    }
                    else
                    {
                        return atan(sqrt(static_cast<T>(1) - value * value) / value);
                    }
                }

                /// @brief Handles the sign of `value` before delegating to `acos_compute`.
                /// @param value The argument.
                /// @return `value` if NaN; `acos_compute(value)` for positive arguments; otherwise
                /// `π/2 - acos_compute(-value)`.
                template <FloatingPointType T>
                static constexpr auto acos_check(const T& value)
                {
                    using math::constants::pi;

                    if (is_nan(value) == true)
                    {
                        return value;
                    }
                    else if (value > static_cast<T>(0))
                    {
                        return acos_compute(value);
                    }
                    else
                    {
                        return static_cast<T>(pi / 2) - 
                            acos_compute(-value);
                    }
                }

                /// @brief Computes the two-argument arctangent `atan2(y, x)`, handling all
                /// quadrant and axis-aligned special cases (including signed zeros).
                /// @param y The y-coordinate.
                /// @param x The x-coordinate.
                /// @return The angle, in radians, of the point `(x, y)` relative to the positive
                /// x-axis, in the range (-π, π]; NaN if either argument is NaN.
                template <FloatingPointType T>
                static constexpr auto atan2_compute(const T& y, const T& x)
                {
                    using math::constants::pi;

                    if (is_nan(y) || is_nan(x))
                    {
                        return limits<T>::nan;
                    }
                    else if (limits<T>::epsilon > abs(x))
                    {
                        if (limits<T>::epsilon > abs(y))
                        {
                            if (y == static_cast<T>(0) && sign_bit(y))
                            {
                                if (x == static_cast<T>(0) && sign_bit(x))
                                {
                                    return -static_cast<T>(pi / 2);
                                }
                                else
                                {
                                    return -static_cast<T>(0);
                                }
                            }
                            else if (x == static_cast<T>(0) && sign_bit(x))
                            {
                                return static_cast<T>(pi / 2);
                            }
                            else
                            {
                                return static_cast<T>(0);
                            }
                        }
                        else if (y > static_cast<T>(0))
                        {
                            return static_cast<T>(pi / 2);
                        }
                        else
                        {
                            return -static_cast<T>(pi / 2);
                        }
                    }
                    else if (x < static_cast<T>(0))
                    {
                        if (y < static_cast<T>(0))
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

                /// @brief Common-type overload of `atan2`, converting both arguments before
                /// delegating to `atan2_compute`.
                /// @tparam T1 A floating-point type.
                /// @tparam T2 A floating-point type.
                /// @param y The y-coordinate.
                /// @param x The x-coordinate.
                /// @return `atan2(y, x)` computed in the common type of `T1` and `T2`.
                template <FloatingPointType T1, FloatingPointType T2>
                static constexpr auto atan2_type_check(const T1& y, const T2& x)
                {
                    using RetType = std::common_type_t<T1, T2>;

                    return atan2_compute(
                        static_cast<RetType>(y), 
                        static_cast<RetType>(x)
                    );
                }

                /// @brief Computes `sinh(value) = (e^x - e^-x) / 2`, handling NaN and near-zero inputs.
                /// @param value The argument.
                /// @return `value` if NaN; 0 if within epsilon of 0; otherwise `sinh(value)`.
                template <FloatingPointType T>
                static constexpr auto sinh_check(const T& value)
                {
                    if (is_nan(value) == true)
                    {
                        return value;
                    }
                    else if (limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(0);
                    }
                    else
                    {
                        return (exp(value) - exp(-value)) / static_cast<T>(2);
                    }
                }

                /// @brief Computes `cosh(value) = (e^x + e^-x) / 2`, handling NaN and near-zero inputs.
                /// @param value The argument.
                /// @return `value` if NaN; 1 if within epsilon of 0; otherwise `cosh(value)`.
                template <FloatingPointType T>
                static constexpr auto cosh_check(const T& value)
                {
                    if (is_nan(value) == true)
                    {
                        return value;
                    }
                    else if (limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(1);
                    }
                    else
                    {
                        return (exp(value) + exp(-value)) / static_cast<T>(2);
                    }
                }

                /// @brief Recursive continued-fraction term for the hyperbolic tangent series.
                /// @tparam Depth The current recursion depth (truncated at 25).
                /// @tparam T A floating-point type.
                /// @param value The squared argument.
                /// @return The partial continued-fraction denominator at `Depth`.
                template <usize Depth, FloatingPointType T>
                static constexpr auto tanh_cf(const T value)
                {
                    if constexpr (Depth < 25)
                    {
                        return (2 * Depth - 1) + value / tanh_cf<Depth + 1>(value);
                    }
                    else
                    {
                        return static_cast<T>(2 * Depth - 1);
                    }
                }

                /// @brief Computes `tanh(value)` via the continued fraction in `tanh_cf`.
                /// @param value The argument.
                /// @return An approximation of `tanh(value)`.
                template <FloatingPointType T>
                static constexpr auto tanh_begin(const T& value)
                {
                    return value / tanh_cf<1>(value * value);
                }

                /// @brief Handles NaN/near-zero/sign special cases before delegating to `tanh_begin`.
                /// @param value The argument.
                /// @return `value` if NaN; 0 if within epsilon of 0; the negation of
                /// `tanh_begin(-value)` for negative arguments; otherwise `tanh_begin(value)`.
                template <FloatingPointType T>
                static constexpr auto tanh_check(const T& value)
                {
                    if (is_nan(value) == true)
                    {
                        return value;
                    }
                    else if (limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(0);
                    }
                    else if (value < static_cast<T>(0))
                    {
                        return -tanh_begin(-value);
                    }
                    else
                    {
                        return tanh_begin(value);
                    }
                }

                /// @brief Computes `asinh(value) = ln(value + sqrt(value^2 + 1))`.
                /// @param value The argument.
                /// @return `value` if NaN; 0 if within epsilon of 0; otherwise `asinh(value)`.
                template <FloatingPointType T>
                static constexpr auto asinh_compute(const T& value)
                {
                    if (is_nan(value) == true)
                    {
                        return value;
                    }
                    else if (limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(0);
                    }
                    else
                    {
                        return log(value + sqrt(value * value + static_cast<T>(1)));
                    }
                }

                /// @brief Computes `acosh(value) = ln(value + sqrt(value^2 - 1))`.
                /// @param value The argument, expected `>= 1`.
                /// @return `value` if NaN; 0 if within epsilon of 1; otherwise `acosh(value)`.
                template <FloatingPointType T>
                static constexpr auto acosh_compute(const T& value)
                {
                    if (is_nan(value) == true)
                    {
                        return value;
                    }
                    else if (limits<T>::epsilon > abs(value - static_cast<T>(1)))
                    {
                        return static_cast<T>(0);
                    }
                    else
                    {
                        return log(value + sqrt(value * value - static_cast<T>(1)));
                    }
                }

                
                /// @brief Computes `atanh(value) = ln((1+value)/(1-value)) / 2`, without edge-case handling.
                /// @param value The argument, in (-1, 1).
                /// @return `atanh(value)`.
                template <FloatingPointType T>
                static constexpr auto atanh_compute(const T& value)
                {
                    return log(
                        (static_cast<T>(1) + value) / 
                        (static_cast<T>(1) - value)
                    ) / static_cast<T>(2);
                }

                /// @brief Handles NaN/domain/infinity/near-zero special cases before delegating
                /// to `atanh_compute`.
                /// @param value The argument.
                /// @return `value` if NaN; NaN if `|value| > 1`; a signed infinity if `|value|`
                /// is within epsilon of 1; 0 if within epsilon of 0; otherwise `atanh(value)`.
                template <FloatingPointType T>
                static constexpr auto atanh_check(const T& value)
                {
                    if (is_nan(value) == true)
                    {
                        return value;
                    }
                    else if (static_cast<T>(1) < abs(value))
                    {
                        return limits<T>::nan;
                    }
                    else if (limits<T>::epsilon > (static_cast<T>(1) - abs(value)))
                    {
                        return sgn(value) * limits<T>::infinity; 
                    }
                    else if (limits<T>::epsilon > abs(value))
                    {
                        return static_cast<T>(0);
                    }
                    else
                    {
                        return atanh_compute(value);
                    }
                        
                }

                /// @brief Evaluates the Lanczos approximation's rational coefficient series.
                /// @param value The shifted argument (`x - 1`), see `lgamma_begin`.
                /// @return The Lanczos series sum used by `lgamma_term_2`.
                template <FloatingPointType T>
                static constexpr auto lgamma_coeff_term(const T& value)
                {
                    return (     
                        0.99999999999999709182l                    + 
                        57.156235665862923517l     / (value + 1)   -
                        59.597960355475491248l     / (value + 2)   + 
                        14.136097974741747174l     / (value + 3)   -
                        0.49191381609762019978l    / (value + 4)   + 
                        0.33994649984811888699e-4l / (value + 5)   +
                        0.46523628927048575665e-4l / (value + 6)   - 
                        0.98374475304879564677e-4l / (value + 7)   +
                        0.15808870322491248884e-3l / (value + 8)   - 
                        0.21026444172410488319e-3l / (value + 9)   +
                        0.21743961811521264320e-3l / (value + 10)  - 
                        0.16431810653676389022e-3l / (value + 11)  +
                        0.84418223983852743293e-4l / (value + 12)  -
                        0.26190838401581408670e-4l / (value + 13)  +
                        0.36899182659531622704e-5L / (value + 14) 
                    );  
                }
                
                /// @brief Computes the logarithmic correction term of the Lanczos approximation.
                /// @param value The shifted argument (`x - 1`), see `lgamma_begin`.
                /// @return `ln(sqrt(π)) + ln(lgamma_coeff_term(value))`.
                template <FloatingPointType T>
                static constexpr auto lgamma_term_2(const T& value)
                {
                    return (
                        static_cast<T>(constants::log_sqrt_pi) + 
                        log(static_cast<T>(lgamma_coeff_term(value)))
                    );
                }
                
                /// @brief Computes the leading power term of the Lanczos approximation.
                /// @param value The shifted argument (`x - 1`), see `lgamma_begin`.
                /// @return `(value + 0.5) * ln(value + g) - (value + g)` for the Lanczos constant `g`.
                template <NumericType T>
                static constexpr auto lgamma_term_1(const T& value)
                {   
                    constexpr f128 _key_gamma = 5.2421875l;

                    return ( 
                        (value + static_cast<T>(0.5l)) * 
                        log(value + static_cast<T>(_key_gamma)) - 
                        (value + static_cast<T>(_key_gamma))
                    );
                }
                
                /// @brief Combines the two Lanczos-approximation terms into `lgamma(value + 1)`.
                /// @param value The shifted argument (`x - 1`), where `x` is the original lgamma input.
                /// @return An approximation of `ln(Γ(value + 1))`.
                template <FloatingPointType T>
                static constexpr auto lgamma_begin(const T& value)
                {   
                    return lgamma_term_1(value) + lgamma_term_2(value);
                }
                
                /// @brief Handles NaN/near-1/near-0 special cases before delegating to `lgamma_begin`.
                /// @param value The argument.
                /// @return `value` if NaN; 0 if within epsilon of 1; +infinity if within epsilon
                /// of 0; otherwise `lgamma_begin(value - 1)`.
                template <FloatingPointType T>
                static constexpr auto lgamma_check(const T& value)
                {
                    if (is_nan(value) == true)
                    {
                        return value;
                    }
                    else if (limits<T>::epsilon > abs(value - static_cast<T>(1)))
                    {
                        return static_cast<T>(0);
                    }
                    else if (limits<T>::epsilon > value)
                    {
                        return limits<T>::infinity;
                    }
                    else
                    {
                        return lgamma_begin(value - static_cast<T>(1));
                    }
                }

                /// @brief Computes `tgamma(value)` (the gamma function), handling NaN, poles,
                /// negative arguments (via the reflection/recurrence relation), and factorial
                /// fast paths for positive integers.
                /// @param value The argument.
                /// @return `value` if NaN; 1 if within epsilon of 1; +infinity if within epsilon
                /// of 0; NaN at negative integer poles; `(value)!` for positive integers;
                /// otherwise `exp(lgamma(value))`.
                template <FloatingPointType T>
                static constexpr auto tgamma_check(const T& value)
                {
                    if (is_nan(value) == true)
                    {
                        return value;
                    }
                    else if (limits<T>::epsilon > abs(value - static_cast<T>(1)))
                    {
                        return static_cast<T>(1);
                    }
                    else if (limits<T>::epsilon > abs(value))
                    {
                        return limits<T>::infinity;
                    }
                    else if (value < static_cast<T>(0))
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
                        if (static_cast<u64>(value) == value)
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

                /// @brief `f32` overload: truncates towards zero via an `i32` round-trip.
                /// @param x The input value.
                /// @return `x` unchanged if `|x|` is at or beyond the exact-integer threshold
                /// (2^23), otherwise `x` truncated towards zero.
                static constexpr f32 trunc_check(f32 x)
                {
                    constexpr f32 _threshold = 8388608.f;

                    if (abs(x) >= _threshold)
                    {
                        return x;
                    }
                    else
                    {
                        return static_cast<f32>(static_cast<i32>(x));
                    }
                }

                /// @brief `f64` overload: truncates towards zero via an `i64` round-trip.
                /// @param x The input value.
                /// @return `x` unchanged if `|x|` is at or beyond the exact-integer threshold
                /// (2^52), otherwise `x` truncated towards zero.
                static constexpr f64 trunc_check(f64 x)
                {
                    constexpr f64 _threshold = 4503599627370496.;

                    if (abs(x) >= _threshold)
                    {
                        return x;
                    }
                    else
                    {
                        return static_cast<f64>(static_cast<i64>(x));
                    }
                }

                /// @brief `f128` overload: truncates towards zero via a `u64` round-trip.
                /// @param x The input value.
                /// @return `x` unchanged if `|x|` is at or beyond the exact-integer threshold
                /// (2^63), otherwise `x` truncated towards zero.
                static constexpr f128 trunc_check(f128 x)
                {
                    constexpr f128 _threshold = 9223372036854775808.l;

                    if (abs(x) >= _threshold)
                    {
                        return x;
                    }
                    else
                    {
                        return static_cast<u64>(abs(x)) * ((x < 0) ? -1.l : 1.l);
                    }
                }
            } // namespace ctmath_detail

            template <NumericType T>
            static constexpr auto factorial(usize size)
            {
                T _result = 1;

                for (usize _index = size; _index >= 2; _index--)
                    _result *= static_cast<T>(_index);

                return _result;
            }

            /// @brief Truncates `value` towards zero.
            /// @tparam T A floating-point type.
            /// @param value The input value.
            /// @return `value` unchanged for NaN/±infinity, otherwise `value` truncated towards zero.
            template <FloatingPointType T>
            static constexpr auto trunc(const T& value)
            {
                if (ctmath_detail::invalid_case(value))
                {
                    return value;
                }
                else
                {
                    return ctmath_detail::trunc_check(value);
                }
            }

            /// @brief Computes the floating-point remainder of `value / factor`.
            /// @tparam T A floating-point type.
            /// @param value The dividend.
            /// @param factor The divisor.
            /// @return `value` unchanged for NaN/±infinity, otherwise `value - trunc(value/factor) * factor`.
            template <FloatingPointType T>
            static constexpr auto mod(const T& value, const T& factor)
            {
                if (ctmath_detail::invalid_case(value))
                {
                    return value;
                }
                else
                {
                    auto _res = trunc(value / factor);

                    return value - _res * factor;
                }
            }

            template <FloatingPointType T>
            static constexpr auto floor(const T& value)
            {
                if (ctmath_detail::invalid_case(value))
                {
                    return value;
                }
                else
                {
                    auto _res = trunc(value);

                    return _res - (value < 0 && (value - _res) != 0);
                }
            }

            /// @brief Rounds `value` up to the nearest integer.
            /// @tparam T A floating-point type.
            /// @param value The input value.
            /// @return The smallest integral value not less than `value`; NaN/Inf are passed through.
            template <FloatingPointType T>
            static constexpr auto ceil(const T& value)
            {
                if (ctmath_detail::invalid_case(value))
                {
                    return value;
                }
                else
                {
                    auto _res = trunc(value);

                    return _res + (value > 0 && (value - _res) != 0);
                }
            }

            /// @brief Rounds `value` to the nearest integer, halfway cases rounding away from zero.
            /// @tparam T A floating-point type.
            /// @param value The input value.
            /// @return The nearest integral value to `value`; NaN/Inf are passed through.
            template <FloatingPointType T>
            static constexpr auto round(const T& value)
            {
                if (ctmath_detail::invalid_case(value))
                {
                    return value;
                }
                else
                {
                    return (
                        floor(value) + (value > 0 && constexpr_math::mod(value, static_cast<T>(1)) >= 0.5) + 
                        (
                            value < 0 && constexpr_math::mod(1 + 
                                constexpr_math::mod(value, static_cast<T>(1)), static_cast<T>(1)) >= 0.5
                        )
                    );
                }
            }

            /// @brief Computes the tangent of `value`, in radians.
            /// @tparam T A numeric type.
            /// @param value The input value.
            /// @return `value` if NaN; 0 if within epsilon of 0; otherwise `tan(value)`.
            template <NumericType T>
            static constexpr auto tan(T value)
            {
                if (is_nan(value) == true)
                {
                    return value;
                }
                else if (limits<T>::epsilon > math::abs(value))
                {
                    return static_cast<T>(0);
                }
                else
                {
                    return value < 0 ? -ctmath_detail::tan_begin(-value) : ctmath_detail::tan_begin(value);
                }
            }

            /// @brief Computes the sine of `value`, in radians.
            /// @details Reduces to the half-angle tangent via the tangent half-angle identity,
            /// after handling NaN and the axis-aligned special angles (0, ±π/2, ±π).
            /// @tparam T A numeric type.
            /// @param value The input value.
            /// @return `sin(value)`.
            template <NumericType T>
            static constexpr auto sin(T value)
            {
                using constants::pi;

                if (is_nan(value) == true)
                {
                    return value;
                }
                else if (limits<T>::epsilon > math::abs(value))
                {
                    return static_cast<T>(0);
                }
                else if (limits<T>::epsilon > math::abs(value - static_cast<T>(pi / 2)))
                {
                    return static_cast<T>(1);
                }
                else if (limits<T>::epsilon > math::abs(value + static_cast<T>(pi / 2)))
                {
                    return -static_cast<T>(1);
                }
                else if (limits<T>::epsilon > math::abs(value - static_cast<T>(pi)))
                {
                    return static_cast<T>(0);
                }
                else if (limits<T>::epsilon > math::abs(value + static_cast<T>(pi)))
                {
                    return -static_cast<T>(0);
                }
                else
                {
                    return ctmath_detail::sin_compute(constexpr_math::tan(value / static_cast<T>(2)));
                }
            }

            /// @brief Computes the cosine of `value`, in radians.
            /// @details Reduces to the half-angle tangent via the tangent half-angle identity,
            /// after handling NaN and the axis-aligned special angles (0, ±π/2, ±π).
            /// @tparam T A numeric type.
            /// @param value The input value.
            /// @return `cos(value)`.
            template <NumericType T>
            static constexpr auto cos(T value)
            {
                using constants::pi;

                if (is_nan(value) == true)
                {
                    return value;
                }
                else if (limits<T>::epsilon > math::abs(value))
                {
                    return static_cast<T>(1);
                }
                else if (limits<T>::epsilon > math::abs(value - static_cast<T>(pi / 2)))
                {
                    return static_cast<T>(0);
                }
                else if (limits<T>::epsilon > math::abs(value + static_cast<T>(pi / 2)))
                {
                    return static_cast<T>(0);
                }
                else if (limits<T>::epsilon > math::abs(value - static_cast<T>(pi)))
                {
                    return -static_cast<T>(1);
                }
                else if (limits<T>::epsilon > math::abs(value + static_cast<T>(pi)))
                {
                    return -static_cast<T>(1);
                }
                else
                {
                    return ctmath_detail::cos_compute(constexpr_math::tan(value / static_cast<T>(2)));
                }
            }

            template <NumericType T>
            static constexpr auto log(T value)
            {
                if (is_nan(value) == true || value < static_cast<T>(0))
                {
                    return limits<T>::nan;
                }
                else if (limits<T>::epsilon > value)
                {
                    return -limits<T>::infinity;
                }
                else if (limits<T>::epsilon > math::abs(value - static_cast<T>(1)))
                {
                    return static_cast<T>(0);
                }
                else if (value == limits<T>::infinity)
                {
                    return limits<T>::infinity;
                }
                else
                {
                    if (value < static_cast<T>(0.5) || value > static_cast<T>(1.5))
                    {
                        return ctmath_detail::log_breakup(value);
                    }
                    else
                    {
                        return ctmath_detail::log_main(value);
                    }
                }  
            }

            /// @brief Computes the base-2 logarithm of `value`.
            /// @tparam T A numeric type.
            /// @param value The input value.
            /// @return `log(value) / ln(2)`; NaN for negative inputs, -infinity for zero.
            template <NumericType T>
            static constexpr auto log2(T value)
            {
                if (is_nan(value) == true || value < static_cast<T>(0))
                {
                    return limits<T>::nan;
                }
                else if (limits<T>::epsilon > value)
                {
                    return -limits<T>::infinity;
                }
                else if (limits<T>::epsilon > math::abs(value - static_cast<T>(1)))
                {
                    return static_cast<T>(0);
                }
                else if (value == limits<T>::infinity)
                {
                    return limits<T>::infinity;
                }
                else
                {
                    return log(value) / static_cast<T>(constants::ln_table[0]);
                }  
            }

            /// @brief Computes the base-10 logarithm of `value`.
            /// @tparam T A numeric type.
            /// @param value The input value.
            /// @return `log(value) / ln(10)`; NaN for negative inputs, -infinity for zero.
            template <NumericType T>
            static constexpr auto log10(T value)
            {
                if (is_nan(value) == true || value < static_cast<T>(0))
                {
                    return limits<T>::nan;
                }
                else if (limits<T>::epsilon > value)
                {
                    return -limits<T>::infinity;
                }
                else if (limits<T>::epsilon > math::abs(value - static_cast<T>(1)))
                {
                    return static_cast<T>(0);
                }
                else if (value == limits<T>::infinity)
                {
                    return limits<T>::infinity;
                }
                else
                {
                    return log(value) / static_cast<T>(constants::ln_table[8]);
                }  
            }

            template <NumericType T>
            static constexpr auto exp(T value)
            {
                if (is_nan(value) == true || value == limits<T>::infinity)
                {
                    return value;
                }
                else if (value == -limits<T>::infinity)
                {
                    return static_cast<T>(0);
                }
                else if (limits<T>::epsilon > math::abs(value))
                {
                    return static_cast<T>(1);
                }
                else if (math::abs(value) < static_cast<T>(2))
                {
                    return static_cast<T>(ctmath_detail::exp_cf(value));
                }
                else
                {
                    return static_cast<T>(ctmath_detail::exp_split(value));
                }
            }

            /// @brief Computes `base` raised to the power `exp_term`.
            /// @tparam T1 A numeric base type.
            /// @tparam T2 A numeric exponent type.
            /// @param base The base value.
            /// @param exp_term The exponent.
            /// @return `base^exp_term`; integral exponents use exponentiation by squaring, real
            /// exponents use `exp(exp_term * log(base))` and return NaN for negative `base`.
            template <NumericType T1, NumericType T2>
            static constexpr auto pow(T1 base, T2 exp_term)
            {
                return ctmath_detail::pow_type_check(base, exp_term);
            }

            /// @brief Computes the square root of `value` via 10 iterations of Newton's method.
            /// @tparam T A floating-point type.
            /// @param value The input value.
            /// @return The non-negative square root of `value`, or NaN if `value` is negative.
            template <FloatingPointType T>
            static constexpr auto sqrt(const T& value)
            {
                auto _mid_val = static_cast<T>(1.0);

                if (value <= static_cast<T>(-0.0))
                {
                    return limits<T>::nan;
                }
                for (usize _count = 0; _count < 10; ++_count)
                {
                    const auto _euler = value / _mid_val;
                    _mid_val = midpoint(_mid_val, _euler);
                }

                return _mid_val;
            }

            /// @brief Computes the arctangent of `value`, in radians.
            /// @tparam T A numeric type.
            /// @param value The input value.
            /// @return The arctangent of `value`, in the range [-π/2, π/2].
            template <NumericType T>
            static constexpr auto atan(T value)
            {
                return ctmath_detail::atan_check(value);
            }

            /// @brief Computes the arcsine of `value`, in radians.
            /// @tparam T A numeric type.
            /// @param value The input value, expected in [-1, 1].
            /// @return The arcsine of `value`, in the range [-π/2, π/2]; NaN if `|value| > 1`.
            template <NumericType T>
            static constexpr auto asin(T value)
            {
                return ctmath_detail::asin_check(value);
            }

            /// @brief Computes the arccosine of `value`, in radians.
            /// @tparam T A numeric type.
            /// @param value The input value, expected in [-1, 1].
            /// @return The arccosine of `value`, in the range [0, π]; NaN if `|value| > 1`.
            template <NumericType T>
            static constexpr auto acos(T value)
            {
                return ctmath_detail::acos_check(value);
            }

            /// @brief Computes the two-argument arctangent `atan2(y, x)`, in radians.
            /// @tparam T1 A numeric type.
            /// @tparam T2 A numeric type.
            /// @param y The y-coordinate.
            /// @param x The x-coordinate.
            /// @return The angle of the point `(x, y)` relative to the positive x-axis, in (-π, π].
            template <NumericType T1, NumericType T2>
            static constexpr auto atan2(const T1& y, const T2& x)
            {
                return ctmath_detail::atan2_type_check(y, x);
            }

            /// @brief Computes the hyperbolic sine of `value`.
            /// @tparam T A numeric type.
            /// @param value The input value.
            /// @return `sinh(value)`.
            template <NumericType T>
            static constexpr auto sinh(const T& value)
            {
                return ctmath_detail::sinh_check(value);
            }

            /// @brief Computes the hyperbolic cosine of `value`.
            /// @tparam T A numeric type.
            /// @param value The input value.
            /// @return `cosh(value)`.
            template <NumericType T>
            static constexpr auto cosh(const T& value)
            {
                return ctmath_detail::cosh_check(value);
            }

            /// @brief Computes the hyperbolic tangent of `value`.
            /// @tparam T A numeric type.
            /// @param value The input value.
            /// @return `tanh(value)`.
            template <NumericType T>
            static constexpr auto tanh(const T& value)
            {
                return ctmath_detail::tanh_check(value);
            }

            /// @brief Computes the inverse hyperbolic sine of `value`.
            /// @tparam T A numeric type.
            /// @param value The input value.
            /// @return `asinh(value)`.
            template <NumericType T>
            static constexpr auto asinh(const T& value)
            {
                return ctmath_detail::asinh_compute(value);
            }

            /// @brief Computes the inverse hyperbolic cosine of `value`.
            /// @tparam T A numeric type.
            /// @param value The input value, expected `>= 1`.
            /// @return `acosh(value)`.
            template <NumericType T>
            static constexpr auto acosh(const T& value)
            {
                return ctmath_detail::acosh_compute(value);
            }

            /// @brief Computes the inverse hyperbolic tangent of `value`.
            /// @tparam T A numeric type.
            /// @param value The input value, expected in (-1, 1).
            /// @return `atanh(value)`; NaN if `|value| > 1`, a signed infinity if `|value| == 1`.
            template <NumericType T>
            static constexpr auto atanh(const T& value)
            {
                return ctmath_detail::atanh_check(value);
            }

            template <NumericType T>
            static constexpr auto lgamma(const T& value)
            {
                return ctmath_detail::lgamma_check(value);
            }

            /// @brief Computes the gamma function of `value`.
            /// @tparam T A numeric type.
            /// @param value The input value.
            /// @return `Γ(value)`; uses the factorial fast path for positive integers, the
            /// reflection/recurrence relation for negative values, and NaN at negative-integer poles.
            template <NumericType T>
            static constexpr auto tgamma(const T& value)
            {
                return ctmath_detail::tgamma_check(value);
            }

            /// @brief Computes the natural logarithm of the beta function, `ln(B(a, b))`.
            /// @tparam T1 A numeric type.
            /// @tparam T2 A numeric type.
            /// @param a The first shape parameter.
            /// @param b The second shape parameter.
            /// @return `lgamma(a) + lgamma(b) - lgamma(a + b)`.
            template <NumericType T1, NumericType T2>
            static constexpr auto lbeta(const T1& a, const T2& b)
            {
                using RetType = std::common_type_t<T1, T2>;
                return static_cast<RetType>(
                    (lgamma(a) + lgamma(b)) - lgamma(a + b)
                );
            }

            /// @brief Computes the beta function, `B(a, b) = Γ(a)Γ(b) / Γ(a+b)`.
            /// @tparam T1 A numeric type.
            /// @tparam T2 A numeric type.
            /// @param a The first shape parameter.
            /// @param b The second shape parameter.
            /// @return `exp(lbeta(a, b))`.
            template <NumericType T1, NumericType T2>
            static constexpr auto beta(const T1& a, const T2& b)
            {
                return exp(lbeta(a, b));
            }
        } // namespace constexpr_math

        /// @brief Definition of `math::midpoint` for `Middleable` value types.
        /// @details Integral types compute the midpoint using unsigned arithmetic on the
        /// difference to avoid signed overflow. Floating-point types avoid overflow by
        /// halving each operand first when both would otherwise overflow, and avoid
        /// underflow to zero by only halving one operand when the other is subnormal.
        /// @param a First value.
        /// @param b Second value.
        /// @return Overflow-safe midpoint of `a` and `b`.
        template <Middleable T>
        constexpr auto midpoint(T a, T b)
        {
            if constexpr (is_integral<T>::value)
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

                if (_abs_a <= _high_limit && _abs_b <= _high_limit)
                {
                    return (a + b) / 2;
                }
                else if (_abs_a < _low_limit)
                {
                    return a + b / 2;
                }
                else if (_abs_b < _low_limit)
                {
                    return a / 2 + b;
                }
                else
                {
                    return a / 2 + b / 2;
                }
            }
        }

        /// @brief Definition of `math::midpoint` for pointers into the same array.
        /// @details Computed as `a + (b - a) / 2`, relying on well-defined pointer subtraction.
        /// @param a First pointer.
        /// @param b Second pointer.
        /// @return Pointer halfway between `a` and `b`.
        template <ObjectType T>
        constexpr T* midpoint(T* a, T* b)
        {
            static_assert(sizeof(T) != 0, "type must be complete");
            return a + (b - a) / 2;
        }

        /// @brief Computes the floating-point remainder of `value / factor`.
        /// @tparam T A floating-point type.
        /// @param value The dividend.
        /// @param factor The divisor.
        /// @return The constexpr implementation's result during constant evaluation,
        /// otherwise `std::fmod(value, factor)`.
        template <FloatingPointType T>
        static constexpr auto mod(const T& value, const T& factor)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::mod(value, factor);
            }
            else
            {
                return std::fmod(value, factor);
            }
        }

        /// @brief Truncates `value` towards zero.
        /// @tparam T A floating-point type.
        /// @param value The input value.
        /// @return The constexpr implementation's result during constant evaluation,
        /// otherwise `std::trunc(value)`.
        template <FloatingPointType T>
        static constexpr auto trunc(const T& value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::trunc(value);
            }
            else
            {
                return std::trunc(value);
            }
        }

        /// @brief Rounds `value` down to the nearest integer.
        /// @tparam T A floating-point type.
        /// @param value The input value.
        /// @return The constexpr implementation's result during constant evaluation,
        /// otherwise `std::floor(value)`.
        template <FloatingPointType T>
        static constexpr auto floor(const T& value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::floor(value);
            }
            else
            {
                return std::floor(value);
            }
        }

        /// @brief Rounds `value` up to the nearest integer.
        /// @tparam T A floating-point type.
        /// @param value The input value.
        /// @return The constexpr implementation's result during constant evaluation,
        /// otherwise `std::ceil(value)`.
        template <FloatingPointType T>
        static constexpr auto ceil(const T& value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::ceil(value);
            }
            else
            {
                return std::ceil(value);
            }
        }

        /// @brief Rounds to the nearest integer, delegating to constexpr math during constant evaluation.
        /// @param value Floating-point input.
        /// @return Rounded value.
        template <FloatingPointType T>
        static constexpr auto round(const T& value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::round(value);
            }
            else
            {
                return std::round(value);
            }
        }

        /// @brief Computes tangent, using constexpr math during constant evaluation.
        /// @param value Numeric angle in radians.
        /// @return Tangent of `value`.
        template <NumericType T>
        static constexpr auto tan(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::tan(value);
            }
            else
            {
                return std::tan(value);
            }
        }

        /// @brief Computes sine, using constexpr math during constant evaluation.
        /// @param value Numeric angle in radians.
        /// @return Sine of `value`.
        template <NumericType T>
        static constexpr auto sin(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::sin(value);
            }
            else
            {
                return std::sin(value);
            }
        }

        /// @brief Computes cosine, using constexpr math during constant evaluation.
        /// @param value Numeric angle in radians.
        /// @return Cosine of `value`.
        template <NumericType T>
        static constexpr auto cos(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::cos(value);
            }
            else
            {
                return std::cos(value);
            }
        }

        template <NumericType T>
        static constexpr auto atan(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::atan(value);
            }
            else
            {
                return std::atan(value);
            }
        }

        /// @brief Computes inverse sine, using constexpr math during constant evaluation.
        /// @param value Numeric input expected in `[-1, 1]`.
        /// @return Inverse sine of `value`.
        template <NumericType T>
        static constexpr auto asin(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::asin(value);
            }
            else
            {
                return std::asin(value);
            }
        }

        /// @brief Computes inverse cosine, using constexpr math during constant evaluation.
        /// @param value Numeric input expected in `[-1, 1]`.
        /// @return Inverse cosine of `value`.
        template <NumericType T>
        static constexpr auto acos(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::acos(value);
            }
            else
            {
                return std::acos(value);
            }
        }

        /// @brief Computes the two-argument arctangent.
        /// @param y Y coordinate.
        /// @param x X coordinate.
        /// @return Angle of `(x, y)` in radians.
        template <NumericType T1, NumericType T2>
        static constexpr auto atan2(const T1 y, const T2 x)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::atan2(y, x);
            }
            else
            {
                return std::atan2(y, x);
            }
        }

        /// @brief Computes the natural logarithm.
        /// @param value Numeric input.
        /// @return Natural logarithm of `value`.
        template <NumericType T>
        static constexpr auto log(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::log(value);
            }
            else
            {
                return std::log(value);
            }
        }

        /// @brief Computes the base-2 logarithm.
        /// @param value Numeric input.
        /// @return Base-2 logarithm of `value`.
        template <NumericType T>
        static constexpr auto log2(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::log2(value);
            }
            else
            {
                return std::log2(value);
            } 
        }

        /// @brief Computes the base-10 logarithm.
        /// @param value Numeric input.
        /// @return Base-10 logarithm of `value`.
        template <NumericType T>
        static constexpr auto log10(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::log10(value);
            }
            else
            {
                return std::log10(value);
            }
        }

        template <NumericType T>
        static constexpr auto sqrt(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::sqrt(value);
            }
            else
            {
                return std::sqrt(value);
            }
        }

        /// @brief Computes the natural exponential.
        /// @param value Numeric exponent.
        /// @return $e^{value}$.
        template <NumericType T>
        static constexpr auto exp(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::exp(value);
            }
            else
            {
                return std::exp(value);
            }
        }
        
        /// @brief Raises `base` to `exp_term`.
        /// @param base Base value.
        /// @param exp_term Exponent value.
        /// @return `base` raised to `exp_term`.
        template <NumericType T1, NumericType T2>
        static constexpr auto pow(T1 base, T2 exp_term)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::pow(base, exp_term);
            }
            else
            {
                return std::pow(base, exp_term);
            }
        }

        /// @brief Computes hyperbolic sine.
        /// @param value Numeric input.
        /// @return Hyperbolic sine of `value`.
        template <NumericType T>
        static constexpr auto sinh(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::sinh(value);
            }
            else
            {
                return std::sinh(value);
            }
        }

        /// @brief Computes hyperbolic cosine.
        /// @param value Numeric input.
        /// @return Hyperbolic cosine of `value`.
        template <NumericType T>
        static constexpr auto cosh(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::cosh(value);
            }
            else
            {
                return std::cosh(value);
            }
        }

        /// @brief Computes hyperbolic tangent.
        /// @param value Numeric input.
        /// @return Hyperbolic tangent of `value`.
        template <NumericType T>
        static constexpr auto tanh(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::tanh(value);
            }
            else
            {
                return std::tanh(value);
            }
        }

        /// @brief Computes inverse hyperbolic sine.
        /// @param value Numeric input.
        /// @return Inverse hyperbolic sine of `value`.
        template <NumericType T>
        static constexpr auto asinh(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::asinh(value);
            }
            else
            {
                return std::asinh(value);
            }
        }

        /// @brief Computes inverse hyperbolic cosine.
        /// @param value Numeric input expected to be at least one.
        /// @return Inverse hyperbolic cosine of `value`.
        template <NumericType T>
        static constexpr auto acosh(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::acosh(value);
            }
            else
            {
                return std::acosh(value);
            }
        }

        /// @brief Computes inverse hyperbolic tangent.
        /// @param value Numeric input expected in `(-1, 1)`.
        /// @return Inverse hyperbolic tangent of `value`.
        template <NumericType T>
        static constexpr auto atanh(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::atanh(value);
            }
            else
            {
                return std::atanh(value);
            }
        }

        /// @brief Computes the logarithm of the gamma function.
        /// @param value Numeric input.
        /// @return `lgamma(value)`.
        template <NumericType T>
        static constexpr auto lgamma(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::lgamma(value);
            }
            else
            {
                return std::lgamma(value);
            }
        }

        /// @brief Computes the gamma function.
        /// @param value Numeric input.
        /// @return `tgamma(value)`.
        template <NumericType T>
        static constexpr auto tgamma(T value)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::tgamma(value);
            }
            else
            {
                return std::tgamma(value);
            }
        }

        /// @brief Computes the logarithm of the beta function.
        /// @param a First shape parameter.
        /// @param b Second shape parameter.
        /// @return `lbeta(a, b)`.
        template <NumericType T1, NumericType T2>
        static constexpr auto lbeta(const T1& a, const T2& b)
        {
            if (is_constant_evaluated())
            {
                return constexpr_math::lbeta(a, b);
            }
            else
            {
                return std::log(std::beta(a, b));
            }
        }

        /// @brief Computes the beta function.
        /// @param a First shape parameter.
        /// @param b Second shape parameter.
        /// @return `beta(a, b)`.
        template <NumericType T1, NumericType T2>
        static constexpr auto beta(const T1& a, const T2& b)
        {
            if (is_constant_evaluated())
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