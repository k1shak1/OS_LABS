#include <stdlib.h>

float Pi(int K) {
    float pi = 1.0;
    
    for (int i = 1; i <= K; i++) {
        float num = 4.0 * i * i;
        float den = 4.0 * i * i - 1.0;
        pi *= num / den;
    }
    
    return pi * 2;
}

void qsort_impl(int* arr, int low, int high) {
    if (low >= high) return;
    
    int pivot = arr[(low + high) / 2];
    int i = low, j = high;
    
    while (i <= j) {
        while (arr[i] < pivot) i++;
        while (arr[j] > pivot) j--;
        if (i <= j) {
            int tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
            i++; j--;
        }
    }
    
    qsort_impl(arr, low, j);
    qsort_impl(arr, i, high);
}

int* Sort(int* array, int size) {
    if (size <= 0) {
        int* empty = (int*)malloc(sizeof(int));
        return empty;
    }
    
    int* sorted = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        sorted[i] = array[i];
    }
    
    if (size > 1) {
        qsort_impl(sorted, 0, size - 1);
    }
    
    return sorted;
}