#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include "sort_types.h"

#define KSORT_DEV "/dev/sort"
#define TIMSORT 0
#define LINUXSORT 1
#define PDQSORT 2
#define QSORT 3


typedef struct {
    unsigned long long qsort_user;
    unsigned long long timsort_user;
    unsigned long long linuxsort_user;
    unsigned long long qsort_kernal;
    unsigned long long timsort_kernal;
    unsigned long long linuxsort_kernal;
} sorttime;

sorttime time_analysis(size_t);

int main()
{
    FILE *file = fopen("output.csv", "w");
    sorttime result;
    size_t start = 1000, end = 20000;
    size_t step = 500;

    for (size_t k = start; k <= end; k += step) {
        result = time_analysis(k);
        fprintf(file, "%zu,%llu,%llu,%llu,%llu,%llu,%llu\n", k,
                result.qsort_user, result.timsort_user, result.linuxsort_user,
                result.qsort_kernal, result.timsort_kernal,
                result.linuxsort_kernal);
    }
    fclose(file);
    return 0;
}

sorttime time_analysis(size_t num)
{
    int fd = open(KSORT_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        goto error;
    }

    bool pass = true;

    struct timespec start, end;
    sorttime time;

    size_t n_elements = num;
    size_t size = n_elements * sizeof(int);
    int *inbuf = malloc(size);


    if (!inbuf) {
        goto error;
    }

    { /*Timsort test*/
        for (size_t i = 0; i < n_elements; i++)
            inbuf[i] = rand() % n_elements;

        sort_method_t method = TIMSORT;
        if (write(fd, &method, sizeof(method)) != sizeof(method)) {
            perror("Failed to set sort method");
            close(fd);
            goto error;
        }

        printf("Sorting with Timsort\n");

        clock_gettime(CLOCK_MONOTONIC, &start);

        ssize_t r_sz = read(fd, inbuf, size);

        clock_gettime(CLOCK_MONOTONIC, &end);


        // printf("TIMSORT Time taken: %lld nanoseconds\n", Timsort_diff);

        if (r_sz != size) {
            perror("Failed to write character device");
            goto error;
        }

        time.timsort_user = (end.tv_sec - start.tv_sec) * 1000000000LL +
                            (end.tv_nsec - start.tv_nsec);
        pass = true;
        /* Verify the result of sorting */

        for (size_t i = 1; i < n_elements; i++) {
            if (inbuf[i] < inbuf[i - 1]) {
                pass = false;
                break;
            }
        }
        time.timsort_kernal = (unsigned long long) ioctl(fd, 0, 0);
        // printf("tim sort kernal time %llu\n", time.timsort_kernal);

        printf("Soring %s!\n", pass ? "succeeded" : "failed");
    }

    { /*Linux sort test*/
        for (size_t i = 0; i < n_elements; i++)
            inbuf[i] = rand() % n_elements;

        sort_method_t method = LINUXSORT;
        if (write(fd, &method, sizeof(method)) != sizeof(method)) {
            perror("Failed to set sort method");
            close(fd);
            goto error;
        }

        printf("Sorting with linux_sort\n");
        clock_gettime(CLOCK_MONOTONIC, &start);

        ssize_t r_sz = read(fd, inbuf, size);

        clock_gettime(CLOCK_MONOTONIC, &end);

        // printf("LINUXSORT Time taken: %lld nanoseconds\n",
        // Linuxsort_diff);

        if (r_sz != size) {
            perror("Failed to write character device");
            goto error;
        }

        time.linuxsort_user = (end.tv_sec - start.tv_sec) * 1000000000LL +
                              (end.tv_nsec - start.tv_nsec);

        pass = true;
        /* Verify the result of sorting */

        for (size_t i = 1; i < n_elements; i++) {
            if (inbuf[i] < inbuf[i - 1]) {
                pass = false;
                break;
            }
        }

        time.linuxsort_kernal = (unsigned long long) ioctl(fd, 0, 0);
        // printf("tim sort kernal time %llu\n", time.linuxsort_kernal);

        printf("Soring %s!\n", pass ? "succeeded" : "failed");
    }

    { /*qsort test*/
        for (size_t i = 0; i < n_elements; i++)
            inbuf[i] = rand() % n_elements;

        sort_method_t method = QSORT;
        if (write(fd, &method, sizeof(method)) != sizeof(method)) {
            perror("Failed to set sort method");
            close(fd);
            goto error;
        }

        printf("Sorting with qsort\n");
        clock_gettime(CLOCK_MONOTONIC, &start);

        ssize_t r_sz = read(fd, inbuf, size);

        clock_gettime(CLOCK_MONOTONIC, &end);


        // printf("QSORT Time taken: %lld nanoseconds\n", Qsort_diff);

        if (r_sz != size) {
            perror("Failed to write character device");
            goto error;
        }
        time.qsort_user = (end.tv_sec - start.tv_sec) * 1000000000LL +
                          (end.tv_nsec - start.tv_nsec);

        pass = true;
        /* Verify the result of sorting */

        for (size_t i = 1; i < n_elements; i++) {
            if (inbuf[i] < inbuf[i - 1]) {
                pass = false;
                break;
            }
        }

        time.qsort_kernal = (unsigned long long) ioctl(fd, 0, 0);
        // printf("tim sort kernal time %llu\n", time.qsort_kernal);

        printf("Soring %s!\n", pass ? "succeeded" : "failed");
    }
error:
    free(inbuf);
    if (fd > 0)
        close(fd);
    return time;
}
