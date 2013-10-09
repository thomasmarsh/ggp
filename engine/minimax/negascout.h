#ifndef NEGASCOUT_H
#define NEGASCOUT_H
#pragma once

#include "common.h"
#include "sort.h"
#include "thread.h"

#include <queue>


template <typename S>
struct Negascout {
        typedef typename S::ML MoveList;

        void make_child(const S &state, S &child, MoveList &ml, size_t i) {
                child.copy_from(state);
                child.move(ml[i]);
        }

        int search(S &state, int depth, bool maximise) {
                return -pvs(state,
                            numeric_limits<int>::min(),
                            numeric_limits<int>::max(),
                            depth,
                            maximise);
        }

        int pvs(S &state, int alpha, int beta, int depth, bool maximise) {
                if (state.game_over() || (depth == 0))
                        return maximise ? state.score(maximise) : -state.score(maximise);

                MoveList ml;
                state.moves(ml);

                // build sort list
                S children[ml.size()];
                IndexSort<int> sorter(ml.size());
                for (size_t i=0; i < ml.size(); ++i) {
                        make_child(state, children[i], ml, i);
                        sorter[i].value = children[i].score(!maximise);
                }

                // sort
                if (maximise)
                        sorter.rsort();
                else sorter.sort();

                // recurse
                int b = beta;
                for (size_t i=0; i < ml.size(); ++i) {
                        S &child = children[sorter[i].index];

                        int result = -pvs(child, -b, -alpha, depth-1, !maximise);

                        // check if null window failed high
                        if (i > 0 && alpha < result && result < beta)
                                result = -pvs(child, -beta, -alpha, depth-1, !maximise);

                        alpha = std::max(alpha, result);

                        // beta cutoff
                        if (alpha >= beta)
                                return alpha;

                        // set new null window
                        b = alpha+1;
                }
                return alpha;
        }
};

#endif // NEGASCOUT_H
