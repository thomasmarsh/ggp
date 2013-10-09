#ifndef GONNECT_H
#define GONNECT_H
#pragma once

#include "common.h"

namespace gonnect {

#pragma pack(1)
struct Move {
        uint16_t data;

        uint8_t x() const { return (data >> 8) & 0xFF; }
        uint8_t y() const { return data & 0xFF; }

        void set(uint8_t _x, uint8_t _y) {
                data = 0;
                data |= _y & 0xFF;
                data |= (_x & 0xFF) << 8;
        }


        string str() const {
                stringstream s;
                s << "{ " << (int) x() << " " << (int) y() << " }";
                return s.str();
        }

        bool operator == (Move&r) const {
                return x() == r.x() &&
                       y() == r.y();
        }

        void debugMove(Color c) const {
                LOG("move(" << ColorStr(c) << ") = " << str());
        }
};

#pragma pack(1)
template <size_t MAX_MOVES>
struct MoveList {
        Move move[MAX_MOVES];
        uint8_t count;

        MoveList() { clear(); }
        void clear() { memset(this, 0, sizeof(MoveList)); }

        void add(uint8_t x, uint8_t y) {
                assert((count+2U) < MAX_MOVES);
                move[count++].set(x, y);
        }
};


#pragma pack(1)
template <size_t MAX_X,
          size_t MAX_Y,
          size_t MAX_MOVES>
struct State {
        typedef MoveList<MAX_MOVES> ML;
        typedef Move M;

        Color color[MAX_X][MAX_Y];
        Color winner, just_played;
        bool _game_over;

        bool game_over() { return _game_over; }
        void set_game_over() { _game_over  = true; }
        Color current() { return other(just_played); }

        State() { clear(); }

        State(const State &rhs) { copy_from(rhs); }

        void copy_from(const State &rhs) {
                memcpy(this, &rhs, sizeof(State));
        }

        void clear() {
                memset(this, 0, sizeof(State));
                just_played = WHITE;
                winner = NONE;
        }

        void remove_group(uint8_t x, uint8_t y) {
        }

        void remove_dead(uint8_t x, uint8_t y) {
                path_finder.clear(NONE, &color);

                for (uint8_t x=0; x < MAX_X; ++x) {
                        for (uint8_t y=0; y < MAX_Y; ++y) {
                                if (!path_finder.seen(x,y)) {
                                        if (path2.liberties(x, y, &color) == 0) {

                                        }
                                }
                        }
                }
        }

        void place(Color c, uint8_t x, uint8_t y) {
                //LOG("color[x][y] = " << ColorStr(color[x][y]));
                assert(color[x][y] == NONE);
                color[x][y] = c;
                
                remove_dead(x, y);
        }

        float result(Color c) {
                if (winner == c) return 1.0;
                else if (winner == other(c)) return 0;
                return 0.5;
        }

        int ipow(int x, int p) {
                int i = 1;
                for (int j = 1; j < p; j++)  i *= x;
                return i;
        }

        int score(bool maximise) {
        }

        void move(const Move &m) {
                Color player = current();
                //LOG("move(" << ColorStr(player) << ", " << m.str() << ')');

                place(player, m.x(), m.y());

                if (path_finder.connected(player, &color)) {
                        winner = player;
                        _game_over = true;
                }

                just_played = player;
        }

        void moves(Color c, ML &ml) {
                for (uint8_t x=0; x < MAX_X; ++x)
                        for (uint8_t y=0; y < MAX_Y; ++y)
                                if (color[x][y] == NONE)
                                        ml.add(x, y);
        }

        void moves(ML &ml) {
                if (_game_over)
                        return;

                Color player = other(just_played);

                moves(player, ml);

                if (ml.count == 0)
                        _game_over = true;
        }

        string str() {
                stringstream s;
                for (uint8_t y=0; y < MAX_Y; ++y) {
                        for (uint8_t x=0; x < MAX_X; ++x) {
                                char c = '-';
                                if (color[x][y] == BLACK) c = 'X';
                                else if (color[x][y] == WHITE) c = 'O';
                                s << c;
                        }
                        s << endl;
                }
                return s.str();
        }

        void print() {
                //cout << "-----------------------------------" << endl;
                cout << str();
                cout << flush;
        }
};

} // namespace gonnect

#endif // GONNECT_H
