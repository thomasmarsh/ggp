#ifndef CONTEST_H
#define CONTEST_H
#pragma once

#include "common.h"

static const size_t NUM_TRIALS = 3;

static const float MIN_CP = 0.001,
                   MAX_CP = 3.0;

template <typename G>
struct Contest {
        float best;
        float min, max;

        Contest() : best(sqrt(2)), min(MIN_CP), max(MAX_CP) {}

        float next(float a, float b) {
                float r = (float) random() / (float) 2147483647;
                if (a < b) std::swap(a,b);
                return (a-b)*r + b;
        }

        Color play(Color champ, float cpB) {
                G game;
                if (champ == BLACK) {
                        game.black.set_param(best);
                        game.white.set_param(cpB);
                } else {
                        game.black.set_param(cpB);
                        game.white.set_param(best);
                }
                Color winner = game.play(false);
                if (winner == champ) {
                        LOG(ColorStr(winner) << ": champion wins");
                } else if (winner == other(champ)) {
                        LOG(ColorStr(winner) << ": challenger wins");
                } else LOG("tie, champion playing " << ColorStr(champ));
                return winner;
        }

        bool too_close(float a, float b) {
                return ((a-b)*(a-b)) < .1;
        }

        void run_one(float test) {
                if (sqrt(test*test-best*best) < 2) {
                        LOG("difference too small.. bailout");
                        return;
                }
                LOG("CHALLENGE: champ=" << best << " challenger=" << test);
                size_t curw=0, testw=0;
                LOG("-- Round 1 -- (champion defends)");
                for (size_t i=0; i < 2; ++i) {
                        Color winner = play(WHITE, test);
                        switch (winner) {
                        case WHITE: ++curw; break;
                        case BLACK: ++testw; break;
                        case NONE: ++curw, ++testw; break;
                        }
                        if (testw > 0) break;
                }
                if (testw > 0) {
                        curw=0, testw=0;
                        LOG("-- Round 2 -- (challenger defends)");
                        for (size_t i=0; i < NUM_TRIALS; ++i) {
                                Color winner = play(BLACK, test);
                                switch (winner) {
                                case BLACK: ++curw; break;
                                case WHITE: ++testw; break;
                                case NONE: ++curw, ++testw; break;
                                }
                                if (curw == 2 && testw == 0)
                                        break;
                        }
                }
                settle_up(testw, curw, test);
        }

        void settle_up(size_t testw, size_t curw, float test) {
                LOG("score=" << curw << ':' << testw);
                if (testw == 0) {
                        if (test < best) min = test;
                        else if (test > best) max = test;
                } else {
                        if (testw > curw) {
                                best = test;
                        }
                }

                if (too_close(min, best) && too_close(max,best)) {
                        LOG("done");
                        exit(0);
                }

                LOG("best=" << best << " min=" << min << " max=" << max);
        }

        void run() {
                while (true)
                        run_one(next(max, min));
        }

};

#endif // CONTEST_H
