#ifndef GGP_GENETIC_H
#define GGP_GENETIC_H

#include "common.h"
#include "thread.h"
#include "../ui/plot.h"

#include <vector>
#include <algorithm>


template <typename T,
          typename Policy>
struct Pool {

        size_t size;
        vector<T> pop;
        size_t id;
        Mutex mutex;

        Pool(size_t s, size_t i) : size(s), pop(size), id(i) {
                clear();
        }

        Pool(const Pool &p) {
                //clear();
                size = p.size;
                pop = p.pop;

                id = p.id;

                //assert(pop.size() == size);
        }

        T choice() { return pop[random() % pop.size()]; }

        void clear() {
                for (size_t i=0; i < size; ++i) {
                        Policy::release(pop[i]);
                        pop[i] = Policy::null();
                }
        }

        void randomize() {
                clear();
                for (size_t i=0; i < size; ++i)
                        Policy::randomize(pop[i], id);
        }

        struct Sorter {
                size_t pid;
                Sorter(size_t pool) : pid(pool) {}
                bool operator () (const T i, const T j) const {
                        return Policy::compare(i, j, pid);
                }
        };

        void sort() { std::sort(pop.begin(), pop.end(), Sorter(id)); }
};

template <typename T,
          typename Policy>
struct Population {
        std::vector< Pool<T,Policy> > pools;
        std::vector< T > best;
        size_t npools, psize;
        //Plot plot;


        Population(size_t pool_size, size_t num_pools)
                : npools(num_pools),
                  psize(pool_size)
                  //, plot(400, 400, 10, -10, 10, -10)
        {
                for (size_t i=0; i < num_pools; ++i) {
                        pools.push_back(Pool<T,Policy>(pool_size, i));
                        pools[i].randomize();
                        pools[i].sort();
                        best.push_back(Policy::null());
                }
        }

        struct Task {
                Population *pop;
                size_t pool_id;

                Task() : pop(0), pool_id(0) {}
                Task(Population *p, size_t i) : pop(p), pool_id(i) {}

                void operator() (int &dummy) {
                        Pool<T,Policy> tmp(pop->psize, pool_id);
                        size_t i=0,
                               next_id = (pool_id+1)%(pop->npools),
                               last_id = pop->pools[pool_id].pop.size()-1;

                        T null_entry = Policy::null();
                        while (true) {
                                Policy::iterate(pop->pools[pool_id], tmp, pool_id);
                                i++;
                                { Lock lock(pop->pools[pool_id].mutex);
                                        // have a migrant
                                        if (pop->best[pool_id] == null_entry) {
                                                //LOG("import migrant(" << pool_id << "):" << Policy::error(pop->best[pool_id], pool_id));
                                                // sort
                                                pop->pools[pool_id].sort();
                                                // delete worst
                                                Policy::release(pop->pools[pool_id].pop[last_id]);
                                                pop->pools[pool_id].pop[last_id] = Policy::null();
                                                // set worst as the migrant
                                                pop->pools[pool_id].pop[last_id] = pop->best[pool_id];
                                                // set the migrant origin to null
                                                pop->best[pool_id] = Policy::null();
                                        }
                                        // re-sort
                                        pop->pools[pool_id].sort();
                                        // if enough time passed
                                        if ((i + (pool_id * (800 / (pop->npools)))) % 800 == 0) {
                                                Lock lock(pop->pools[next_id].mutex);
                                                //SLOG("migrate " << pool_id << " -> " << next_id);
#ifdef DEEP_COPY
                                                pop->best[next_id] = pop->pools[pool_id].pop[0]->deep_copy();
#else
                                                pop->best[next_id] = pop->pools[pool_id].pop[0];
#endif
                                                Policy::set_pool(pop->best[next_id], next_id);
                                        }
                                }
                        }
                }
        };

        void iterate() {
                TaskPool<Task> tasks(npools);
                for (size_t i=0; i < npools; ++i) {
                        Task t(this, i);
                        tasks.push(t);
                }
                tasks.run();
        }
};


#endif // GGP_GENETIC_H
