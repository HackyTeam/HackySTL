#include "../../cpp/Time.hpp"
#include "../../cpp/Io.hpp"

int main()
{
    hsd::time t1;
    hsd::io::print(
        "Today we are in {}/{}/{} {}:{}", 
        t1.month_day(), t1.month(), 
        t1.year(), t1.hour(), t1.minute()
    );
}