#include <cassert>
#include <Serialize/Io.hpp>
#include <Containers/String.hpp>

using namespace hsd::format_literals;

struct TestingOVR
{
    hsd::i32 nr;
    const char* chr;
    hsd::f32 fp;

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
                "TestingOVR: [{}, {}, {exp}]"_fmt;

            return _fmt();
        }
        else
        {
            constexpr auto _fmt = 
                L"TestingOVR: [{}, {}, {exp}]"_fmt;

            return _fmt();
        }
    }

    constexpr auto pretty_args() const
        -> hsd::tuple<hsd::i32, const char*, hsd::f32>
    {
        return {nr, chr, fp};
    }
};

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
    TestingOVR test;

public:
    inline CustomObject(const hsd::string& _error_text, ErrorType _error_type)
        : error_text{_error_text}, error_type{_error_type}, test{1, "a", 0x1.aad12p-2}
    {}

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
                "CustomObject(error_text: {bold,blink,fg=9}, error_type: {bold,fg=14}, {ovr})"_fmt;

            return _fmt();
        }
        else
        {
            constexpr auto _fmt = 
                L"CustomObject(error_text: {bold,blink,fg=9}, error_type: {bold,fg=14}, {ovr})"_fmt;

            return _fmt();
        }
    }

    constexpr auto pretty_args() const
        -> hsd::tuple<const char*, const char*, TestingOVR>
    {
        constexpr hsd::stack_array arr = {
            "None",
            "InvalidArgument",
            "InvalidFormat",
            "InvalidIndex",
            "InvalidLength",
            "InvalidRange",
            "InvalidSize",
            "InvalidType",
            "OutOfRange",
            "Overflow",
            "Underflow",
            "Unsupported",
            "UnsupportedOperation"
        };
        
        switch (error_type)
        {
            default: return {
                error_text.c_str(), arr[static_cast<hsd::usize>(error_type)], test
            };
        }
    
        return {"", "", TestingOVR{}};
    }
};

static constexpr auto func()
{
    hsd::static_sstream<256> str;

    str.write_data("{} {} {exp}\n"_fmt, 132, "ana", 0.000005);

    return str;
}

int main()
{
    constexpr auto hope = func();

    hsd::sstream ss;
    ss.reserve(256);
    hsd::i32 x, z;
    hsd::f32 y = 0x1.aad12p-2;

    hsd::string str{"123.2"};

    CustomObject obj{
        "Invalid Function Arguments", 
        CustomObject::ErrorType::InvalidArgument
    };

    assert(hope.size() > 0);
    assert(obj.pretty_args().get<0>() != nullptr);
    ss.write_data("hello, {} and other words\n"_fmt, str);
    
    hsd::io::cin()
    .read_chunk()
    .unwrap()
    .get()
    .set_data(x, y, z)
    .unwrap();

    assert(x == 1 && y == 2.5f && z == 3);
    
    hsd::io::cin()
    .read_chunk()
    .unwrap()
    .get()
    .set_data(x, y, z)
    .unwrap();

    assert(x == 4 && y == 5.5f && z == 6);
    
    auto file = hsd::io::load_file(
        "test.txt", hsd::io_options::read_write
    ).unwrap();
    
    [[maybe_unused]] auto c = 0;
    
    file
        .read_chunk()
        .unwrap()
        .get()
        .set_data(c)
        .unwrap();

    assert(c == 123);
    
    // apparently this prints it's own buffer, including what was read
    file.print("{} {} {}\n"_fmt, "Hello", 12, -3.4).flush().unwrap();
}