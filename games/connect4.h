#ifndef CONNECT4_H
#define CONNECT4_H
#pragma once

#include "common.h"
#include "board.h"

namespace connect4 {

#pragma pack(1)
struct Move {
        uint8_t x;

        Move() : x(0) {}

        string str() const {
                stringstream s;
                s << (int)x;
                return s.str();
        }

        bool operator == (const Move&r) const {
                return x == r.x;
        }

        void debugMove(Color c) const {
                LOG("move(" << ColorStr(c) << ") = " << x);
        }
};

#pragma pack(1)
template <size_t MAX_MOVES>
struct MoveList {
        Move move[MAX_MOVES];
        size_t size() { return count; }
        uint8_t count;

        MoveList() { clear(); }
        void clear() { memset(this, 0, sizeof(MoveList)); }
        Move& operator[](size_t i) { return move[i]; }

        void add(uint8_t x) {
                assert(count < MAX_MOVES);
                move[count++].x = x;
        }
};


#pragma pack(1)
template <size_t MAX_X, size_t MAX_Y>
struct State {
        typedef board::Rectangle<Color,MAX_X,MAX_Y> Board;
        typedef MoveList<MAX_X> ML;
        typedef Move M;

        uint8_t height[MAX_X];
        Board color;
        Color _winner, _just_played;
        bool _game_over;

        Color winner() const { return _winner; }
        Color just_played() const { return _just_played; }
        Color current() const { return other(_just_played); }

        typedef MCMoveCounter<State, MAX_X> MoveCounter;

        size_t get_index(const M &m) { return m.x; }
        bool valid_index(size_t i) { return height[i] < (MAX_Y-1); }
        void set_index(M &m, size_t i) { m.x = i; }

        float operator[] (size_t i) {
                uint8_t x = (i/2) % MAX_X,
                        y = (i/2) / MAX_X;

                if ((i%2)==0 && color.get(x,y) == BLACK) return  1.0;
                if ((i%2)==0 && color.get(x,y) == WHITE) return 1.0;
                return 0.0;
        }

        bool game_over() { return _game_over; }
        void set_game_over() { _game_over  = true; }

        State() { clear(); }

        State(const State &rhs) { copy_from(rhs); }

        void copy_from(const State &rhs) {
                memcpy(this, &rhs, sizeof(State));
        }

        void clear() {
                memset(this, 0, sizeof(State));
                _just_played = WHITE;
                _winner = NONE;
        }

        void place(Color c, uint8_t x) {
                uint8_t y = height[x];
                assert(color.get(x,y) == NONE);
                color.set(x,y,c);
                height[x] += 1;
        }

        float result(Color c) {
                if (_winner == c) return 1.0;
                else if (_winner == other(c)) return 0;
                return 0.5;
        }

        int score(bool maximise) {
                return 0;
        }

        string move_str(const Move &move) {
                stringstream s;
                s << "move(" << ColorStr(other(_just_played)) << ") = " << (int) move.x;
                return s.str();
        }

        void announce(const Move &move) {
             //   LOG(move_str(move));
        }

        enum Direction { N, S, E, W, NE, NW, SE, SW };

        bool valid(Direction d, uint8_t x, uint8_t y) const {
                switch (d) {
                case N:  return y > 0;
                case S:  return y < (MAX_Y-1);
                case E:  return x < (MAX_X-1);
                case W:  return x > 0;
                case NW: return valid(N,x,y) && valid(W,x,y);
                case SE: return valid(S,x,y) && valid(E,x,y);
                case NE: return valid(N,x,y) && valid(E,x,y);
                case SW: return valid(S,x,y) && valid(W,x,y);
                }
                DIE("unknown direction: " << d);
        }

        void move_dir(Direction d, uint8_t &x, uint8_t &y) const {
                switch (d) {
                case N: --y; break;       case S: ++y; break;
                case E: ++x; break;       case W: --x; break;
                case NW: --y, --x; break; case SE: ++y, ++x; break;
                case NE: --y, ++x; break; case SW: ++y, --x; break;
                default: DIE("unknown direction: " << d);
                }
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

        size_t scan(Color c, Direction d, uint8_t x, uint8_t y) const {
                if (color.get(x,y) != c)
                        return 0;

                if (valid(d, x, y)) {
                        move_dir(d, x, y);
                        return 1+scan(c, d, x, y);
                }

                return 1;
        }

        size_t line_length(Color c, Direction d, uint8_t x, uint8_t y) const {
                size_t count = 0;
                count += scan(c, d, x, y);
                count += scan(c, reverse(d), x, y);
                return count > 0 ? count-1 : 0;
        }

        bool connect4(uint8_t x) const {
                uint8_t y = height[x]-1;
                Color c = color.get(x,y);
                return (line_length(c, N,  x, y) >= 4)
                    || (line_length(c, E,  x, y) >= 4)
                    || (line_length(c, NW, x, y) >= 4)
                    || (line_length(c, NE, x, y) >= 4);
        }


        void move(const Move &m) {
                Color player = current();

                place(player, m.x);

                if (connect4(m.x)) {
                        _winner = player;
                        _game_over = true;
                }

                _just_played = player;
        }

        bool random_move(ML &ml, M &m) {
                ml.clear();
                moves(ml);
                if (!ml.size())
                        return false;
                m.x = ml.move[random() % ml.size()].x;
                return true;
        }

        void moves(ML &ml) {
                if (_game_over)
                        return;

                for (uint8_t x=0; x < MAX_X; ++x)
                        if (height[x] == 0 || ((height[x]-1U) < (MAX_Y-1U)))
                                ml.add(x);

                if (ml.size() == 0)
                        _game_over = true;
        }

        string str() {
                stringstream s;
                for (uint8_t x=0; x < MAX_X; ++x)
                        s << (int) (x+1) << ' ';
                s << endl;
                for (uint8_t y=0; y < MAX_Y; ++y) {
                        for (uint8_t x=0; x < MAX_X; ++x) {
                                char c = '.';
                                if (color.get(x,MAX_Y-y-1) == BLACK) c = 'X';
                                else if (color.get(x,MAX_Y-y-1) == WHITE) c = 'O';
                                s << c << ' ';
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

        bool parse_move(M &m, const vector<string> &tokens) const {
                if (tokens.size() != 1) {
                        cout << "missing move" << endl;
                        return false;
                }

                m.x = atoi(tokens[0].c_str()) - 1;

                return true;
        }
};

} // namespace connect4

#endif // CONNECT4_H
