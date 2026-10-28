// ryu_generic_hsd.hpp
//
// An hsd-flavored port of ryu_generic.hpp: a header-only implementation of
// the correctly-rounded, shortest round-trip float-to-decimal-string
// algorithm described in:
//
//   Ulf Adams, "Ryu: Fast Float-to-String Conversion", PLDI 2018.
//   https://doi.org/10.1145/3192366.3192369
//
// -----------------------------------------------------------------------
// What this is, and what it deliberately is not
// -----------------------------------------------------------------------
// This is Section 2's "basic" conversion routine (arbitrary/sufficiently-
// large-precision integer arithmetic, all three of Steele & White's
// correctness criteria), generalized with C++20 templates -- exactly like
// the original ryu_generic.hpp. What changed here is *only* the plumbing:
// every standard-library component has been swapped for its HackySTL
// (`hsd::`) counterpart, so this header now depends solely on the library
// found under lib/ (Base/, Containers/, Serialize/) and no longer includes
// anything from <string>, <limits>, <bit>, <cstdint>, or <type_traits>.
//
//   std::string                    -> hsd::string
//   std::numeric_limits<T>         -> hsd::limits<T>
//   std::bit_cast<To>(from)        -> hsd::bit_cast<To>(from)
//   std::conditional_t/is_void_v   -> hsd::conditional_t / hsd::is_void
//   std::is_floating_point_v<T>    -> hsd::FloatingPointType<T> (concept)
//   std::to_string(small integer)  -> hsd::to_pseudo_string(...)
//   std::uint*_t / std::int64_t    -> hsd::u* / hsd::i64
//
// The one substantial change is BigUInt itself. The original file paired
// with a *dynamically growing* arbitrary-precision `big_uint.hpp`. HackySTL
// has no such type -- `hsd::integer<N>` (Containers/Integer.hpp) is a
// fixed-width integer whose bit width `N` is a *compile-time* template
// parameter, backed by a `hsd::bitset<N>` (i.e. a plain stack array, no
// heap allocation). To stand in for BigUInt, this file computes, per
// floating-point type T, a conservative worst-case bit width (see
// `detail::big_uint_bits<T>` below) and instantiates `hsd::integer<N>` at
// exactly that width -- so genericity across float/double/binary128 is now
// expressed as "the right N per T" rather than "a value that grows as
// needed". Three practical consequences worth knowing:
//
//   * The required width is dominated by the smallest-subnormal case,
//     which needs pow5(bias + mantissa_bits + 1) worth of bits -- roughly
//     2.32 bits per unit of exponent range. That's ~2.6 Kbit for a 64-bit
//     double, but tens of kilobits for a true 128-bit float, since a
//     128-bit format's exponent range is enormously larger.
//   * Because `hsd::integer<N>` lives entirely on the stack (no heap),
//     `compute_shortest` keeping several live N-bit values at once means
//     stack usage scales the same way. Fine for float/double; worth
//     checking your thread's stack size before reaching for a 128-bit type.
//   * `hsd::integer<N>`'s only built-in division, `divide_with_reminder`,
//     is bit-serial: exactly N iterations *every* call, regardless of the
//     divisor. compute_shortest calls a small-divisor division a few times
//     per decimal digit it removes, so using that directly is O(N) per
//     digit, O(N^2) overall -- measured at ~1.5 *seconds* for a single
//     double denorm_min before this was found. `detail::divmod_small`/
//     `mod_small` below instead do a manual base-2^32 division directly
//     over `get_underlying_array()`, which is what gets this back down to
//     something reasonable (see the comment on those two for specifics).
//     Measured on this machine: ~150-250us for everyday values, ~2ms for
//     values near 1e300, and up to ~15-20ms for the most extreme
//     subnormals (double's denorm_min/min). That's still well short of the
//     original's microseconds across the board, since a real
//     arbitrary-precision integer shrinks as digits are removed while
//     `hsd::integer<N>` never does -- if you need double conversion to be
//     fast across its *entire* range rather than merely correct, this
//     header (like the file it's ported from) is optimizing for the
//     latter.
//
// Also note hsd's own float type universe is narrower than the original
// file's: `hsd::limits`/`hsd::is_floating_point` only recognize f32, f64,
// and f128 (== float, double, long double). On the common x86/x86-64
// toolchains `f128` *is* the 80-bit extended format padded into 16 bytes
// with an explicit (not implicit) leading significand bit -- exactly the
// layout the padding static_assert below is designed to reject, so this
// header only actually accepts f32/f64 there, same as upstream Ryu's own
// public implementations. It will accept `f128` wherever the platform's
// `long double` genuinely is IEEE binary128 (e.g. many non-x86 ABIs).
//
// -----------------------------------------------------------------------

