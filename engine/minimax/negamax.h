#ifndef NEGAMAX_H
#define NEGAMAX_H
#pragma once

#include "common.h"

template <typename S>
struct Negamax {
        typedef typename S::ML MoveList;

        void make_child(const S &state, S &child, const MoveList &ml, size_t i) {
                child.copy_from(state);
                child.move(ml.move[i]);
        }

        int search(S &state, int depth, bool maximise) {
                if (state.game_over() || (depth == 0))
                        return maximise ? state.score(maximise) : -state.score(maximise);

                MoveList ml;
                state.moves(ml);
                S child;

                int alpha = numeric_limits<int>::min();
                for (size_t i=0; i < ml.size(); ++i) {
                        make_child(state, child, ml, i);
                        int result = -search(child, depth-1, !maximise);
                        alpha = std::max(alpha, result);
                }
                return alpha;
        }
};

#endif // NEGAMAX_H
