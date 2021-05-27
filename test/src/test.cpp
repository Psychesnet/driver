#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define CDEVICE "/dev/cdevice0"

int main(int argc, char *argv[])
{
    int fd = 0;
    char buf[8] = {0};
    char val = 0xff;
    ssize_t count = 0;
    do {
        fd = open(CDEVICE, O_RDWR);
        if (fd < 0) {
            fprintf(stderr, "fail to open %s\n", CDEVICE);
            break;
        }

        // let's read something
        count = read(fd, buf, sizeof(buf));
        if (count > 0) {
            for (int i = 0; i < count; ++i) {
                fprintf(stderr, "%02x ", buf[i]);
            }
            fprintf(stderr, "\n");
        }

        // let's write something
        val = 0xc;
        if (write(fd, &val, 1) < 0) {
            fprintf(stderr, "fail to write %s\n", CDEVICE);
        }

        // let's read something
        count = read(fd, buf, sizeof(buf));
        if (count > 0) {
            for (int i = 0; i < count; ++i) {
                fprintf(stderr, "%02x ", buf[i]);
            }
            fprintf(stderr, "\n");
        }

        close(fd);
    } while (false);
    return 0;
}
