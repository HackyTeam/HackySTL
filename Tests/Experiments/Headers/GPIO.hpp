#pragma once

#include "../Path.hpp"
#include "../Io.hpp"

namespace hsd
{
    namespace gpio
    {
        enum class direction
        {
            input,
            output
        };

        enum class value
        {
            high,
            low
        };

        enum class pins
        {
            pin_0 , pin_1 , pin_2 , pin_3 , 
            pin_4 , pin_5 , pin_6 , pin_7 , 
            pin_8 , pin_9 , pin_10, pin_11, 
            pin_12, pin_13, pin_14, pin_15, 
            pin_16, pin_17, pin_18, pin_19, 
            pin_20, pin_21, pin_22, pin_23, 
            pin_24, pin_25, pin_26, pin_27
        };

        class pin
        {
        private:
            i32 _pin = -1;
            direction _direction = direction::input;
            bool _is_initialized = false;

        public:
            inline pin() = default;
            inline pin(const pin&) = delete;
            inline pin& operator=(const pin&) = delete;

            inline pin(pins pin)
                : _pin{static_cast<i32>(pin)}
            {                
                auto _init_file = io::load_file(
                    "/sys/class/gpio/export", io::options::text::write
                ).unwrap();

                _init_file.print<"{}">(_pin).unwrap();
                _init_file.close();
                _is_initialized = true;
            }

            inline pin(pin&& other)
                : _pin{other._pin}, _direction{other._direction}, 
                _is_initialized{other._is_initialized}
            {
                other._pin = -1;
                other._is_initialized = false;
            }

            inline pin& operator=(pin&& rhs)
            {
                swap(_pin, rhs._pin);
                swap(_direction, rhs._direction);
                swap(_is_initialized, rhs._is_initialized);

                return *this;
            }

            inline ~pin()
            {                
                if (_is_initialized == true)
                {
                    auto _clear_file = io::load_file(
                        "/sys/class/gpio/unexport", io::options::text::write
                    ).unwrap();

                    _clear_file.print<"{}">(_pin).unwrap();
                }
            }

            inline bool is_initialized() const
            {
                return _is_initialized;
            }

            static inline pin& get(pins pin_value)
            {
                static stack_array<pin, 28> _pins;
                u32 _index = static_cast<u32>(pin_value);

                if (_pins[_index].is_initialized() == false)
                {
                    _pins[_index] = pin{pin_value};
                }

                return _pins[_index];
            }

            inline void set_direction(direction dir)
            {
                sstream<255> _stream;
                _stream.write_data<"/sys/class/gpio/gpio{}/direction">(_pin);
                auto _direction_file = io::load_file(
                    _stream.c_str(), io::options::text::write
                ).unwrap();
                
                _direction_file.print<"{}">(
                    dir == direction::input ? "in" : "out"
                ).unwrap();
                
                _direction = dir;
            }

            inline option_err<runtime_error> set_value(value val)
            {
                if (_direction == direction::output)
                {
                    sstream<255> _stream;
                    _stream.write_data<"/sys/class/gpio/gpio{}/value">(_pin);
                    
                    auto _value_file = io::load_file(
                        _stream.c_str(), io::options::text::write
                    ).unwrap();

                    _value_file.print<"{}">(
                        val == value::high ? "1" : "0"
                    ).unwrap();
                    
                    return {};
                }
                else
                {
                    return runtime_error{"Error: GPIO pin is not an output."};
                }
            }

            inline result<value, runtime_error> get_value()
            {
                if (_direction == direction::input)
                {
                    sstream<255> _stream;
                    _stream.write_data<"/sys/class/gpio/gpio{}/value">(_pin);
                    
                    auto _value_file = io::load_file(
                        _stream.c_str(), io::options::text::read
                    ).unwrap();
                    
                    char _res;
                    _value_file.read().unwrap().set_data(_res).unwrap();
                    
                    _value_file.close();
                    return _res == '1' ? value::high : value::low;
                }
                else
                {
                    return runtime_error{"Error: GPIO pin is not an input."};
                }
            }
        };
    } // namespace gpio
} // namespace hsd
