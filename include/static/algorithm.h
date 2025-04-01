#ifndef ALGORITHM_H
#define ALGORITHM_H

template <typename T>
int binary_search(T* values, T& target, unsigned int size) {
    int left = 0;
    int right = size - 1;    
    
    while (left <= right) {
        int mid = (left + right) / 2;
        
        if (target < values[mid])
            right = mid - 1;
        
        else if (values[mid] < target)
            left = mid + 1;
        
        else
            return mid;
    }
    
    return -1;
}

template <typename T>
void quicksort(T* values, int left, int right) {
    if (left >= right || left < 0)
        return;

    unsigned int pivot = values[(left + right) / 2];

    int i = left;
    int j = right;

    while (i <= j) {
        while (values[i] < pivot)
            i++;
        while (pivot < values[j])
            j--;
        if (i <= j) {
            unsigned int temp = values[i];
            values[i] = values[j];
            values[j] = temp;
            i++;
            j--;
        }
    }
    if (left < j)
        quicksort(values, left, j);
    if (i < right)
        quicksort(values, i, right);
}

#endif