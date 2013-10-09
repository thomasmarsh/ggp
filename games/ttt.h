#ifndef TTT_H
#define TTT_H
#pragma once

#include "common.h"
#include "board.h"
#include "state.h"

#include <bitset>

namespace ttt {

#pragma pack(1)
template <size_t SIZE>
struct Move {
        uint8_t i;

        uint8_t x() const { return i%SIZE; }
        uint8_t y() const { return i/SIZE; }

        void set(uint8_t index) {
                i = index;
        }


        string str() const {
                stringstream s;
                s << "{ " << (int) x() << " " << (int) y() << " }";
                return s.str();
        }

        bool operator == (Move&r) const {
                return i == r.i;
        }

        void debugMove(Color c) const {
                LOG("move(" << ColorStr(c) << ") = " << str());
        }
};

#pragma pack(1)
template <size_t SIZE, size_t MAX_MOVES>
struct MoveList {
        Move<SIZE> move[MAX_MOVES];
        uint8_t count;

        size_t size() { return count; }
        MoveList() { clear(); }
        void clear() { memset(this, 0, sizeof(MoveList)); }

        void add(size_t i) {
                assert(count < MAX_MOVES);
                move[count++].set(i);
        }

        Move<SIZE>& operator[](size_t i) { return move[i]; }
};


#pragma pack(1)
template <size_t SIZE>
struct State : BaseState {
        typedef board::Square<Color,SIZE> Board;
        typedef MoveList<SIZE, SIZE*SIZE> ML;
        typedef Move<SIZE> M;

        typedef MCMoveCounter<State,SIZE*SIZE> MoveCounter;
        size_t get_index(const M &m) { return m.i; }
        bool valid_index(size_t i) { return color(i) == NONE; }
        void set_index(M &m, size_t i) { m.i = i; }

        bitset<SIZE*SIZE> black;
        bitset<SIZE*SIZE> white;

        Color color(size_t i) {
                if (black[i]) return BLACK;
                if (white[i]) return WHITE;
                return NONE;
        }

        float operator[](size_t i) {
                if ((i % 2) == 0)
                        return color(i/2) == BLACK;
                return color(i/2) == WHITE;
        }

        State() { clear(); }

        State(const State &rhs) { copy_from(rhs); }

        void copy_from(const State &rhs) {
                memcpy(this, &rhs, sizeof(State));
        }

        void clear() {
                memset(this, 0, sizeof(State));
                reset();
        }

        void place(Color c, size_t i) {
                assert(color(i) == NONE);
                switch (c) {
                case BLACK: black.set(i); break;
                case WHITE: white.set(i); break;
                default: DIE("bad color: " << c);
                }
        }

        int ipow(int x, int p) {
                int i = 1;
                for (int j = 1; j < p; j++)  i *= x;
                return i;
        }

        int score(bool maximise) {
                int cb=0, cw=0;
                int diag_b=0, diag_w=0;
                size_t i=0;
                for (size_t x=0; x < SIZE; ++x) {
                        int col_b=0, col_w=0,
                            row_b=0, row_w=0;
                        for (size_t y=0; y < SIZE; ++y) {
                                i = x + y*SIZE;
                                col_b += color(i) == BLACK ? 1 : 0;
                                col_w += color(i) == WHITE ? 1 : 0;
                                i = y + x*SIZE;
                                row_b += color(i) == BLACK ? 1 : 0;
                                row_w += color(i) == WHITE ? 1 : 0;
                        }
                        cb += ipow(10, col_b+1)-1;
                        cw += ipow(10, col_w+1)-1;
                        cb += ipow(10, row_b+1)-1;
                        cw += ipow(10, row_w+1)-1;

                        i = x + x*SIZE;
                        diag_b += color(i) == BLACK ? 1 : 0;
                        diag_w += color(i) == WHITE ? 1 : 0;
                }
                cb += ipow(10, diag_b+1)-1;
                cw += ipow(10, diag_w+1)-1;

                diag_b=0, diag_w=0;
                for (size_t x=SIZE-1, y=0; y < SIZE; --x, ++y) {
                        i = x + y*SIZE;
                        diag_b += color(i) == BLACK ? 1 : 0;
                        diag_w += color(i) == WHITE ? 1 : 0;
                }
                cb += ipow(10, diag_b+1)-1;
                cw += ipow(10, diag_w+1)-1;

                cb += maximise ? 300 : 0;
                cw -= maximise ? 0 : 300;
                return cb - cw;
        }