#include "Base/Types.hpp"
#include "Base/Limits.hpp"
#include "Base/Concepts.hpp"
#include "Base/TypeTraits.hpp"
#include "Base/Utility.hpp"
#include "Base/Math.hpp"
#include "Containers/String.hpp"
#include "Containers/Integer.hpp"
#include "Serialize/ToString.hpp"

namespace hsd::ryu_generic {

// =========================================================================
// Section 2.1: bit-layout traits for an IEEE floating point type.
// =========================================================================
template <class T>
struct fp_traits {
    // hsd::limits doesn't expose is_iec559/radix fields (its float support
    // is limited to f32/f64/f128, all of which are binary formats), so the
    // real defense against a non-conforming layout -- including x86's
    // 80-bit extended "long double" -- is the padding check below, exactly
    // as in the upstream implementation this is ported from.
    static_assert(is_floating_point<T>::value,
                  "ryu_generic (hsd) requires a type hsd::limits recognizes as "
                  "floating point (currently f32/float, f64/double, or "
                  "f128/long double)");

    static constexpr int total_bits = sizeof(T) * 8;

    // hsd::limits<T>::mantissa is already "digits - 1", i.e. the stored
    // mantissa width with the implicit leading bit excluded.
    static constexpr int mantissa_bits = static_cast<int>(limits<T>::mantissa);

    // hsd::limits<T>::exponent is precomputed by hsd as the number of bits
    // needed so that 2^exponent == (max_exponent + 1) * 2 -- i.e. the width
    // of the exponent field. hsd::limits<T>::max_exponent itself is offset
    // by one from the C++ standard's numeric_limits convention (it mirrors
    // <float.h>'s FLT_MAX_EXP - 1 instead), but that offset cancels out
    // inside `exponent`'s own derivation, so this still lines up with
    // std::bit_width(numeric_limits<T>::max_exponent) from the original.
    static constexpr int exponent_bits = static_cast<int>(limits<T>::exponent);

    static_assert(1 + exponent_bits + mantissa_bits == total_bits,
                  "T has a padding/explicit-integer-bit layout incompatible with a "
                  "packed IEEE-754 sign+exponent+mantissa bit pattern "
                  "(e.g. x86 80-bit extended 'long double', which hsd maps to "
                  "f128, stored in 16 bytes with an explicit rather than "
                  "implicit leading significand bit). ryu_generic only supports "
                  "types whose bits exactly fill their storage as "
                  "sign(1) + exponent(e) + mantissa(m) with an implicit "
                  "leading 1, as f32/f64 do.");

    static constexpr i64 bias = (i64(1) << (exponent_bits - 1)) - 1;

    // Smallest unsigned hsd integer type that can hold a full bit-pattern of T.
    using storage_type =
        conditional_t<total_bits <= 8, u8,
        conditional_t<total_bits <= 16, u16,
        conditional_t<total_bits <= 32, u32,
        conditional_t<total_bits <= 64, u64,
#if defined(HSD_COMPILER_GCC) || defined(HSD_COMPILER_MSVC)
        conditional_t<total_bits <= 128, u128,
        void>
#else
        void
#endif
        >>>>;

