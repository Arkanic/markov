#include <stdlib.h>
#include <time.h>

void rand_init(void) {
    srand(time(NULL));
}

unsigned long long rand_num(unsigned long long min, unsigned long long max) {
    return rand() % (max + 1 - min) + min;
}