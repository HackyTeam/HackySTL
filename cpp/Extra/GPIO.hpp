#pragma once

#include "../Path.hpp"
#include "../Io.hpp"
#include "../UnorderedMap.hpp"

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

        class pin
        {
        private:
            i32 _pin = -1;
            direction _direction = direction::input;
            io _direction_file;
            io _value_file;

        public:
            inline pin(const pin&) = delete;
            inline pin& operator=(const pin&) = delete;

            inline pin(i32 pin)
                : _pin{pin}
            {
                namespace fs = filesystem;
                static static_sstream<255> _stream;
                static auto _init_file = io::load_file(
                    "/sys/class/gpio/export", io::options::text::write
                ).unwrap();

                _init_file.print<"{}\n">(pin);

                _stream.write_data<"/sys/class/gpio{}/direction">(pin);

                if (!fs::path{_stream.c_str()}.status().exists())
                {
                    hsd_fprint_check(
                        stderr, "Error: GPIO pin %d does not exist.\n", pin
                    );

                    abort();
                }
                
                _direction_file = io::load_file(
                    _stream.c_str(), io::options::text::write
                ).unwrap();
                
                _stream.write_data<"/sys/class/gpio{}/value">(pin);
                
                _value_file = io::load_file(
                    _stream.c_str(), io::options::text::read_write
                ).unwrap();
            }

            inline pin(pin&& other)
                : _pin{other._pin}, _direction{other._direction}, 
                _direction_file{move(other._direction_file)}, 
                _value_file{move(other._value_file)}
            {}

            inline ~pin()
            {
                _direction_file.close();
                _value_file.close();
                
                static auto _clear_file = io::load_file(
                    "/sys/class/gpio/unexport", io::options::text::write
                ).unwrap();

                _clear_file.print<"{}\n">(_pin);
            }

            inline pin& operator=(pin&& rhs)
            {
                swap(_pin, rhs._pin);
                swap(_direction, rhs._direction);
                swap(_direction_file, rhs._direction_file);
                swap(_value_file, rhs._value_file);

                return *this;
            }

            inline void set_direction(direction dir)
            {
                _direction_file.print<"{}\n">(
                    dir == direction::input ? "in" : "out"
                ).unwrap();
                
                _direction = dir;
            }

            inline Result<void, runtime_error> set_value(value val)
            {
                if (_direction == direction::output)
                {
                    _value_file.print<"{}\n">(
                        val == value::high ? "1" : "0"
                    ).unwrap();
                    
                    return {};
                }
                else
                {
                    return runtime_error{"Error: GPIO pin is not an output."};
                }
            }

            inline Result<i32, runtime_error> get_value()
            {
                if (_direction == direction::input)
                {
                    return _value_file.read().unwrap().parse<i32>();
                }
                else
                {
                    return runtime_error{"Error: GPIO pin is not an input."};
                }
            }
        };

        static inline pin& get_pin(i32 pin_value)
        {
            static auto _pin_map = unordered_map<i32, pin>{};
            return _pin_map.emplace(pin_value, pin_value).first->second;
        }
    } // namespace gpio
} // namespace hsd
