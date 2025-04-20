#ifndef DEQUE_H
#define DEQUE_H

#include <stdexcept>
#include <iostream>

template <typename T> 
class Deque {
    private:
        T* circle;
        unsigned _size;
        unsigned _capacity;
        unsigned _front;
        unsigned _back;
    public:
        Deque()
          : _size(0), _capacity(1), _front(0), _back(0) {
            circle = new T[_capacity];
        }

        ~Deque() {
            delete[] circle;
        }

        Deque(const Deque& other)
          : _size(other._size), _capacity(other._capacity), 
            _front(other._front), _back(other._back) {
            circle = new T[_capacity];
            for (unsigned i = _front; i != (_back + 1) % _capacity; i = (i + 1) % _capacity) {
                circle[i] = other.circle[i];
            }
        }

        Deque& operator=(const Deque& other) {
            if (this == &other)
                return *this;

            delete[] circle;
            _size = other._size;
            _capacity = other._capacity;
            _front = other._front;
            _back = other._back;
            circle = new T[_capacity];

            for (unsigned i = _front; i != (_back + 1 % _capacity); i = (i + 1) % _capacity)
                circle[i] = other.circle[i];
                
            return *this;
        }

        void reserve(unsigned capacity_in) {
            if (capacity_in <= _size)
                return;

            T* new_circle = new T[capacity_in](); // These aren't needed, they're just here to shut up valgrind

            for (unsigned i = 0; i < _size; ++i) {
                new_circle[i] = circle[(i + _front) % _capacity];
            }

            delete[] circle;
            circle = new_circle;
            _front = 0;
            _back = _size - 1;
            _capacity = capacity_in;
        }

        void shrink_to_fit() {
            if (_size == 0) {
                T* new_circle = new T[1](); // These aren't needed, they're just here to shut up valgrind

                delete[] circle;
                circle = new_circle;
                _capacity = 1;
                _front = 0;
                _back = 0;
                return;
            }

            if (_size == _capacity)
                return;

            T* new_circle = new T[_size](); // These aren't needed, they're just here to shut up valgrind
            for (unsigned i = 0; i < _size; ++i) {
                new_circle[i] = this->operator[](i);
            }

            delete[] circle;
            circle = new_circle;
            _capacity = _size;
            _front = 0;
            _back = _size - 1;
        }

        unsigned size() const {
            return _size;
        }

        unsigned capacity() const {
            return _capacity;
        }

        void push_front(const T& value) {
            if (_size == 0) {
                circle[_front] = value;
                _size++;
                return;
            }

            if (_size == _capacity) {
                reserve(2 * _capacity);
            }

            _front = (_front == 0) ? _capacity - 1: _front - 1;
            circle[_front] = value;
            _size++;
        }

        void push_back(const T& value) {
            if (_size == 0) {
                circle[_front] = value;
                _size++;
                return;
            }

            if (_size == _capacity) {
                reserve(2 * _capacity);
            }

            _back = (_back + 1) % _capacity;
            circle[_back] = value;
            _size++;
        }

        void pop_front() {
            if (_size == 0)
                throw std::out_of_range("Deque is empty");;

            _front = (_front + 1) % _capacity;
            _size--;
        }

        void pop_back() {
            if (_size == 0)
                throw std::out_of_range("Deque is empty");;

            _back = (_back == 0) ? _size - 1: _back - 1;
            _size--;
        }

        T& operator[](unsigned index) {
            if (index >= _size) {
                throw std::out_of_range("Index out of bounds");
            }
        
            return circle[(index + _front) % _capacity];
        }

        T& front() {
            if (_size == 0)
                throw std::out_of_range("Deque is empty");

            return circle[_front];
        }

        T& back() {
            if (_size == 0)
                throw std::out_of_range("Deque is empty");

            return circle[_back];
        }

        void print() {
            for (unsigned i = 0; i < _capacity; ++i) {
                std::cout << circle[i] << ' ';
            }
            std::cout << std::endl;
        }

        void print_inorder() {
            for (unsigned i = 0; i < _size; ++i) {
                std::cout << this->operator[](i) << ' ';
            }
            std::cout << std::endl;
        }

        void resize(unsigned size_in) {
            if (size_in == _size)
                return;

            if (size_in == 0) {
                _size = 0;
                _back = _front;
                return;
            }

            if (size_in <= _capacity) {
                _size = size_in;
                _back = (_front + _size - 1) % _capacity;
                return;
            }

            unsigned new_capacity = _capacity;
            while (new_capacity < size_in)
                new_capacity *= 2;
            
            reserve(new_capacity);
            _size = size_in;
        }

        void clear() {
            resize(0);
        }

};

#endif