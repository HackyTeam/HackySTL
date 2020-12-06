#include "../../cpp/Printf.hpp"

#include <random>

int main() {
    std::random_device drng;
    std::uniform_int_distribution dist(1, 999);
    int num = dist(drng);

    static char const* const days[] = {
        "Monday", "Tuesday", "Wednesday", "Thursday",
        "Friday", "Saturday", "Sunday", "Hiddenday", "Nullday"
    };

    int di = num % 9;
    char const* day = days[di];

    return xprintf("Hello, User%d!\nYour random day is %s\n", num, day);
}
