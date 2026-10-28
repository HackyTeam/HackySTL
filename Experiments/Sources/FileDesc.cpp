#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wchar.h>

int main()
{
    char buf[128]{};

    int fd = open("Tests/Experiments/Sources/foo.txt", O_RDWR | O_CREAT);
    
    if (fd == -1)
    {
        printf("Error: %s\n", strerror(errno));
        return -1;
    }

    auto res = read(fd, buf, sizeof(buf));

    if (res != -1)
    {
        printf("size = %zd\nbuffer = '%s'\n", res, buf);
        close(fd);
    }
    else
    {
        printf("Error: %s\n", strerror(errno));
        return - 1;
    }

    return 0;
}
