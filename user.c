#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define KSORT_DEV "/dev/sort"
#define TIMSORT 0
#define PDQSORT 1
#define LINUXSORT 2
#define QSORT 3

int main()
{
    int fd = open(KSORT_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        goto error;
    }

    size_t n_elements = 10;
    size_t size = n_elements * sizeof(int);
    int *inbuf = malloc(size);

    size_t typesize = sizeof(int);
    int *typebuf = malloc(sizeof(int));

    if (!inbuf || !typebuf)
        goto error;

    for (size_t i = 0; i < n_elements; i++)
        inbuf[i] = rand() % n_elements;


    // typebuf[0] = TIMSORT;
    // typebuf[0] = PDQSORT;
    // typebuf[0] = LINUXSORT;
    typebuf[0] = TIMSORT;

    ssize_t type_sz = write(fd, typebuf, typesize);
    if (type_sz != typesize) {
        perror("Failed to cheange type of sort");
        goto error;
    }

    ssize_t r_sz = read(fd, inbuf, size);
    if (r_sz != size) {
        perror("Failed to write character device");
        goto error;
    }

    bool pass = true;
    int ret = 0;
    /* Verify the result of sorting */

    for (size_t i = 1; i < n_elements; i++) {
        if (inbuf[i] < inbuf[i - 1]) {
            pass = false;
            break;
        }
    }

    printf("Soring %s!\n", pass ? "succeeded" : "failed");

error:
    free(inbuf);
    if (fd > 0)
        close(fd);
    return ret;
}
