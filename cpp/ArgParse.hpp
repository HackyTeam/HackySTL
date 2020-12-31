#pragma once

#include "UnorderedMap.hpp"
#include "Functional.hpp"
#include "SStream.hpp"

namespace hsd
{
    class argument_parser
    {
    private:
        using function_type = function<void(hsd::sstream&)>;
        using action_type = pair<function_type, int>;
        unordered_map<const char*, action_type> _actions;
        vector<const char*> _informations;

    public:
        inline argument_parser(const char* info)
        {
            _informations.emplace_back(info);
        }
        
        argument_parser(const argument_parser&) = delete;
        argument_parser(argument_parser&&) = delete;
        argument_parser& operator=(const argument_parser&) = delete;
        argument_parser& operator=(argument_parser&&) = delete;
        inline ~argument_parser() = default;

        inline void add(const char* argument, int num_args,
            function_type&& func, const char* help)
        {
            _actions.emplace(argument, func, num_args);
            _informations.emplace_back(help);
        }

        inline void parse(int argc, const char** argv)
        {
            if(argc == 1)
            {
                return;
            }
            else if(cstring::compare(argv[1], "-h") == 0 || cstring::compare(argv[1], "--help") == 0)
            {
                for(auto& _tips : _informations)
                    puts(_tips);
            }
            else
            {
                //       0          1      2     3    4     5      6
                // launch_program clear command arg1 arg2 arg3 command2 
                // num_args = 3
                // index = 2

                int _index = 1, _incrementor;
                sstream _buf{512};

                while(_index < argc)
                {
                    auto& [_function, _num_args] = _actions.at(argv[_index]).unwrap(HSD_FUNCTION_NAME);
                    _incrementor = _num_args;
                    _index += 1;
                    
                    while(_num_args != 0)
                    {
                        _buf.add_raw_data(argv[_index + _incrementor - _num_args]);
                        _num_args--;
                    }

                    _buf.data()[_buf.size() - 1] = '\0';
                    _function(_buf).unwrap(HSD_FUNCTION_NAME);
                    _index += _incrementor;
                    _buf.reset_data();
                }
            }
        }
    };
} // namespace hsd
