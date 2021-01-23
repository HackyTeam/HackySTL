#pragma once

#include "Vector.hpp"
#include "FStatus.hpp"
#include "_FSDetail.hpp"

#if defined(HSD_PLATFORM_POSIX)
#include <dirent.h>
#include <utime.h>
#include <fcntl.h>
#include <errno.h>
#endif

namespace hsd
{
    #if defined(HSD_PLATFORM_POSIX)
    namespace filesystem
    {
        class path
        {
        private:
            DIR* _directory = nullptr;
            vector<string> _parents;
            fs_detail::Location _location;

            void _destroy()
            {
                if (_directory != nullptr)
                    closedir(_directory);
            }

        public:
            inline path()
                : path{string(".", 1)}
            {
                // Opens the directory at the
                // location of the executable
            }

            inline path(const char* raw_path)
                : _location{raw_path}
            {
                _directory = opendir(raw_path);
                const string& _absolute_path = absolute_name();
                _parents.emplace_back("/", 1);
                usize _index, _pos;

                for (_index = _absolute_path.find('/', 1), _pos = 1; 
                    _index != string::npos; _index = _absolute_path.find('/', _pos))
                {
                    _parents.emplace_back(_absolute_path.c_str() + _pos, _index - _pos);
                    _pos = _index + 1;
                }
            }

            inline path(const string& raw_path)
                : _location{raw_path}
            {
                const string& _absolute_path = absolute_name();
                _directory = opendir(_absolute_path.c_str());
                _parents.emplace_back("/", 1);
                usize _index, _pos;

                for (_index = _absolute_path.find('/', 1), _pos = 1; 
                    _index != string::npos; _index = _absolute_path.find('/', _pos))
                {
                    _parents.emplace_back(_absolute_path.c_str() + _pos, _index - _pos);
                    _pos = _index + 1;
                }
            }

            inline path(string&& raw_path)
                : _location{move(raw_path)}
            {
                const string& _absolute_path = absolute_name();
                _directory = opendir(_absolute_path.c_str());
                _parents.emplace_back("/", 1);
                usize _index, _pos;

                for (_index = _absolute_path.find('/', 1), _pos = 1; 
                    _index != string::npos; _index = _absolute_path.find('/', _pos))
                {
                    _parents.emplace_back(_absolute_path.c_str() + _pos, _index - _pos);
                    _pos = _index + 1;
                }
            }

            inline path(const path& other)
                : _location{other._location},
                  _parents{other._parents}
            {
                _directory = opendir(absolute_name().c_str());
            }

            inline path(path&& other)
                : _location{move(other._location)},
                  _parents{move(other._parents)}
            {
                swap(_directory, other._directory);
            }

            inline ~path()
            {
                _destroy();
            }

            path& operator=(const path& rhs)
            {
                _location = rhs._location;
                _parents = rhs._parents;
                _destroy();
                _directory = opendir(absolute_name().c_str());
                return *this;
            }

            path& operator=(path&& rhs)
            {
                swap(_location, rhs._location);
                swap(_directory, rhs._directory);
                swap(_parents, rhs._parents);
                return *this;
            }

            inline bool operator==(const path& rhs) const
            {
                return absolute_name() == rhs.absolute_name();
            }

            inline bool operator!=(const path& rhs) const
            {
                return absolute_name() != rhs.absolute_name();
            }

            inline bool operator<(const path& rhs) const
            {
                return absolute_name() < rhs.absolute_name();
            }

            inline bool operator<=(const path& rhs) const
            {
                return absolute_name() <= rhs.absolute_name();
            }

            inline bool operator>(const path& rhs) const
            {
                return absolute_name() > rhs.absolute_name();
            }

            inline bool operator>=(const path& rhs) const
            {
                return absolute_name() >= rhs.absolute_name();
            }

            static inline path root()
            {
                return path{string("/", 1)};
            }

            inline path parent()
            {
                if(absolute_name() == "/")
                    return root();

                return path{(relative_name() + "/..")};
            }

            inline auto list()
                -> Result<hsd::vector<path>, runtime_error>
            {
                if(_directory != nullptr)
                {
                    hsd::vector<path> _paths;
                    dirent* _dir_entry;

                    while ((_dir_entry = readdir(_directory)))
                    {
                        if(*(relative_name().end() - 1) == '/')
                        {
                            _paths.emplace_back(relative_name() + 
                                _dir_entry->d_name);
                        }
                        else
                        {
                            _paths.emplace_back(relative_name() + 
                                "/" + _dir_entry->d_name);
                        }
                    }

                    return move(_paths);
                }

                return runtime_error{"No such directory"};
            }

            inline const string& relative_name() const
            {
                return _location._relative;
            }

            inline const string& absolute_name() const
            {
                return _location._absolute;
            }

            inline string extension()
            {
                if(status().is_regular_file().unwrap())
                {
                    usize _point_pos = relative_name().rfind('.');

                    if(_point_pos != string::npos)
                    {
                        const char* _str_data = relative_name().c_str();
                        usize _str_size = relative_name().size();
                        return string (
                            _str_data + _point_pos, 
                            _str_size - _point_pos
                        );
                    }

                    return string{};
                }

                // will return an empty string instead
                return string{};
            }

            path operator/(const char* rhs) const
            {
                if(rhs[0] != '/')
                    return path{relative_name() + "/" + rhs};

                return path{relative_name() + rhs};
            }

            path operator/(const string& rhs) const
            {
                if(rhs[0] != '/')
                    return path{relative_name() + "/" + rhs};

                return path{relative_name() + rhs};
            }

            path operator/(const path& rhs) const
            {
                if(rhs.relative_name()[0] != '/')
                    return path{relative_name() + "/" + rhs.relative_name()};

                return path{relative_name() + rhs.relative_name()};
            }

            path& operator/=(const char* rhs)
            {
                *this = move(*this / rhs);
                return *this;
            }

            path& operator/=(const string& rhs)
            {
                *this = move(*this / rhs);
                return *this;
            }

            path& operator/=(const path& rhs)
            {
                *this = move(*this / rhs);
                return *this;
            }

            inline fs_status status() const
            {
                return {relative_name().c_str()};
            }

            inline auto begin()
            {
                return _parents.begin();
            }

            inline auto end()
            {
                return _parents.end();
            }
        };
    } // namespace filesystem
    #endif
} // namespace hsd
