#include <Time.hpp>
#include <Thread.hpp>
#include <Io.hpp>

int main()
{
    hsd::time t1;
    hsd::io::print<"Today we are in {}">(t1.to_text());
    hsd::this_thread::sleep_for(60.f);    
    t1.update();
    hsd::io::print<"Today we are in {}">(t1.to_text());
}