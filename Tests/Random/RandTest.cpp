#include <Random.hpp>
#include <Io.hpp>

int main()
{
    hsd::mt19937_32 engine;

    for (hsd::u16 i = 0; i < 65000; i++)
    {
        hsd_println(
            L"{}, {}",
            engine.generate(1, 4).unwrap(),
            engine.generate(1., 4.).unwrap()
        );

        engine.discard(5);
    }
}