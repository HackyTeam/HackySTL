#include <ArgParse.hpp>

int main(int argc, const char** argv)
{
    auto arg_parser = hsd::argument_parser{"This is a test"};

    arg_parser.add("-clear", 0, [](hsd::sstream&){printf("\e[1;1H\e[2J");}, "-clear: Clears the console");
    arg_parser.add("-sum", 2, [](hsd::sstream& stream)
    {
        int a, b;
        stream.set_data(a, b).unwrap(HSD_FUNCTION_NAME);
        printf("%d\n", a + b);
    }, "-sum num1 num2: Sums two numbers");
    arg_parser.parse(argc, argv);
}