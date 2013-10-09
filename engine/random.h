#ifndef DRUID_RANDOM_H
#define DRUID_RANDOM_H
#pragma once

#include "common.h"

template <typename S>
struct Random {
        typename S::ML ml;

        void next(Color c, S &state) {
                ml.clear();
                state.moves(ml);
                if (ml.size()) {
                        size_t choice = random() % ml.size();
                        state.announce(ml[choice]);
                        state.move(ml[choice]);
                }
        }

        void set_param(float p) {}
};

#endif // DRUID_RANDOM_H
