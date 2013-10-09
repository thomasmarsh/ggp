#ifndef GAME_H
#define GAME_H
#pragma once

#include "common.h"

template <typename S, typename B, typename W>
struct Game {
        B black;
        W white;
        S state;

        Game() {}

        void set_param(Color c, float p) {
                switch (c) {
                case BLACK: black.set_param(p); break;
                case WHITE: white.set_param(p); break;
                case NONE:
                default: assert(1==0);
                }
        }

        Color play(bool verbose=true) {
                state.clear();
                if (verbose) state.print();
                Color player = BLACK;
                while (!state.game_over()) {
                        switch (player) {
                        case BLACK: black.next(BLACK, state); break;
                        case WHITE: white.next(WHITE, state); break;
                        case NONE: assert(player != NONE);
                        }
                        if (verbose)
                                state.print();
                        player = other(player);
                }
                if (verbose)
                        cout << "winner = " << ColorStr(state.winner()) << endl;
                return state.winner();
        }
};

#endif // GAME_H
