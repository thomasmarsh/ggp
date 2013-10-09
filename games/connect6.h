#ifndef CONNECT6_H
#define CONNECT6_H
#pragma once

#include <engine/board.h>
#include <engine/memory.h>
#include <engine/state.h>
#include <engine/montecarlo.h>
#include <ui/board.h>

#include <bitset>


//----------------------------------------------------------------------------
//
// Calculate number of moves at compile time
//
//----------------------------------------------------------------------------

template <size_t N, size_t K>
struct binomial_coeff {
        enum { result = (binomial_coeff<N-1,K-1>::result * N) / K };
};

template <size_t N>
struct binomial_coeff <N, 0> {
        enum { result = 1 };
};


namespace connect6 {

#pragma pack(1)
template <size_t SIZE>
struct Move {
        uint16_t a:9, b:9;

        string str() const {
                stringstream s;
                s << "{ (" << (int) (a%SIZE) << ", " << (int) (a/SIZE) << ") ("
                          << (int) (b%SIZE) << ", " << (int) (b/SIZE) << ") }";
                return s.str();
        }

        bool operator == (Move&r) const {
                return a == r.a && b == r.b;
        }

        void debugMove(Color c) const {
                LOG("move(" << ColorStr(c) << ") = " << str());
        }

        void set(uint16_t na, uint16_t nb) {
                a=na, b = nb;
        }
};

#pragma pack(1)
template <typename S, size_t SIZE>
struct MoveList {
        bitset<SIZE*SIZE> occupied;
        uint16_t count, two_moves:1;
        Move<SIZE> m;

        size_t size() const { return count; }

        MoveList() { clear(); }

        void clear() {
                occupied.reset();
                count=0;
                two_moves=false;
                m.a=0, m.b=0;
        }

        size_t bin_coeff(size_t n, size_t k) {
                size_t r=1;
                assert(n >= k);
                for (size_t d=1; d <= k; d++) {
                        r *= n--;
                        r /= d;
                }
                return r;
        }

        void set(S &s) {
                occupied = s.black | s.white;
                two_moves = s.two_moves;

                size_t empty = SIZE*SIZE - occupied.count();
                if (two_moves)
                        count = bin_coeff(empty,2);
                else    count = empty;
                if (count == 1) {
                        assert(empty == 1);
                        two_moves = false;
                }
        }

        void fill_move(size_t k) {
                uint16_t n=0;

                if (!two_moves) {
                        for (uint16_t i=0; i < (SIZE*SIZE); ++i) {
                                if (!occupied[i]) {
                                        if (n == k) {
                                                m.a = i;
                                                m.b = 0;
                                                return;
                                        }
                                        ++n;
                                }
                        }
                        DIE("not reached");
                }

                for (uint16_t i=0; i < (SIZE*SIZE); ++i) {
                        for (uint16_t j=(i+1); j < ((SIZE*SIZE)); ++j) {
                                if (!(occupied[i] || occupied[j])) {
                                        if (n == k)  {
                                                m.a = i;
                                                m.b = j;
                                                return;
                                        }
                                        ++n;
                                }
                        }
                }
                DIE("not reached");
        }

        Move<SIZE>& operator[](size_t i) {
                fill_move(i);
                return m;
        }
};

static GBoard WINDOW(600, 600, 19);

#pragma pack(1)
template <size_t SIZE, size_t MAX_MOVES>
struct State : BaseState {
        typedef board::Square<Color,SIZE> Board;
        typedef MoveList<State, SIZE> ML;
        typedef Move<SIZE> M;

        enum Direction { N, S, E, W, NE, NW, SE, SW };

        bool two_moves:1;
        bitset<SIZE*SIZE> black;
        bitset<SIZE*SIZE> white;

        typedef MCMoveCounter<State, MAX_MOVES> MoveCounter;

        void index_to_moves(size_t k, uint16_t &i, uint16_t &j) {
                size_t n=0;
                for (i=0; i < (SIZE*SIZE); ++i) {
                        for (j=(i+1); j < ((SIZE*SIZE)); ++j) {
                                if (n == k)
                                        return;
                                ++n;
                        }
                }
                DIE("not reached");
        }

        size_t get_index(const M &m) {
                size_t n=0;
                for (uint16_t i=0; i < (SIZE*SIZE); ++i) {
                        for (uint16_t j=(i+1); j < ((SIZE*SIZE)); ++j) {
                                if (i == m.a && j == m.b)
                                        return n;
                                ++n;
                        }
                }
                DIE("not reached");
        }

        bool valid_index(size_t k) {
                uint16_t i, j;
                index_to_moves(k, i, j);
                return !occupied(i) && !occupied(j);
        }

        void set_index(M &m, size_t k) {
                uint16_t i, j;
                index_to_moves(k, i, j);
                m.a = i;
                m.b = j;
        }



        float operator[](size_t i) const {
                if (black[i]) return 1.0;
                if (white[i]) return -1.0;
                return 0.0;
        }

        bool occupied(uint16_t i) const {
                return black[i] || white[i];
        }

