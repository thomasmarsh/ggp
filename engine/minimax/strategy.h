#ifndef STRATEGY_H
#define STRATEGY_H

#include "thread.h"

template <typename S, typename A, size_t D>
struct BasicMinimax {
        typedef typename S::ML MoveList;

        bool parallel;
        MoveList global_ml;

        BasicMinimax() : parallel(true)
        {}

        struct Result {
                size_t index;
                int score;
        };

        struct Task {
                BasicMinimax *parent;
                size_t index;
                S child;
                bool maximise;

                void operator() (Result &result) {
                        A algo;
                        int score = algo.search(
                                        child,
                                        D,
                                        !maximise);

                        if (!maximise)
                                score = -score;

                        result.index = index;
                        result.score = score;
                }
        };

        void next(Color c, S &state) {

                bool maximise = (c == BLACK);

                int score = maximise ? numeric_limits<int>::min()
                                     : numeric_limits<int>::max();

                global_ml.clear();
                state.moves(global_ml);

                int best = parallel
                        ? async_search(state, maximise, score)
                        : sync_search(state, maximise, score);

                if (global_ml.size() > 0) {
                        if (best == -1)
                                best = random() % global_ml.size();

                        state.announce(global_ml[best]);
                        state.move(global_ml[best]);
                }
        }

        int async_search(S &state, bool maximise, int score) {
                TaskPool<Task,Result> tasks(NUM_THREADS);

                for (size_t i=0; i < global_ml.size(); ++i) {
                        Task task;

                        task.parent = this;
                        task.index = i;
                        task.maximise = maximise;
                        make_child(state, task.child, i);

                        tasks.push(task);
                }

                tasks.run();

                int best = -1;

                for (size_t i=0; i < global_ml.size(); ++i) {
                        int s = tasks[i].score;
                        if ((maximise && s > score) || (!maximise && s < score))
                                score = s, best = tasks[i].index;
                }

                return best;
        }


        int sync_search(S &state, bool maximise, int score) {
                A algo;

                int best = -1;

                for (size_t i=0; i < global_ml.size(); ++i) {
                        S child;
                        make_child(state, child, i);
                        int s = algo.search(child,
                                            D,
                                            maximise);
                        //LOG("child=" << s << " move: " << s.move_str(global_ml.move[i]));
                        if ((maximise && s > score) || (!maximise && s < score))
                                score = s, best = i;
                }

                return best;
        }

        void make_child(const S &state, S &child, size_t i) {
                child.copy_from(state);
                child.move(global_ml[i]);
        }

        void set_param(float p) {}
};


#endif // STRATEGY_H
