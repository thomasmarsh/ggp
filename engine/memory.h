#ifndef MEMORY_H
#define MEMORY_H
#pragma once

#include "common.h"

template <typename T>
struct MemoryPool {
        size_t counter;
        size_t size;
        T *data;

        MemoryPool(size_t s) : counter(0), size(s), data(0) {}

        void prealloc() {
                data = new T[size];
        }

        ~MemoryPool() {
                if (data)
                        delete[] data;
        }

        T *alloc() {
                if (!data) prealloc();
                assert(counter < size);
                return &data[counter++];
        }

        void clear() { counter = 0; }
};

#endif // MEMORY_H
