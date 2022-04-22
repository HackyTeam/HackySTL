#include <Math.hpp>
#include <Io.hpp>

int main()
{
    using namespace hsd::format_literals;

    {
        constexpr hsd::f64 val1 = hsd::math::sqrt(5.0);
        hsd::f64 val2 = std::sqrt(5.0);
        hsd::println("sqrt: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::abs(-5.0);
        hsd::f64 val2 = std::abs(-5.0);
        hsd::println("abs: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::ceil(5.22);
        hsd::f64 val2 = std::ceil(5.22);
        hsd::println("ceil: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::cos(5.0);
        hsd::f64 val2 = std::cos(5.0);
        hsd::println("cos: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::exp(5.0);
        hsd::f64 val2 = std::exp(5.0);
        hsd::println("exp: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::floor(-5.22);
        hsd::f64 val2 = std::floor(-5.22);
        hsd::println("floor: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::log10(5.0);
        hsd::f64 val2 = std::log10(5.0);
        hsd::println("log10: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::log2(5.0);
        hsd::f64 val2 = std::log2(5.0);
        hsd::println("log2: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::log(5.0);
        hsd::f64 val2 = std::log(5.0);
        hsd::println("log: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::max(5.0, 21.0);
        hsd::f64 val2 = std::max(5.0, 21.0);
        hsd::println("max: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::min(5.0, 21.0);
        hsd::f64 val2 = std::min(5.0, 21.0);
        hsd::println("min: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::mod(5.0, 1.3);
        hsd::f64 val2 = std::fmod(5.0, 1.3);
        hsd::println("mod: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::pow(5.0, 2.33);
        hsd::f64 val2 = std::pow(5.0, 2.33);
        hsd::println("pow: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::round(-5.232);
        hsd::f64 val2 = std::round(-5.232);
        hsd::println("round: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::sin(5.0);
        hsd::f64 val2 = std::sin(5.0);
        hsd::println("sin: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::cos(5.0);
        hsd::f64 val2 = std::cos(5.0);
        hsd::println("cos: {} {}"_fmt, val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::tan(5.0);
        hsd::f64 val2 = std::tan(5.0);
        hsd::println("tan: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::asin(0.323);
        hsd::f64 val2 = std::asin(0.323);
        hsd::println("asin: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::acos(0.323);
        hsd::f64 val2 = std::acos(0.323);
        hsd::println("acos: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::atan(5.0);
        hsd::f64 val2 = std::atan(5.0);
        hsd::println("atan: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::atan2(0.0, -0.0);
        hsd::f64 val2 = std::atan2(0.0, -0.0);
        hsd::println("atan2: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::sinh(5.0);
        hsd::f64 val2 = std::sinh(5.0);
        hsd::println("sinh: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::cosh(5.0);
        hsd::f64 val2 = std::cosh(5.0);
        hsd::println("cosh: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::tanh(5.0);
        hsd::f64 val2 = std::tanh(5.0);
        hsd::println("tanh: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::asinh(5.0);
        hsd::f64 val2 = std::asinh(5.0);
        hsd::println("asinh: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::acosh(5.0);
        hsd::f64 val2 = std::acosh(5.0);
        hsd::println("acosh: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::atanh(0.22);
        hsd::f64 val2 = std::atanh(0.22);
        hsd::println("atanh: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::lgamma(5.0);
        hsd::f64 val2 = std::lgamma(5.0);
        hsd::println("lgamma: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::tgamma(5.33);
        hsd::f64 val2 = std::tgamma(5.33);
        hsd::println("tgamma: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::lbeta(5.0, 3.0);
        hsd::f64 val2 = std::log(std::beta(5.0, 3.0));
        hsd::println("lbeta: {} {}"_fmt, val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::beta(5.0, 7.0);
        hsd::f64 val2 = std::beta(5.0, 7.0);
        hsd::println("beta: {} {}"_fmt, val1, val2);
    }
}