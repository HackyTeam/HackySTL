#include <Random.hpp>
#include <assert.h>
#include <random>

int main()
{
    std::mt19937_64 rng;
    std::uniform_int_distribution d{0, 99};
    d(rng);

    /*hsd::mt19937_32 gen32{};
    hsd::mt19937_64 gen64;
 
    for (auto n{1}; n != 10'000; gen32.generate(), gen64.generate(), ++n);
 
    assert(gen32.generate() == 4'123'659'995 and
           gen64.generate() == 9'981'545'732'273'789'042ULL);*/
}