        Color complete() {
                bool bail = false;
                size_t i=0;
                Color c;
                for (size_t x=0; x < SIZE; ++x) {
                        c = color(x);
                        if (c == NONE)
                                continue;
                        for (size_t y=1; y < SIZE; y++) {
                                i = x+y*SIZE;
                                if (color(i) != c) {
                                        bail = true;
                                        break;
                                }
                        }
                        if (!bail)
                                return c;
                        bail = false;
                }

                for (size_t y=0; y < SIZE; ++y) {
                        c = color(y*SIZE);
                        if (c == NONE)
                                continue;
                        for (size_t x=1; x < SIZE; x++) {
                                i = x+y*SIZE;
                                if (color(i) != c) {
                                        bail = true;
                                        break;
                                }
                        }
                        if (!bail)
                                return c;
                        bail = false;
                }

                c = color(0);
                for (size_t x=1, y=1; x < SIZE && y < SIZE; ++x, ++y) {
                        if (color(x+y*SIZE) != c) {
                                bail = true;
                                break;
                        }
                }
                if (!bail)
                        return c;
                bail = false;

                c = color(SIZE-1);
                for (size_t x=SIZE-2, y=1; y < SIZE; --x, ++y) {
                        if (color(x+y*SIZE) != c) {
                                bail = true;
                                break;
                        }
                }
                if (!bail)
                        return c;

                return NONE;
        }

        string move_str(const M &m) const {
                stringstream s;
                s << "move(" << ColorStr(other(_just_played)) << ") = " << m.str();
                return s.str();
        }

        void announce(const M &move) const {
                LOG(move_str(move));
        }

        void move(const M &m) {
                Color player = current();

                place(player, m.i);

                _winner = complete();
                if (_winner != NONE)
                        _game_over = true;

                _just_played = player;
        }

        void moves(Color c, ML &ml) {
                for (size_t i=0; i < SIZE*SIZE; ++i) {
                        if (color(i) == NONE)
                                ml.add(i);
                }
        }

        void moves(ML &ml) {
                if (_game_over)
                        return;

                Color player = other(_just_played);

                moves(player, ml);

                if (ml.size() == 0)
                        _game_over = true;
        }

        bool random_move(ML &ml, M &m) {
                ml.clear();
                moves(ml);
                if (!ml.size()) return false;
                m = ml.move[random() % ml.size()];
                return true;
        }

        string str() {
                stringstream s;
                for (uint8_t i=0; i < SIZE*SIZE; ++i) {
                        char c = '-';
                        if (color(i) == BLACK) c = 'X';
                        else if (color(i) == WHITE) c = 'O';
                        s << c;
                        if ((i+1) % SIZE == 0)
                                s << endl;
                }
                return s.str();
        }

        void print() {
                //cout << "-----------------------------------" << endl;
                cout << str();
                cout << flush;
        }

        bool parse_move(M &m, const vector<string> &tokens) {
                if (tokens.size() != 2) {
                        LOG("must provide two arguments");
                        return false;
                }
                uint8_t x, y;
                x = atoi(tokens[0].c_str())-1;
                y = atoi(tokens[1].c_str())-1;
                if (x >= SIZE || y >= SIZE)
                {
                        LOG("coordinates out of range");
                        return false;
                }
                m.i = x + y*SIZE;
                return true;
        }
};

} // namespace ttt

#endif // TTT_H
