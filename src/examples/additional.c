#include <stdio.h>
#include "lib/stdlib.h"
#include <syscall.h>

int main (int argc, const char* argv[]){
    int num[4];

    if(argc != 5)
        return EXIT_FAILURE;

    for(int i=0 ; i < 4 ; i++) {
        // why won't you use strtol?
        num[i] = atoi(argv[i+1]);
    }

    printf(
            "%d %d\n",
            fibonacci(num[0]),
            max_of_four_int(num[0], num[1], num[2], num[3])
          );

    return EXIT_SUCCESS;
}
