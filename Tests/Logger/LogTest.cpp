#include "../../cpp/Io.hpp"
#include "../../cpp/Logging.hpp"

void log(const char* message)
{
    hsd::logger location = hsd::logger::current();
    hsd::io::print("info: {}:{} {}\n", location.file_name(), location.line(), message);
}
 
int main()
{
    log("Hello world!");
}