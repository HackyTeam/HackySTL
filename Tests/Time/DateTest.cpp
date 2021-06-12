#include <Time.hpp>
#include <Thread.hpp>
#include <Io.hpp>

int main()
{
    using namespace hsd::time_literals;
    hsd::date t1;
    hsd::io::print<"Today we are in {}\n">(t1.get_time().to_text());
    hsd::this_thread::sleep_for(10.f).unwrap();    
    t1.update();
    hsd::io::print<"Today we are in {}\n">(t1.get_time().to_text());

    auto epoch_year = 0_yr;
    hsd::io::print<"{}\n">(epoch_year.to_text());
}