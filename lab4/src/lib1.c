#include <stdlib.h>

float Pi(int K) {
    float pi = 0.0;
    int sign = 1;
    
    for (int i = 0; i < K; i++) {
        pi += sign * 4.0 / (2 * i + 1);
        sign = -sign;
    }
    
    return pi;
}

int* Sort(int* array, int size) {
    int* sorted = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        sorted[i] = array[i];
    }
    
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - 1; j++) {
            if (sorted[j] > sorted[j + 1]) {
                int temp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = temp;
            }
        }
    }
    
    return sorted;
}