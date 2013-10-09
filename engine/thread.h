#ifndef THREAD_H
#define THREAD_H

#include "common.h"

#ifdef __APPLE__
#include <libkern/OSAtomic.h>
#endif

#include <queue>


struct Mutex {
        volatile int exclusion;
        Mutex() : exclusion(0) {}
        void lock() { while(__sync_lock_test_and_set(&exclusion, 1)); }
        void unlock() { __sync_lock_release(&exclusion); }
};


struct Lock {
        Mutex &mutex;
        Lock(Mutex &m) : mutex(m) { mutex.lock(); }
        ~Lock() { mutex.unlock(); }
};

static Mutex LOG_MUTEX;

#define SLOG(x) { Lock _l(LOG_MUTEX); LOG(x); }

template <typename A, typename B=int>
struct TaskPool {
        size_t num_threads;
        queue<A> tasks;
        vector<B> results;
        Mutex task_m, result_m;

        TaskPool(size_t num) : num_threads(num) {}

        void clear() {
                tasks.erase(tasks.begin(), tasks.end());
                results.erase(results.begin(), results.end());
        }

        void push(const A &a) {
                tasks.push(a);
        }

        void work() {
                while (true) {
                        A arg;
                        { Lock lock(task_m);
                                if (tasks.size() == 0)
                                        return;

                                arg = tasks.front();
                                tasks.pop();
                        }

                        B r;
                        arg(r);

                        { Lock lock(result_m);
                                results.push_back(r);
                        }
                }
        }

        B& operator [] (size_t index) {
                return results[index];
        }

        static void *spawn_thread(void *self) {
                ((TaskPool *) self)->work();
                return NULL;
        }

        void run() {
                pthread_t thread[num_threads];
                for (size_t i=0; i < num_threads; ++i)
                        pthread_create(&thread[i], NULL, spawn_thread, (void*) this);
                for (size_t i=0; i < num_threads; ++i)
                        pthread_join(thread[i], NULL);
        }
};

#endif // THREAD_H
