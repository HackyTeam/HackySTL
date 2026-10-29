#include <Base/Result.hpp>
#include <string_view>

// Regression test: the type names in the "Expected X, got Y" panic message must be the names
// of the types. On GCC they used to come out as "get_type<int", which is the helper's own name.
namespace
{
    constexpr std::string_view view_of(hsd::pair<const char*, hsd::i32> name)
    {
        return std::string_view{name.first, static_cast<std::size_t>(name.second)};
    }

    template <typename T>
    constexpr std::string_view type_name()
    {
        constexpr auto _name = hsd::result_detail::get_type<T>();
        return view_of(_name);
    }
}

// The names of real types, as this compiler prints them.
static_assert(type_name<int>() == "int");
static_assert(type_name<void>() == "void");
static_assert(type_name<float>() == "float");

// Signatures in the layouts of GCC, Clang and MSVC, so each parser is checked on every layout.
static_assert(view_of(hsd::result_detail::bracket_name(
    "consteval const char* {anonymous}::sig() [with T = const int&]")) == "const int&");
static_assert(view_of(hsd::result_detail::bracket_name(
    "const char *(anonymous namespace)::sig() [T = int[3]]")) == "int[3]");
static_assert(view_of(hsd::result_detail::angle_name(
    "struct hsd::pair<char const *,int> __cdecl hsd::result_detail::get_type<int>(void)")) == "int");
static_assert(view_of(hsd::result_detail::angle_name(
    "struct hsd::pair<char const *,int> __cdecl hsd::result_detail::get_type<hsd::pair<int,float>>(void)"))
    == "hsd::pair<int,float>");

int main()
{
    return 0;
}
