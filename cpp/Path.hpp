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
        namespace fs_detail
        {
            inline auto copy_file(const char* from, const char* to)
                -> Result<void, runtime_error>
            {
                i32 _value;
                using file_type = FILE*;
                file_type _from_file = fopen(from, "r");
                file_type _to_file = fopen(to, "w");
                
                if(_from_file == nullptr || _to_file == nullptr)
                {
                    return runtime_error{"File not found"};
                }

                for (_value = fgetc(_from_file); _value != EOF; _value = fgetc(_from_file)) 
                {
                    if (fputc(_value, _to_file) == EOF)
                    {
                        fclose(_to_file);
                        fclose(_from_file);
                        return runtime_error{"Couldn't write into the file"};
                    }
                }

                fclose(_to_file);
                fclose(_from_file);
                return {};
            }
        } // namespace fs_detail
        

        class path
        {
        private:
            DIR* _directory = nullptr;
            fs_detail::Location _location;

            static constexpr auto _default_perms = 
                fs_perms_mask::owner_all;

            void _destroy()
            {
                if (_directory != nullptr)
                    closedir(_directory);
            }

            void _reconstruct()
            {
                if (_directory != nullptr)
                    closedir(_directory);

                _directory = 
                    opendir(absolute_name().c_str());
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
            }

            inline path(const string& raw_path)
                : _location{raw_path}
            {
                const string& _absolute_path = absolute_name();
                _directory = opendir(_absolute_path.c_str());
            }

            inline path(string&& raw_path)
                : _location{move(raw_path)}
            {
                const string& _absolute_path = absolute_name();
                _directory = opendir(_absolute_path.c_str());
            }

            inline path(const path& other)
                : _location{other._location}
            {
                _directory = opendir(absolute_name().c_str());
            }

            inline path(path&& other)
                : _location{move(other._location)}
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
                _destroy();
                _directory = opendir(absolute_name().c_str());
                return *this;
            }

            path& operator=(path&& rhs)
            {
                swap(_location, rhs._location);
                swap(_directory, rhs._directory);
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
                        if(cstring::compare(_dir_entry->d_name, ".") != 0 && 
                            cstring::compare(_dir_entry->d_name, "..") != 0)
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
                    }

                    _reconstruct();
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

            inline auto change_permissions(const fs_perms_mask& mask)
                -> Result<void, runtime_error>
            {
                if(!status().exists())
                {
                    return runtime_error{"File/Directory not found"};
                }
                else
                {
                    int _res = chmod(absolute_name().c_str(), static_cast<u32>(mask));

                    if(_res == -1)
                    {
                        return runtime_error{"Couldn't change permissions of the file"};
                    }   
                }

                return {};
            }

            inline auto rename(const string& new_name)
                -> Result<void, runtime_error> 
            {
                if(!status().exists())
                {
                    return runtime_error{"File/Directory not found"};
                }
                else
                {
                    usize _slash_pos = relative_name().rfind('/');
                    int _res = 0;

                    if(_slash_pos == string::npos)
                    {
                        path _new_path;
                        _new_path /= new_name;

                        _res = ::rename(
                            absolute_name().c_str(), 
                            _new_path.absolute_name().c_str()
                        );

                        if(_res == -1)
                            return runtime_error{strerror(errno)};

                        _new_path._destroy();
                        swap(*this, _new_path);
                    }
                    else
                    {
                        path _new_path{string(relative_name().c_str(), _slash_pos)};
                        _new_path /= new_name;

                        _res = ::rename(
                            absolute_name().c_str(), 
                            _new_path.absolute_name().c_str()
                        );

                        if(_res == -1)
                            return runtime_error{strerror(errno)};

                        _new_path._destroy();
                        _new_path._directory = opendir(
                            relative_name().c_str()
                        );
                        swap(*this, _new_path);
                    }
                }

                return {};
            }

            inline string extension() const
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

            inline auto copy(const path& to)
                -> Result<void, runtime_error> 
            {
                if(status().is_directory().unwrap() &&
                    to.status().is_directory().unwrap())
                {
                    for(auto& _item : list().unwrap())
                    {
                        const char* _item_name =
                            _item.absolute_name().c_str() +
                            _item.absolute_name().rfind('/');

                        path _new_to = to + _item_name;

                        if(_item.status().is_directory().unwrap())
                        {
                            mkdir(
                                _new_to.absolute_name().c_str(),
                                static_cast<u32>(_default_perms)
                            );
                        }
                        else
                        {
                            auto* _dummy_file = 
                                fopen(_new_to.absolute_name().c_str(), "w+");
                            fclose(_dummy_file);
                        }
                        _item.copy(_new_to).unwrap();
                    }
                }
                else if(status().is_regular_file().unwrap() &&
                    to.status().is_regular_file().unwrap())
                {
                    return fs_detail::copy_file(
                        absolute_name().c_str(),
                        to.absolute_name().c_str()
                    );
                }
                else if(status().is_regular_file().unwrap() &&
                    to.status().is_directory().unwrap())
                {
                    const char* _item_name =
                        absolute_name().c_str() +
                        absolute_name().rfind('/');

                    path _new_to = to + _item_name;
                    return fs_detail::copy_file(
                        absolute_name().c_str(), 
                        _new_to.absolute_name().c_str()
                    );
                }
                else if(status().is_directory().unwrap() &&
                    to.status().is_regular_file().unwrap())
                {
                    return runtime_error{
                        "Cannot copy a directory into a file"
                    };
                }
                else
                {
                    return runtime_error{"Unreachable code"};
                }
                
                return {};
            }

            inline auto create_directory(const string& name) const
                -> Result<void, runtime_error>
            {
                if(name.rfind('/') < name.size() - 1)
                {
                    return runtime_error{
                        "Cannot create multiple directories"
                    };
                }

                mkdir(
                    (absolute_name() + "/" + name).c_str(), 
                    static_cast<u32>(_default_perms)
                );
                return {};
            }

            inline path operator+(const char* rhs) const
            {
                return path{relative_name() + rhs};
            }

            inline path operator+(const string& rhs) const
            {
                return path{relative_name() + rhs};
            }

            inline path operator+(const path& rhs) const
            {
                return path{relative_name() + rhs.relative_name()};
            }

            inline path& operator+=(const char* rhs)
            {
                *this = move(*this + rhs);
                return *this;
            }

            inline path& operator+=(const string& rhs)
            {
                *this = move(*this + rhs);
                return *this;
            }

            inline path& operator+=(const path& rhs)
            {
                *this = move(*this + rhs);
                return *this;
            }

            inline path operator/(const char* rhs) const
            {
                if(rhs[0] != '/')
                    return path{relative_name() + "/" + rhs};

                return path{relative_name() + rhs};
            }

            inline path operator/(const string& rhs) const
            {
                if(rhs[0] != '/')
                    return path{relative_name() + "/" + rhs};

                return path{relative_name() + rhs};
            }

            inline path operator/(const path& rhs) const
            {
                if(rhs.relative_name()[0] != '/')
                    return path{relative_name() + "/" + rhs.relative_name()};

                return path{relative_name() + rhs.relative_name()};
            }

            inline path& operator/=(const char* rhs)
            {
                *this = move(*this / rhs);
                return *this;
            }

            inline path& operator/=(const string& rhs)
            {
                *this = move(*this / rhs);
                return *this;
            }

            inline path& operator/=(const path& rhs)
            {
                *this = move(*this / rhs);
                return *this;
            }

            inline fs_status status() const
            {
                return {relative_name().c_str()};
            }

            inline auto parents()
            {
                usize _index, _pos;
                vector<string> _parents;
                _parents.emplace_back("/", 1);

                for (_index = absolute_name().find('/', 1), _pos = 1; 
                    _index != string::npos; _index = absolute_name().find('/', _pos))
                {
                    _parents.emplace_back(
                        absolute_name().c_str() 
                        + _pos, _index - _pos
                    );
                    _pos = _index + 1;
                }

                return _parents;
            }
        };
    } // namespace filesystem
    #endif
} // namespace hsd
