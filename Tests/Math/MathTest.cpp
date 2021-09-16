#include <Math.hpp>
#include <Io.hpp>

int main()
{
    {
        constexpr hsd::f64 val1 = hsd::math::sqrt(5.0);
        hsd::f64 val2 = std::sqrt(5.0);
        hsd_println("sqrt: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::abs(-5.0);
        hsd::f64 val2 = std::abs(-5.0);
        hsd_println("abs: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::ceil(5.22);
        hsd::f64 val2 = std::ceil(5.22);
        hsd_println("ceil: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::cos(5.0);
        hsd::f64 val2 = std::cos(5.0);
        hsd_println("cos: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::exp(5.0);
        hsd::f64 val2 = std::exp(5.0);
        hsd_println("exp: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::floor(-5.22);
        hsd::f64 val2 = std::floor(-5.22);
        hsd_println("floor: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::log10(5.0);
        hsd::f64 val2 = std::log10(5.0);
        hsd_println("log10: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::log2(5.0);
        hsd::f64 val2 = std::log2(5.0);
        hsd_println("log2: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::log(5.0);
        hsd::f64 val2 = std::log(5.0);
        hsd_println("log: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::max(5.0, 21.0);
        hsd::f64 val2 = std::max(5.0, 21.0);
        hsd_println("max: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::min(5.0, 21.0);
        hsd::f64 val2 = std::min(5.0, 21.0);
        hsd_println("min: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::mod(5.0, 1.3);
        hsd::f64 val2 = std::fmod(5.0, 1.3);
        hsd_println("mod: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::pow(5.0, 2.33);
        hsd::f64 val2 = std::pow(5.0, 2.33);
        hsd_println("pow: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::round(-5.232);
        hsd::f64 val2 = std::round(-5.232);
        hsd_println("round: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::sin(5.0);
        hsd::f64 val2 = std::sin(5.0);
        hsd_println("sin: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::cos(5.0);
        hsd::f64 val2 = std::cos(5.0);
        hsd_println("cos: {} {}", val1, val2);
    }
    
    {
        constexpr hsd::f64 val1 = hsd::math::tan(5.0);
        hsd::f64 val2 = std::tan(5.0);
        hsd_println("tan: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::asin(0.323);
        hsd::f64 val2 = std::asin(0.323);
        hsd_println("asin: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::acos(0.323);
        hsd::f64 val2 = std::acos(0.323);
        hsd_println("acos: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::atan(5.0);
        hsd::f64 val2 = std::atan(5.0);
        hsd_println("atan: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::atan2(0.0, -0.0);
        hsd::f64 val2 = std::atan2(0.0, -0.0);
        hsd_println("atan2: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::sinh(5.0);
        hsd::f64 val2 = std::sinh(5.0);
        hsd_println("sinh: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::cosh(5.0);
        hsd::f64 val2 = std::cosh(5.0);
        hsd_println("cosh: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::tanh(5.0);
        hsd::f64 val2 = std::tanh(5.0);
        hsd_println("tanh: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::asinh(5.0);
        hsd::f64 val2 = std::asinh(5.0);
        hsd_println("asinh: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::acosh(5.0);
        hsd::f64 val2 = std::acosh(5.0);
        hsd_println("acosh: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::atanh(0.22);
        hsd::f64 val2 = std::atanh(0.22);
        hsd_println("atanh: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::lgamma(5.0);
        hsd::f64 val2 = std::lgamma(5.0);
        hsd_println("lgamma: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::tgamma(5.33);
        hsd::f64 val2 = std::tgamma(5.33);
        hsd_println("tgamma: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::lbeta(5.0, 3.0);
        hsd::f64 val2 = std::log(std::beta(5.0, 3.0));
        hsd_println("lbeta: {} {}", val1, val2);
    }

    {
        constexpr hsd::f64 val1 = hsd::math::beta(5.0, 7.0);
        hsd::f64 val2 = std::beta(5.0, 7.0);
        hsd_println("beta: {} {}", val1, val2);
    }
}