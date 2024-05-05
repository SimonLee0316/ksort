#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define KSORT_DEV "/dev/sort"
#define TIMSORT 0
#define PDQSORT 1
#define LINUXSORT 2
#define QSORT 3

int main()
{
    bool pass = true;
    int ret = 0;

    struct timespec Timsort_start, Timsort_end;
    struct timespec Linuxsort_start, Linuxsort_end;
    struct timespec Qsort_start, Qsort_end;

    long long Timsort_diff;
    long long Linuxsort_diff;
    long long Qsort_diff;


    int fd = open(KSORT_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        goto error;
    }

    FILE *file = fopen("output.csv", "a");
    if (file == NULL) {
        perror("fopen");
        goto error;
    }

    for (size_t start = 1000; start <= 1000; start += 500) {
        size_t n_elements = start;
        size_t size = n_elements * sizeof(int);
        int *inbuf = malloc(size);

        size_t typesize = sizeof(int);
        int *typebuf = malloc(sizeof(int));

        if (!inbuf || !typebuf) {
            free(inbuf);
            free(typebuf);
            goto error;
        }

        { /*Timsort test*/
            for (size_t i = 0; i < n_elements; i++)
                inbuf[i] = rand() % n_elements;

            // typebuf[0] = TIMSORT;
            // typebuf[0] = LINUXSORT;
            // typebuf[0] = QSORT;
            typebuf[0] = TIMSORT;

            ssize_t type_sz = write(fd, typebuf, typesize);
            if (type_sz != typesize) {
                perror("Failed to cheange type of sort");
                free(inbuf);
                goto error;
            }

            if (clock_gettime(CLOCK_MONOTONIC, &Timsort_start) != 0) {
                perror("clock_gettime");
                free(inbuf);
                goto error;
            }

            ssize_t r_sz = read(fd, inbuf, size);

            if (clock_gettime(CLOCK_MONOTONIC, &Timsort_end) != 0) {
                perror("clock_gettime");
                free(inbuf);
                goto error;
            }

            Timsort_diff =
                (Timsort_end.tv_sec - Timsort_start.tv_sec) * 1000000000LL +
                (Timsort_end.tv_nsec - Timsort_start.tv_nsec);
            // printf("TIMSORT Time taken: %lld nanoseconds\n", Timsort_diff);

            if (r_sz != size) {
                perror("Failed to write character device");
                free(inbuf);
                goto error;
            }

            pass = true;
            ret = 0;
            /* Verify the result of sorting */

            for (size_t i = 1; i < n_elements; i++) {
                if (inbuf[i] < inbuf[i - 1]) {
                    pass = false;
                    break;
                }
            }

            printf("Soring %s!\n", pass ? "succeeded" : "failed");
        }

        { /*Linux sort test*/
            for (size_t i = 0; i < n_elements; i++)
                inbuf[i] = rand() % n_elements;

            // typebuf[0] = TIMSORT;
            // typebuf[0] = LINUXSORT;
            // typebuf[0] = QSORT;
            typebuf[0] = LINUXSORT;

            ssize_t type_sz = write(fd, typebuf, typesize);
            if (type_sz != typesize) {
                perror("Failed to cheange type of sort");
                free(inbuf);
                goto error;
            }

            if (clock_gettime(CLOCK_MONOTONIC, &Linuxsort_start) != 0) {
                perror("clock_gettime");
                free(inbuf);
                goto error;
            }

            ssize_t r_sz = read(fd, inbuf, size);

            if (clock_gettime(CLOCK_MONOTONIC, &Linuxsort_end) != 0) {
                perror("clock_gettime");
                free(inbuf);
                goto error;
            }

            Linuxsort_diff =
                (Linuxsort_end.tv_sec - Linuxsort_start.tv_sec) * 1000000000LL +
                (Linuxsort_end.tv_nsec - Linuxsort_start.tv_nsec);

            // printf("LINUXSORT Time taken: %lld nanoseconds\n",
            // Linuxsort_diff);

            if (r_sz != size) {
                perror("Failed to write character device");
                free(inbuf);
                goto error;
            }

            pass = true;
            ret = 0;
            /* Verify the result of sorting */

            for (size_t i = 1; i < n_elements; i++) {
                if (inbuf[i] < inbuf[i - 1]) {
                    pass = false;
                    break;
                }
            }

            printf("Soring %s!\n", pass ? "succeeded" : "failed");
        }

        { /*qsort test*/
            for (size_t i = 0; i < n_elements; i++)
                inbuf[i] = rand() % n_elements;

            // typebuf[0] = TIMSORT;
            // typebuf[0] = LINUXSORT;
            // typebuf[0] = QSORT;
            typebuf[0] = QSORT;

            ssize_t type_sz = write(fd, typebuf, typesize);
            if (type_sz != typesize) {
                perror("Failed to cheange type of sort");
                free(inbuf);
                goto error;
            }

            if (clock_gettime(CLOCK_MONOTONIC, &Qsort_start) != 0) {
                perror("clock_gettime");
                free(inbuf);
                goto error;
            }

            ssize_t r_sz = read(fd, inbuf, size);

            if (clock_gettime(CLOCK_MONOTONIC, &Qsort_end) != 0) {
                perror("clock_gettime");
                free(inbuf);
                goto error;
            }

            Qsort_diff =
                (Qsort_end.tv_sec - Qsort_start.tv_sec) * 1000000000LL +
                (Qsort_end.tv_nsec - Qsort_start.tv_nsec);

            // printf("QSORT Time taken: %lld nanoseconds\n", Qsort_diff);

            if (r_sz != size) {
                perror("Failed to write character device");
                free(inbuf);
                goto error;
            }

            pass = true;
            ret = 0;
            /* Verify the result of sorting */

            for (size_t i = 1; i < n_elements; i++) {
                if (inbuf[i] < inbuf[i - 1]) {
                    pass = false;
                    break;
                }
            }

            printf("Soring %s!\n", pass ? "succeeded" : "failed");
        }

        fprintf(file, "%zu,%lld,%lld,%lld\n", n_elements, Timsort_diff,
                Linuxsort_diff, Qsort_diff);
    }
error:
    fclose(file);
    if (fd > 0)
        close(fd);
    return ret;
}
