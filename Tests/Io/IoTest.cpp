#include <Io.hpp>
#include <String.hpp>

class CustomObject
{
public:
    enum class ErrorType
    {
        None,
        InvalidArgument,
        InvalidFormat,
        InvalidIndex,
        InvalidLength,
        InvalidRange,
        InvalidSize,
        InvalidType,
        OutOfRange,
        Overflow,
        Underflow,
        Unsupported,
        UnsupportedOperation
    };

private:
    hsd::string error_text;
    ErrorType error_type;

public:
    inline CustomObject(const hsd::string& _error_text, ErrorType _error_type)
    {
        error_text = _error_text;
        error_type = _error_type;
    }

    template <typename CharT>
    static consteval auto pretty_fmt()
    {
        static_assert(
            hsd::is_same<CharT, char>::value ||
            hsd::is_same<CharT, hsd::wchar>::value,
            "CharT must be either char or hsd::wchar"
        );

        if constexpr (hsd::is_same<CharT, char>::value)
        {
            return hsd::string_literal<54>{
                "CustomObject(error_text: {fg=9}, error_type: {fg=14})"
            };
        }
        else
        {
            return hsd::wstring_literal<54>{
                L"CustomObject(error_text: {fg=9}, error_type: {fg=14})"
            };
        }
    }

    constexpr auto pretty_args() const
        -> hsd::tuple<const char*, const char*>
    {
        switch (error_type)
        {
            case ErrorType::None:
            {
                return hsd::tuple<const char*, const char*>{
                    error_text.c_str(), "None"
                };
            }
            case ErrorType::InvalidArgument:
            {
                return hsd::tuple<const char*, const char*>{
                    error_text.c_str(), "InvalidArgument"
                };
            }
            case ErrorType::InvalidFormat:
            {
                return hsd::tuple<const char*, const char*>{
                    error_text.c_str(), "InvalidFormat"
                };
            }
            case ErrorType::InvalidIndex:
            {
                return hsd::tuple<const char*, const char*>{
                    error_text.c_str(), "InvalidIndex"
                };
            }
            case ErrorType::InvalidLength:
            {
                return hsd::tuple<const char*, const char*>{
                    error_text.c_str(), "InvalidLength"
                };
            }
            case ErrorType::InvalidRange:
            {
                return hsd::tuple<const char*, const char*>{
                    error_text.c_str(), "InvalidRange"
                };
            }
            case ErrorType::InvalidSize:
            {
                return hsd::tuple<const char*, const char*>{
                    error_text.c_str(), "InvalidSize"
                };
            }
            case ErrorType::InvalidType:
            {
                return hsd::tuple<const char*, const char*>{
                    error_text.c_str(), "InvalidType"
                };
            }
            case ErrorType::OutOfRange:
            {
                return hsd::tuple<const char*, const char*>{
                    error_text.c_str(), "OutOfRange"
                };
            }
            case ErrorType::Overflow:
            {
                return hsd::tuple<const char*, const char*>{
                    error_text.c_str(), "Overflow"
                };
            }
            case ErrorType::Underflow:
            {
                return hsd::tuple<const char*, const char*>{
                    error_text.c_str(), "Underflow"
                };
            }
            case ErrorType::Unsupported:
            {
                return hsd::tuple<const char*, const char*>{
                    error_text.c_str(), "Unsupported"
                };
            }
            case ErrorType::UnsupportedOperation:
            {
                return hsd::tuple<const char*, const char*>{
                    error_text.c_str(), "UnsupportedOperation"
                };
            }
        }

        return hsd::tuple<const char*, const char*>{
            "Unknown", "Unknown"
        };
    }
};

int main()
{
    hsd::wsstream str;
    str.reserve(256);
    hsd::i32 x, z;
    hsd::f32 y;
    
    CustomObject obj{
        "Invalid Function Arguments", 
        CustomObject::ErrorType::InvalidArgument
    };

    hsd_wprintln(L"CustomObject Error: {ovr} ", obj);
    hsd_wprintln(L"hello, {} and other words", 123.2);
    str.write_data<L"hello, {} and other words\n">(hsd::string{"123.2"});
    
    //hsd_read_line().set_data(x, y, z).unwrap();
    hsd_read_value(x);
    hsd_read_value(y);
    hsd_read_value(z);

    hsd_wprintln(L"{x,fg=234,bg=43}, {e,fg=143,bg=78}, {fg=84,bg=193}", x, y, z);
    
    auto file = hsd::load_file<char>(
        "Tests/Io/test.txt", hsd::io_options::text::read_write
    ).unwrap();

    file.set_buffer_capacity(1024);
    
    [[maybe_unused]] auto c = 0;
    file.read_line().unwrap().set_data(c).unwrap();
    file.print<"{} {} {}">("Hello", 12, -3.4).unwrap();
}