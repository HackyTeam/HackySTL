#include <Extra/GPIO.hpp>

int main()
{
    auto sensor_pin = hsd::gpio::get_pin(hsd::gpio::pins::pin_4);
    sensor_pin.get().set_direction(hsd::gpio::direction::input);
    
    auto output_pin = hsd::gpio::get_pin(hsd::gpio::pins::pin_17);
    output_pin.get().set_direction(hsd::gpio::direction::output);
    output_pin.get().set_value(hsd::gpio::value::high).unwrap();

    while (true)
    {
        hsd_println("{}", sensor_pin.get().get_value().unwrap() == hsd::gpio::value::high);
    }

    return 0;
}