#include <Io.hpp>
#include <String.hpp>

using namespace hsd::format_literals;

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
            constexpr auto _fmt = 
                "CustomObject(error_text: {bold,blink,fg=9}, error_type: {bold,fg=14})"_fmt;

            return _fmt();
        }
        else
        {
            constexpr auto _fmt = 
                L"CustomObject(error_text: {bold,blink,fg=9}, error_type: {bold,fg=14})"_fmt;

            return _fmt();
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
    hsd::sstream ss;
    ss.reserve(256);
    hsd::i32 x, z;
    hsd::f32 y;

    hsd::string str{"123.2"};
    hsd::io::cin().get_stream().reserve(1024);
    
    CustomObject obj{
        "Invalid Function Arguments", 
        CustomObject::ErrorType::InvalidArgument
    };

    hsd::println("CustomObject Error: {ovr} "_fmt, obj);
    hsd::println("hello, {} and other words"_fmt, 123.2);
    ss.write_data<"hello, {} and other words\n">(str);
    
    hsd::io::cin().
    read_chunk().
    unwrap().
    get_stream().
    set_data(x, y, z).
    unwrap();
    
    hsd::io::cin().
    read_chunk().
    unwrap().
    get_stream().
    set_data(x, y, z).
    unwrap();
    
    hsd::println(
        "{italic,hex,fg=234,bg=255}, "
        "{undrln,exp,fg=143,bg=95}, "
        "{strike,fg=84,bg=105}"_fmt, 
        x, y, z
    );
    
    auto file = hsd::io::load_file(
        "Tests/Io/test.txt", hsd::io_options::read_write
    ).unwrap();

    file.get_stream().reserve(1024);
    
    [[maybe_unused]] auto c = 0;
    
    file.
    read_chunk().
    unwrap().
    get_stream().
    set_data(c).
    unwrap();
    
    file.print<"{} {} {}">("Hello", 12, -3.4);
}