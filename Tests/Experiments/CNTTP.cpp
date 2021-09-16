#include <type_traits>
#include <array>

struct Student
{
    int age;
    float gpa; // yes, C++20 supports it
    const char name[13];
};

template <std::size_t N>
using StudentArray = std::array<Student, N>;

template <Student s>
void invoke_single(auto) {}

template <StudentArray student, typename... Args>
requires (student.size() == sizeof...(Args))
void do_something(Args&&... args)
{
    [&]<std::size_t... Ints>(std::index_sequence<Ints...>)
    {
        (invoke_single<student[Ints]>(args), ...);
    }(std::make_index_sequence<student.size()>{});
}

int main()
{
    do_something<std::array {
        Student {
            .age = 12,
            .gpa = 2.53,
            .name = "David Jonson"
        }
    }>(0);
}