#include <stddef.h>

static void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

static size_t qsort_partition(int *arr, size_t len) {
    int pivot = arr[0];
    size_t i = 1, j = len;
    while (i < j) {
        if (arr[i] <= pivot) {
            i++;
        } else if (arr[j-1] >= pivot) {
            j--;
        } else {
            j--;
            swap(&arr[i], &arr[j]);
            i++;
        }
    }
    if (i > 1) {
        i--;
        swap(&arr[0], &arr[i]);
        return i;
    } else {
        return 0;
    }
}

void qsort(int *arr, size_t len) {
    if (len > 1) {
        size_t pivot_idx = qsort_partition(arr, len);
        qsort(arr, pivot_idx);
        qsort(arr + pivot_idx + 1, len - (pivot_idx + 1));
    }
}