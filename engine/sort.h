#ifndef SORT_H
#define SORT_H

#include "common.h"

#include <algorithm>

template <typename T>
struct IndexSort {
        struct Entry {
                size_t index;
                T value;
        };

        struct Sorter {
                Sorter(Entry*) {}
                bool operator () (const Entry &i, const Entry &j) {
                        return i.value < j.value;
                }
        };

        struct ReverseSorter {
                ReverseSorter(Entry*) {}
                bool operator () (const Entry &i, const Entry &j) {
                        return i.value > j.value;
                }
        };

        size_t size;
        Entry *data;

        IndexSort(size_t l) : size(l) {
                data = new Entry[size];
                for (size_t i=0; i < size; ++i)
                        data[i].index = i;
        }

        ~IndexSort() { if (data) delete[] data; }

        void sort() { std::sort(data, data+size, Sorter(data)); }
        void rsort() { std::sort(data, data+size, ReverseSorter(data)); }



        Entry& operator[] (size_t i) { return data[i]; }
};

#endif // SORT_H
