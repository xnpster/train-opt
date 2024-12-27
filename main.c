#include "stdio.h"
#include "stdlib.h"

void sieve(int a, int* arr);

int main() {
    int arr[100];
    for(int i = 0; i < 100; i++)
        arr[i] = 0;
    sieve(100, arr);

    for(int i = 0; i < 100; i++)
        printf("%d\n", arr[i]);
}   