#ifndef VECTOR_H
#define VECTOR_H

#include <stdexcept>

template <typename T> 
class Vector {
    private:
        T* array;
        unsigned _size;
        unsigned _capacity;
    public:
        Vector()
          : _size(0), _capacity(1) {
            array = new T[_capacity];
        }

        Vector(unsigned size_in)
          : _size(size_in), _capacity(2 * size_in) {
            array = new T[_capacity];
        }

        ~Vector() {
            delete[] array;
        }

        Vector(const Vector& other)
         : _size(other._size), _capacity(other._capacity) {
            array = new T[_capacity];
            for (unsigned i = 0; i < _size; ++i)
                array[i] = other.array[i];
        }

        Vector& operator=(const Vector& other) {
            if (this == &other)
                return *this;
        
            delete[] array;
            _size = other._size;
            _capacity = other._capacity;
            array = new T[_capacity];
        
            for (unsigned i = 0; i < _size; ++i)
                array[i] = other.array[i];
        
            return *this;
        }

        void reserve(unsigned capacity_in) {
            if (capacity_in <= _size)
                return;

            _capacity = capacity_in;
            T* new_array = new T[_capacity];

            for (unsigned i = 0; i < _size; ++i) {
                new_array[i] = array[i];
            }

            delete[] array;
            array = new_array;
        }
        
        void resize(unsigned size_in) {
            if (size_in == _size)
                return;

            // Increase size, but don't reserve
            if (size_in <= _capacity) {
                _size = size_in;
                return;
            }
            
            // Shrink (this comes after because it's less common)
            if (size_in < _size) {
                _size = size_in;
                return;
            }

            // Reverse extra capacity and increase size
            unsigned new_capacity = _capacity;
            while (new_capacity < size_in)
                new_capacity *= 2;
            
            reserve(new_capacity);
            _size = size_in;
        }

        void shrink_to_fit() {
            if (_capacity == _size)
                return;

            T* new_array = new T[_size];
            for (unsigned i = 0; i < _size; ++i) {
                new_array[i] = array[i];
            }

            delete[] array;
            array = new_array;
            _capacity = _size;
        }

        unsigned size() const {
            return _size;
        }
        
        unsigned capacity() const {
            return _capacity;
        }

        void push_back(const T& value) {
            if (_size == _capacity) {
                reserve(2 * _capacity);
            }
            array[_size] = value;
            _size++;
        
            // Alternative version that avoids code duplication. Very slightly
            // slower though. 
        
            // resize(_size + 1);
            // array[_size - 1] = value;
        }

        void pop_back() {
            if (_size != 0) {
                _size--;
            }
        }

        T& operator[](unsigned index) {
            if (index >= _size) {
                throw std::out_of_range("Index out of bounds");
            }
        
            return array[index];
        }

        T* data() {
            return array;
        }
};

#endif