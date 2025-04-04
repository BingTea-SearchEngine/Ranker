#ifndef ALGORITHM_H
#define ALGORITHM_H

template <typename T>
int binary_search(T* values, T& target, unsigned size) {
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
    if (left >= right)
        return;

    unsigned pivot = values[(left + right) / 2];

    int i = left;
    int j = right;

    while (i <= j) {
        while (values[i] < pivot)
            i++;
        while (pivot < values[j])
            j--;
        if (i <= j) {
            unsigned temp = values[i];
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

class RNG {
    private:
    // Internal state (seed)
    long seed;
    static const long a = 16807;
    static const long m = 2147483647; // 2^31 - 1

public:
    // Constructor with a default seed (must be non-zero)
    RNG(long init_seed = 1) {
        if (init_seed == 0)
            init_seed = 1;
        seed = init_seed;
    }

    // Generates the next random integer
    long gen_int() {
        seed = (a * seed) % m;
        return seed;
    }

    // Generates a random float in the range [0, 1)
    double gen_double() {
        return static_cast<double>(gen_int()) / m;
    }

    template <typename T>
    void shuffle(T* array, unsigned n) {
        for (unsigned i = 0; i < n - 1; ++i) {
            unsigned j = gen_int() % n;
            T temp = array[i]; // swap semantics are better but whatever
            array[i] = array[j];
            array[j] = temp;
        }
    }
};

#endif