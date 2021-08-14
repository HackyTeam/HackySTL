#include <Extra/GPIO.hpp>

int main()
{
    auto& sensor_pin = hsd::gpio::get_pin(hsd::gpio::pins::pin_4);
    sensor_pin.set_direction(hsd::gpio::direction::input);

    while (sensor_pin.get_value().unwrap() == hsd::gpio::value::low)
        ;

    return 0;
}