        Color color(uint16_t i) const {
                if (black[i]) return BLACK;
                if (white[i]) return WHITE;
                return NONE;
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

        void place(Color c, uint16_t i) {
                switch (c) {
                case BLACK: black.set(i); break;
                case WHITE: white.set(i); break;
                default: DIE("bad color provided: " << c);
                }
        }

        int score(bool maximise) {
                //cb += maximise ? 300 : 0;
                //cw -= maximise ? 0 : 300;
                return 0;
        }

        bool valid(Direction d, uint16_t i) const {
                if (i >= (SIZE*SIZE))
                        return false;
                uint8_t x = i%SIZE, y = i/SIZE;
                switch (d) {
                case N:  return y > 0;
                case S:  return y < (SIZE-1);
                case E:  return x < (SIZE-1);
                case W:  return x > 0;
                case NW: return valid(N,i) && valid(W,i);
                case SE: return valid(S,i) && valid(E,i);
                case NE: return valid(N,i) && valid(E,i);
                case SW: return valid(S,i) && valid(W,i);
                }
                DIE("unknown direction: " << d);
        }

        void move_dir(Direction d, uint16_t &i) const {
                uint8_t x = i%SIZE, y = i/SIZE;
                switch (d) {
                case N: --y; break;       case S: ++y; break;
                case E: ++x; break;       case W: --x; break;
                case NW: --y, --x; break; case SE: ++y, ++x; break;
                case NE: --y, ++x; break; case SW: ++y, --x; break;
                default: DIE("unknown direction: " << d);
                }
                i = x + y*SIZE;
        }

        Direction reverse(Direction d) const {
                switch (d) {
                case N:  return S;  case S:  return N;
                case E:  return W;  case W:  return E;
                case NW: return SE; case SE: return NW;
                case NE: return SW; case SW: return NE;
                }
                DIE("unknown direction: " << d);
        }

        size_t scan(Color c, Direction d, uint16_t i) const {
                if (color(i) != c)
                        return 0;

                if (valid(d, i)) {
                        move_dir(d, i);
                        return 1+scan(c, d, i);
                }

                return 1;
        }

        size_t line_length(Color c, Direction d, uint16_t i) const {
                size_t count = 0;
                count += scan(c, d, i);
                count += scan(c, reverse(d), i);
                return count > 0 ? count-1 : 0;
        }

        bool found6(uint16_t i) const {
                Color c = color(i);
                return (line_length(c, N,  i) >= 6)
                    || (line_length(c, E,  i) >= 6)
                    || (line_length(c, NW, i) >= 6)
                    || (line_length(c, NE, i) >= 6);
        }


        string move_str(M &m) const {
                stringstream s;
                s << "move(" << ColorStr(current()) << ") = " << m.str();
                return s.str();
        }

        void announce(const M &move) {
                move.debugMove(current());
        }

        void move(const M &m) {
                Color player = current();

                place(player, m.a);
                if (two_moves) {
                        place(player, m.b);

                        if (found6(m.a) || found6(m.b)) {
                                _winner = player;
                                set_game_over();
                        }
                }
                two_moves = true;

                end_move();
        }

        bool random_walk(uint16_t &i, bool skip, uint16_t s) {
                i = random() % Board::AREA;

                for (size_t j=0; j < SIZE; ++j) {
                        if (color(i) == NONE)
                                if ((skip && i != s) || !skip)
                                        return true;
                        i = random() % Board::AREA;
                }

                uint16_t wrap = i;

                while (true) {
                        if (color(i) == NONE) {
                                if (!skip || (skip && i != s))
                                        return true;
                        }

                        i = (i+1) % Board::AREA;
                        if (i == wrap)
                                break;
                }
                return false;
        }

        bool random_move(ML &ml, M &m) {
                uint16_t i=0;
                if (!random_walk(i, false, 0))
                        return false;
                m.a = i;

                if (!random_walk(i, true, m.a))
                        return false;
                m.b = i;

                if (m.a > m.b) {
                        uint16_t tmp = m.a;
                        m.a = m.b;
                        m.b = tmp;
                }
                assert(m.a != m.b);

                return true;
        }

        void moves(ML &ml) {
                if (game_over())
                        return;

                ml.set(*this);

                if (ml.count == 0)
                        set_game_over();
        }

        string str() {
                stringstream s;
                for (uint16_t i=0; i < Board::AREA; ++i) {
                        char c = '.';
                        if (color(i) == BLACK) c = 'X';
                        else if (color(i) == WHITE) c = 'O';
                        s << c << ' ';
                        if (((i+1) % SIZE) == 0)
                                s << endl;
                }
                return s.str();
        }

        void display() {
                GBoard::player_t b, w;
                for (uint16_t i=0; i < Board::AREA; ++i) {
                        if (color(i) == BLACK) b.push_back(i);
                        else if (color(i) == WHITE) w.push_back(i);
                }
                WINDOW.update(GBoard::GO, b, w);
        }

        void print() {
                display(); return;
                cout << str();
                cout << flush;
        }
};

} // namespace connect6

#endif // CONNECT6_H
