#include <fcntl.h>
#include <unistd.h>

int main()
{
    char str[5] = {};
    int fd = ::open("foo.txt", O_RDONLY);
    
    auto sz = ::read(fd, str, 4);
    auto c = ::lseek(fd, -sz, SEEK_CUR);
    ::read(fd, str, 4);
    ::write(1, str, 5);
    
    ::close(fd);
}