    static_assert(!is_void<storage_type>::value,
                  "No suitable unsigned integer large enough to bit_cast this "
                  "floating point type (types above 128 bits are not supported).");

    static_assert(sizeof(storage_type) == sizeof(T),
                  "storage_type must be exactly as wide as T for bit_cast to apply");
};

// =========================================================================
// BigUInt replacement: a fixed-width `hsd::integer<N>` sized per T, plus
// the handful of free-function helpers that stand in for the arbitrary-
// precision BigUInt's member functions from the original implementation.
// =========================================================================
namespace detail {

    // Per-T bit width for the `hsd::integer<N>` standing in for BigUInt.
    // See the file header comment for what drives this number.
    template <class T>
    struct big_uint_bits {
        using Traits = fp_traits<T>;

        // basic_routine (below) scales the interval endpoints by pow5(-e2)
        // whenever e2 = e_f - 2 is negative; the most negative e2 occurs at
        // the smallest subnormal, where e_f = 1 - bias - mantissa_bits.
        static constexpr i64 max_pow5_exponent = Traits::bias + Traits::mantissa_bits + 1;

        // Bits for pow5(max_pow5_exponent) (~log2(5) bits per unit of
        // exponent), plus the mantissa itself (scaled by up to 4, so +2
        // bits), plus a fixed safety margin covering the various small
        // +-1/+-2 adjustments elsewhere in the routine and any rounding in
        // the log2(5) estimate itself.
        static constexpr usize value = static_cast<usize>(
            static_cast<f128>(Traits::mantissa_bits + 4) +
            static_cast<f128>(max_pow5_exponent) * math::log2(static_cast<f128>(5))
        ) + 64;
    };

    template <class T>
    using big_uint_for = integer<big_uint_bits<T>::value>;

    // pow2/pow5: the two scale factors basic_routine needs (Section 2.3).
    template <usize N>
    constexpr integer<N> big_pow2(unsigned exponent) {
        integer<N> result{u64(1)};
        result <<= exponent;
        return result;
    }

    template <usize N>
    constexpr integer<N> big_pow5(unsigned exponent) {
        integer<N> result{u64(1)};
        integer<N> base{u64(5)};

        for (; exponent != 0; exponent >>= 1) {
            if (exponent & 1u) result *= base;
            if (exponent > 1u) base *= base;
        }

        return result;
    }

    // Widens a (possibly >64-bit) unsigned mantissa field into a big_uint.
    template <usize N, typename U>
    constexpr integer<N> big_from_uint(U value) {
        if constexpr (sizeof(U) <= sizeof(u64)) {
            return integer<N>{static_cast<u64>(value)};
        } else {
            integer<N> hi{static_cast<u64>(value >> 64)};
            integer<N> lo{static_cast<u64>(value)};
            hi <<= 64;
            hi |= lo;
            return hi;
        }
    }

