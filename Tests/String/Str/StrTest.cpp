#include "../../../cpp/String.hpp"
#include <stdio.h>

int main()
{
    hsd::string str = "this is a test";
    auto str2 = str;
    
    printf("%s\n", hsd::string::to_string(-4.12f).c_str());
    
    printf("%s\n", (",\nno it\'s not\n" + str).data());
    
    printf("%s\n", str.gen_range<0, 4>().data());
    
    printf("%s\n", str2.data());

    printf("%zu\n", str.find('z'));
    return 0;
}