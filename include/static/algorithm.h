#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <string>
#include <fstream>

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

    T pivot = values[(left + right) / 2];

    int i = left;
    int j = right;

    while (i <= j) {
        while (values[i] < pivot)
            i++;
        while (pivot < values[j])
            j--;
        if (i <= j) {
            T temp = values[i];
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

template <typename T, typename U>
void argsort(T* indices, U* values, int left, int right) {
    if (left >= right)
        return;

    // pick pivot as the value at the middle index
    U pivot = values[indices[(left + right) / 2]];

    int i = left;
    int j = right;

    // partition based on values[indices[*]]
    while (i <= j) {
        while (values[indices[i]] < pivot)
            i++;
        while (pivot < values[indices[j]])
            j--;
        if (i <= j) {
            // swap only the indices
            T tmp = indices[i];
            indices[i] = indices[j];
            indices[j] = tmp;
            i++;
            j--;
        }
    }

    // recurse on sub‑ranges
    if (left < j)
        argsort(indices, values, left, j);
    if (i < right)
        argsort(indices, values, i, right);
}

template <typename T>
bool lt_pair(T& left_first, T& left_second, 
    T& right_first, T& right_second) {
    if (left_first < right_first)
        return true;

    if (left_first > right_first)
        return false;

    return left_second < right_second;
}

template <typename T>
void quicksort_pair(T* first, T* second, 
                     int left, int right) {
    if (left >= right || left < 0)
        return;

    T pivot_first = first[(left + right) / 2];
    T pivot_second = second[(left + right) / 2];

    int i = left;
    int j = right;

    while (i <= j) {
        while (lt_pair(first[i], second[i], pivot_first, pivot_second))
            i++;
        while (lt_pair(pivot_first, pivot_second, first[j], second[j]))
            j--;
        if (i <= j) {
            T temp = first[i];
            first[i] = first[j];
            first[j] = temp;
            temp = second[i];
            second[i] = second[j];
            second[j] = temp;
            i++;
            j--;
        }
    }
    if (left < j)
        quicksort_pair(first, second, left, j);
    if (i < right)
        quicksort_pair(first, second, i, right);
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

inline void dump_1D(unsigned* array, unsigned size, const std::string& filename) {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        // handle error
        return;
    }

    for (unsigned i = 0; i < size; ++i) {
        const auto& value = array[i];
        ofs.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
}

//template <typename T> if this isn't unsigned ints, it wouldn't be readable
inline void dump_2D(unsigned** array, unsigned num_rows, unsigned* sizes, const std::string& filename) {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        // handle error
        return;
    }
    for (unsigned i = 0; i < num_rows; ++i) {
        auto& row = array[i];
        // Write each value in the row
        for (unsigned j = 0; j < sizes[i]; ++j) {
            const auto& value = row[j];
            ofs.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }
        // Write sentinel
        unsigned sentinel = -1;
        ofs.write(reinterpret_cast<const char*>(&sentinel), sizeof(sentinel));
    }
}

template <typename T>
const T& max(const T& first, const T& second) {
    return (first < second) ? second: first;
}

template <typename T>
const T& min(const T& first, const T& second) {
    return (first < second) ? first: second;
}

#endif