    // BigUInt::divmod_small(10) / BigUInt::mod_small(10) equivalents.
    //
    // hsd::integer<N> has no small-divisor fast path of its own: the general
    // `divide_with_reminder` is a bit-serial long division that always does
    // exactly N iterations regardless of the divisor, i.e. O(N) work *per
    // bit*, O(N^2) per call -- and `compute_shortest` below calls this up to
    // a few times per decimal digit removed. For a double, N is a few
    // thousand bits (see `big_uint_bits`), and the smallest subnormal needs
    // on the order of 700 digit removals: that combination is catastrophic
    // in practice (on the order of a second for a single value, rather than
    // the microseconds the original arbitrary-precision BigUInt needed).
    // `divide_by_constant<10>()` doesn't help either -- its reciprocal setup
    // is itself an O(N)-iteration division evaluated once at compile time,
    // which simply moves the same blow-up to (extremely long) build times.
    //
    // Both helpers below instead do a manual base-2^32 schoolbook division
    // by the (always tiny) divisor directly over `get_underlying_array()`,
    // which is O(N) *total* per call: mod_small never touches more than the
    // word array (no reconstruction needed since a remainder < divisor
    // always fits in a u32), and divmod_small reconstructs the N-bit
    // quotient with one O(1) bit assignment per bit via `operator[]`
    // instead of hsd::integer's whole-array `<<=`/`|=` (which are O(N)
    // *each*, and would make a naive word-by-word reconstruction O(N^2)).
    template <usize N>
    constexpr u32 mod_small(const integer<N>& value, u32 divisor) {
        const auto& words = value.get_underlying_array();
        const usize word_count = words.size();

        // Skip the leading run of all-zero words: while `rem == 0` and the
        // next word is `0`, that word contributes nothing. Since a,b,c only
        // ever shrink across the digit-removal loop in compute_shortest,
        // later calls skip progressively more of the (now-unused) leading
        // words, so the *total* cost across the whole loop tracks the
        // value's shrinking size rather than staying pinned at N forever.
        usize i = 0;
        while (i < word_count && words[i] == 0) ++i;

        u64 rem = 0;
        for (; i < word_count; ++i) {
            rem = ((rem << 32) | words[i]) % divisor;
        }

        return static_cast<u32>(rem);
    }

    template <usize N>
    constexpr u32 divmod_small(integer<N>& value, u32 divisor) {
        const auto& words = value.get_underlying_array();
        const usize word_count = words.size();

        integer<N> quotient{};
        u64 rem = 0;

        usize word_idx = 0;
        while (word_idx < word_count && words[word_idx] == 0) ++word_idx;

        for (; word_idx < word_count; ++word_idx) {
            u64 cur = (rem << 32) | words[word_idx];
            u32 qword = static_cast<u32>(cur / divisor);
            rem = cur % divisor;

            const usize base_bit = (word_count - 1 - word_idx) * 32;

            for (unsigned b = 0; b < 32; ++b) {
                const usize bit_idx = base_bit + b;
                if (bit_idx < N && ((qword >> b) & 1u) != 0) {
                    quotient[bit_idx] = true;
                }
            }
        }

        value = move(quotient);
        return static_cast<u32>(rem);
    }

