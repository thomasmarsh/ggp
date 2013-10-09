#ifndef GGP_STATE_H
#define GGP_STATE_H

#include "common.h"

#pragma pack(1)
struct BaseState {
        Color _winner:2, _just_played:2;
        bool _game_over:1;

        void reset() {
                _winner = NONE;
                _just_played = WHITE;
        }

        Color winner() const { return _winner; }
        Color just_played() const { return _just_played; }
        Color current() const { return other(_just_played); }
        void set_game_over() { _game_over = true; }
        bool game_over() const { return _game_over; }

        void end_move() { _just_played = current(); }

        float result(Color c) const {
                if (_winner == c) return 1.0;
                else if (_winner == other(c)) return 0;
                return 0.5;
        }

};


#endif // GGP_STATE_H
