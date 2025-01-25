#include <stdlib.h>
#include <time.h>

unsigned long long rand_num(unsigned long long min, unsigned long long max) {
    unsigned int seed = time(0);
    return rand_r(&seed) % (max - min + 1) + min;
}