    // BigUInt::to_decimal_string() equivalent. hsd does have its own
    // integer<N> -> string printer (Containers/Integer.hpp's `print_to`),
    // but it's wired into hsd's `format_output`/FormatGenerator machinery
    // (ANSI styling tags and all), which is more coupling than a plain
    // "give me the digits" conversion needs; this stays self-contained
    // using the same divide-and-peel approach as the rest of this file.
    template <usize N>
    string to_decimal_string(integer<N> value) {
        string result;

        if (value.none()) {
            result.push_back('0');
            return result;
        }

        while (!value.none()) {
            result.push_back(static_cast<char>('0' + divmod_small(value, 10)));
        }

        for (usize i = 0, j = result.length() - 1; i < j; ++i, --j) {
            char tmp = result[i];
            result[i] = result[j];
            result[j] = tmp;
        }

        return result;
    }

} // namespace detail

// =========================================================================
// Section 2.2, step 1: decode a value into sign / special-case flags /
// (m_f, e_f), and record the parity of the stored mantissa (used later
// for the "accept_smaller"/"accept_larger" round-to-even decision).
// =========================================================================
template <class T>
struct DecodedFloat {
    bool sign = false;
    bool is_zero = false;
    bool is_inf = false;
    bool is_nan = false;
    bool mantissa_field_even = true; // parity of the *stored* mantissa bits
    // True iff the stored mantissa field is all-zero AND the stored
    // exponent field is > 1 -- i.e. d sits exactly on a power-of-two
    // boundary that is *not* the smallest normal number. This is exactly
    // the condition the paper uses (step 2) to pick the asymmetric
    // smaller-halfway-point formula.
    bool at_pow2_boundary_above_smallest_normal = false;
    detail::big_uint_for<T> mantissa;  // m_f
    i64 exponent = 0;                  // e_f, s.t. value == (-1)^sign * m_f * 2^e_f
};

template <class T>
DecodedFloat<T> decode(T value) {
    using Traits = fp_traits<T>;
    using U = typename Traits::storage_type;
    constexpr usize Bits = detail::big_uint_bits<T>::value;

    U bits = bit_cast<U>(value);

    U mantissa_mask = (U(1) << Traits::mantissa_bits) - U(1);
    U exponent_mask = (U(1) << Traits::exponent_bits) - U(1);

    U mantissa_field = bits & mantissa_mask;
    U exponent_field = static_cast<U>(bits >> Traits::mantissa_bits) & exponent_mask;

    DecodedFloat<T> d;
    d.sign = ((bits >> (Traits::mantissa_bits + Traits::exponent_bits)) & U(1)) != 0;

    if (exponent_field == exponent_mask) {
        if (mantissa_field == 0) d.is_inf = true;
        else d.is_nan = true;
        return d;
    }
    if (exponent_field == 0 && mantissa_field == 0) {
        d.is_zero = true;
        return d;
    }

    d.mantissa_field_even = (mantissa_field & U(1)) == 0;
    d.at_pow2_boundary_above_smallest_normal = (mantissa_field == 0) && (exponent_field > U(1));

    if (exponent_field == 0) {
        // subnormal: m_f = m, e_f = 1 - bias - mantissa_bits
        d.mantissa = detail::big_from_uint<Bits>(mantissa_field);
        d.exponent = 1 - Traits::bias - Traits::mantissa_bits;
    } else {
        // normal: m_f = 2^mantissa_bits + m, e_f = e - bias - mantissa_bits
        d.mantissa = detail::big_pow2<Bits>(static_cast<unsigned>(Traits::mantissa_bits))
                   + detail::big_from_uint<Bits>(mantissa_field);
        d.exponent = static_cast<i64>(exponent_field) - Traits::bias - Traits::mantissa_bits;
    }
    return d;
}

// =========================================================================
// Section 2.3 / 2.4: compute_shortest, modified for correct rounding.
//
// Given the interval boundaries (a, b, c) -- all sharing an implicit
// scale of 10^e10 -- and whether the boundaries themselves may be
// returned exactly, find the shortest decimal d_o * 10^(e_o + e10) that:
//   (1) preserves information (round-trips back to the original value),
//   (2) has minimal length among all such values, and
//   (3) among values of that minimal length, is the one closest to the
//       true value b, breaking exact ties to even.
//
// Templated on the big-integer type `BigT` (an `hsd::integer<N>` sized by
// `detail::big_uint_bits`) rather than a single fixed BigUInt type.
// =========================================================================
template <class BigT>
struct ShortestDecimal {
    BigT digits; // d_o
    i64 e;       // e_o, such that this piece represents d_o * 10^e
};

template <class BigT>
ShortestDecimal<BigT> compute_shortest(BigT a, BigT b, BigT c,
                                        bool accept_smaller, bool accept_larger) {
    BigT c0 = c;
    if (!accept_larger) --c0;

    bool all_a_zero = true;
    bool all_b_zero = true;
    u32 last_digit = 0; // digit_i, the most recently removed digit of b
    i64 i = 0;

    BigT a_i = move(a);
    BigT b_i = move(b);
    BigT c_i = move(c0);

    // Primary loop: remove digits while a and c still have room, i.e.
    // while floor(a_i/10) < floor(c_i/10).
    while (true) {
        BigT qa = a_i; u32 ra = detail::divmod_small(qa, 10);
        BigT qc = c_i; detail::divmod_small(qc, 10);
        if (!(qa < qc)) break;

        all_a_zero = all_a_zero && (ra == 0);
        u32 rb = detail::mod_small(b_i, 10);
        all_b_zero = all_b_zero && (rb == 0);
        last_digit = rb;

        detail::divmod_small(b_i, 10);
        a_i = move(qa);
        c_i = move(qc);
        ++i;
    }

    // Secondary loop: if the smaller boundary may be returned exactly and
    // every digit removed from `a` so far was zero, `a` may still have
    // more trailing zeros of its own that can be stripped (independent of
    // c) without losing information-preservation on the low side.
    if (accept_smaller && all_a_zero) {
        while (detail::mod_small(a_i, 10) == 0 && !a_i.none()) {
            u32 rb = detail::mod_small(b_i, 10);
            all_b_zero = all_b_zero && (rb == 0);
            last_digit = rb;

            detail::divmod_small(a_i, 10);
            detail::divmod_small(c_i, 10);
            detail::divmod_small(b_i, 10);
            ++i;
        }
    }

    // Correct rounding: decide between b_i and b_i + 1.
    bool is_tie = (last_digit == 5) && all_b_zero;
    // Break ties to even (the same convention IEEE-754 uses elsewhere):
    // exactly one of b_i, b_i+1 is even, and we prefer it.
    bool break_tie_down = (detail::mod_small(b_i, 2) == 0);
    bool want_round_down = (last_digit < 5) || (is_tie && break_tie_down);

    bool a_ne_b = !(a_i == b_i);
    BigT b_plus_1 = b_i; ++b_plus_1;
    bool b1_illegal = (b_plus_1 <=> c_i) > 0;

    bool round_down = (want_round_down && (a_ne_b || all_a_zero)) || b1_illegal;

    ShortestDecimal<BigT> result;
    result.digits = round_down ? move(b_i) : move(b_plus_1);
    result.e = i;
    return result;
}

// =========================================================================
// Sections 2.2-2.4 combined: full "basic routine" for one finite, nonzero
// value. Returns the shortest, correctly-rounded decimal digit string and
// the decimal exponent of its most significant digit (i.e. the value
// equals 0.d1 d2 d3 ... * 10^(msd_exponent+1), or equivalently
// d1.d2 d3 ... * 10^msd_exponent).
// =========================================================================
struct DecimalResult {
    bool negative = false;
    string digits;         // no leading/trailing zeros, at least one digit
    i64 msd_exponent = 0;  // power of ten of the *first* digit
};

template <class T>
DecimalResult basic_routine(const DecodedFloat<T>& d) {
    using BigT = detail::big_uint_for<T>;
    constexpr usize Bits = detail::big_uint_bits<T>::value;

    // Step 2: interval of information-preserving outputs, (u, v, w) * 2^e2.
    const i64 e2 = d.exponent - 2;
    BigT vm = d.mantissa; vm *= u32(4);       // v = 4*m_f
    BigT wm = vm;         wm += 2;             // w = 4*m_f + 2
    BigT um = vm;
    // u = 4*m_f - (1 if stored mantissa == 0 and stored exponent > 1 else 2)
    um -= (d.at_pow2_boundary_above_smallest_normal ? 1 : 2);

    // Step 3: convert (u, v, w) * 2^e2 to (a, b, c) * 10^e10.
    BigT a, b, c;
    i64 e10;
    if (e2 >= 0) {
        BigT scale = detail::big_pow2<Bits>(static_cast<unsigned>(e2));
        a = um * scale; b = vm * scale; c = wm * scale;
        e10 = 0;
    } else {
        BigT scale = detail::big_pow5<Bits>(static_cast<unsigned>(-e2));
        a = um * scale; b = vm * scale; c = wm * scale;
        e10 = e2;
    }

    // Round-to-nearest-even parser: the smaller/larger boundary may be
    // output exactly iff the stored mantissa is even.
    bool accept = d.mantissa_field_even;

    ShortestDecimal<BigT> shortest = compute_shortest<BigT>(
        move(a), move(b), move(c), accept, accept);

    DecimalResult out;
    out.negative = d.sign;
    out.digits = detail::to_decimal_string(shortest.digits);
    // strip any trailing zeros compute_shortest could still leave in, and
    // fold them into the exponent instead, so `digits` never ends in '0'
    // (keeps the "minimum length" property visible in the string itself).
    i64 extra_trailing_zeros = 0;
    while (out.digits.length() > 1 && out.digits.back() == '0') {
        out.digits.pop_back();
        ++extra_trailing_zeros;
    }
    i64 combined_exp = shortest.e + e10 + extra_trailing_zeros;
    out.msd_exponent = combined_exp + static_cast<i64>(out.digits.length()) - 1;
    return out;
}

// =========================================================================
// Formatting: turn (sign, digits, msd_exponent) into a human-readable
// string, either always in scientific notation, or "auto" (fixed notation
// for everyday magnitudes, scientific for very large/small ones -- the
// same convention most languages' default float formatting uses).
// =========================================================================
enum class Format { Auto, Fixed, Scientific };

inline string format_scientific(const DecimalResult& r) {
    string s;
    if (r.negative) s.push_back('-');
    s.push_back(r.digits[0]);
    if (r.digits.length() > 1) {
        s.push_back('.');
        for (usize i = 1; i < r.digits.length(); ++i) s.push_back(r.digits[i]);
    }
    s.push_back('e');
    s.push_back(r.msd_exponent < 0 ? '-' : '+');
    i64 mag = r.msd_exponent < 0 ? -r.msd_exponent : r.msd_exponent;
    auto mag_digits = to_pseudo_string(mag);
    s += string{mag_digits.data(), mag_digits.size() + 1};
    return s;
}

inline string format_fixed(const DecimalResult& r) {
    string s;
    if (r.negative) s.push_back('-');
    const string& d = r.digits;
    const i64 n = static_cast<i64>(d.length());
    const i64 exp = r.msd_exponent; // power of ten of d[0]

    if (exp >= n - 1) {
        // integer, possibly with trailing zeros: d, then (exp-(n-1)) zeros
        for (usize i = 0; i < d.length(); ++i) s.push_back(d[i]);
        for (i64 i = 0; i < exp - (n - 1); ++i) s.push_back('0');
    } else if (exp >= 0) {
        // decimal point lands inside the digit string
        for (i64 i = 0; i <= exp; ++i) s.push_back(d[static_cast<usize>(i)]);
        s.push_back('.');
        for (usize i = static_cast<usize>(exp + 1); i < d.length(); ++i) s.push_back(d[i]);
    } else {
        // 0.000d...
        // (built with push_back, not operator+=(const char*): that overload
        // requires non-empty existing storage and panics otherwise, and `s`
        // is still empty here whenever the value is non-negative.)
        s.push_back('0');
        s.push_back('.');
        for (i64 i = 0; i < -exp - 1; ++i) s.push_back('0');
        for (usize i = 0; i < d.length(); ++i) s.push_back(d[i]);
    }
    return s;
}

inline string format(const DecimalResult& r, Format fmt) {
    if (fmt == Format::Scientific) return format_scientific(r);
    // Auto: mirrors common "shortest repr" conventions (similar to
    // Python/JS): fixed notation for everyday magnitudes, scientific
    // outside that range.
    if (fmt == Format::Auto) {
        if (r.msd_exponent < -4 || r.msd_exponent >= 17) 
            return format_scientific(r);

        return format_fixed(r);
    }

    return format_fixed(r);
}

// =========================================================================
// Public API.
// =========================================================================
template <FloatingPointType T>
string to_string(T value, Format fmt = Format::Auto) {
    DecodedFloat<T> d = decode<T>(value);
    if (d.is_nan) return "nan";
    if (d.is_inf) return d.sign ? "-inf" : "inf";
    if (d.is_zero) return d.sign ? "-0" : "0";

    DecimalResult r = basic_routine<T>(d);
    return format(r, fmt);
}

} // namespace hsd::ryu_generic