#include <Extra/GPIO.hpp>

int main()
{
    auto& sensor_pin = hsd::gpio::pin::get(hsd::gpio::pins::pin_4);
    sensor_pin.set_direction(hsd::gpio::direction::input);
    
    auto& output_pin = hsd::gpio::pin::get(hsd::gpio::pins::pin_17);
    output_pin.set_direction(hsd::gpio::direction::output);
    output_pin.set_value(hsd::gpio::value::high).unwrap();

    while (true)
    {
        hsd_println("{}", sensor_pin.get_value().unwrap() == hsd::gpio::value::high);
    }

    return 0;
}