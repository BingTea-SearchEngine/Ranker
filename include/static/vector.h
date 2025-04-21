#ifndef VECTOR_H
#define VECTOR_H

#include <stdexcept>
#include <iostream>
#include "static/algorithm.h"

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

            // Change size, but don't reserve
            if (size_in <= _capacity) {
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

        void resize(unsigned size_in, const T& value) {
            unsigned old_size = _size;
            resize(size_in);
            for (unsigned i = old_size; i < _size; ++i) {
                array[i] = value;
            }
        }

        void clear() {
            resize(0);
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

        const T& operator[](unsigned index) const {
            if (index >= _size) {
                throw std::out_of_range("Index out of bounds");
            }
        
            return array[index];
        }

        T& front() {
            return operator[](0);
        }

        const T& front() const {
            return operator[](0);
        }

        T& back() {
            return operator[](_size - 1);
        }

        const T& back() const {
            return operator[](_size - 1);
        }

        T* data() {
            return array;
        }

        void sort() {
            quicksort(array, 0, _size - 1);
        }

        void print() {
            for (unsigned i = 0; i < _size; ++i)
                std::cout << array[i] << ' ';
            std::cout << std::endl;
        }

        bool empty() {
            return _size == 0;
        }
};

#endif

/*
        for (unsigned i = 0; i < network.n; ++i) {
            std::cout << i << " / " << network.n << std::endl;
            unsigned community = network.reverse_communities[indices[i]];
            unsigned node = network.remove_from_community(community);
            double max_diff = network.modularity_diff(node, community);
            unsigned new_community = community;
            for (unsigned j = 0; j < network.out_degrees[node]; ++j) {
                unsigned other_comm = network.reverse_communities[network.from_to[node][j]];
                double diff = network.modularity_diff(node, other_comm);
                if (diff > max_diff) {
                    max_diff = diff;
                    new_community = other_comm;
                }
            }
            for (unsigned j = 0; j < network.in_degrees[node]; ++j) {
                unsigned other_comm = network.reverse_communities[network.to_from[node][j]];
                double diff = network.modularity_diff(node, other_comm);
                if (diff > max_diff) {
                    max_diff = diff;
                    new_community = other_comm;
                }
            }
            network.add_to_community(node, new_community);
            if (network.communities[community].size() < network.communities[community].capacity() / 4) {
                network.communities[community].shrink_to_fit();
            }
        }
*/