#ifndef MONTECARLO_H
#define MONTECARLO_H
#pragma once

#include "common.h"
#include "sort.h"
#include "thread.h"

#include <queue>

template <typename S, size_t MAX_MOVES>
struct MCMoveCounter {
        typedef typename S::M M;
        size_t count[MAX_MOVES];
        size_t sum;

        MCMoveCounter() { reset(); }

        void reset() { memset(this, 0, sizeof(MCMoveCounter)); }
        void add(S &s, const M &m) { ++count[s.get_index(m)], ++sum; }
        void push(const MCMoveCounter &mc) {
                for (size_t i=0; i < MAX_MOVES; ++i)
                        count[i] += mc.count[i];
                sum += mc.sum;
        }

        bool best(S &s, M &m) {
                size_t best=0, result=0;
                bool found = false;
                for (size_t i=0; i < MAX_MOVES; ++i) {
                        if (count[i] > best && s.valid_index(i)) {
                                best = count[i];
                                result = i;
                                found = true;
                        }       
                }       
                if (found) {
                        vector<size_t> top;
                        for (size_t i=0; i < MAX_MOVES; ++i)
                                if (count[i] == best)
                                        top.push_back(i);
                        result = top[random() % top.size()];
                        s.set_index(m, result);
                }

                if (sum > 0) {
                        M tmp;
                        for (size_t i=0; i < MAX_MOVES; ++i) {
                                s.set_index(tmp, i);
                                if (count[i] > 0)
                                        LOG("i=" << i << " score=" << ((float) count[i] / (float) sum) << " move=" << tmp.str());
                        }
                }
                return found;
        }
};

template <typename S, size_t MAX_MOVES, size_t N>
struct MonteCarlo {

        typedef typename S::ML ML;
        typedef typename S::M M;
        typedef typename S::MoveCounter C;

        C wins, win_first;
        float result[MAX_MOVES];
        Mutex mutex;

        void play(Color c, S &s) {
                M r;
                ML dummy;
                C tmp;
                tmp.reset();
                bool first = true;
                size_t index=0;
                while (!s.game_over()) {
                        if (!s.random_move(dummy, r))
                                break;
                        if (first) {
                                { Lock lock(mutex);
                                win_first.add(s, r); }
                                index = s.get_index(r);
                                first = false;
                        }
                        tmp.add(s, r);
                        s.move(r);
                }

                Lock lock(mutex);
                if (s.winner() == c)
                        wins.push(tmp);
                result[index] += s.result(c);
        }

        struct Task {
                MonteCarlo *mc;
                Color c;
                S *orig;
                size_t iter;

                void operator() (int dummy) {
                        S s;
                        s.copy_from(*orig);
                        for (size_t i=0; i < iter; ++i)
                                mc->play(c, s);
                }
        };

        void next(Color c, S &state) {
                ML ml;
                ml.clear();
                wins.reset();
                win_first.reset();

                state.moves(ml);
                if (!ml.size())
                        return;

                TaskPool<Task> tasks(NUM_THREADS);
                Task task[NUM_THREADS];
                for (size_t i=0; i < NUM_THREADS; ++i) {
                        task[i].mc = this;
                        task[i].c = c;
                        task[i].orig = &state;
                        task[i].iter = N / NUM_THREADS;
                        tasks.push(task[i]);
                }

                tasks.run();

                M best;
                //LOG("win total");
                //wins.best(state, best);
                //LOG("win first");
                bool found = win_first.best(state, best);
                if (found) {
                        state.announce(best);
                        state.move(best);
                }
        }

        void set_param(float f) {}
};

#endif // MONTECARLO_H
