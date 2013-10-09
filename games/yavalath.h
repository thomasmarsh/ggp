#ifndef YAVALATH_H
#define YAVALATH_H
#pragma once

#include "common.h"
#include "board.h"

namespace yavalath {

#pragma pack(1)
struct Move {
        uint8_t _x:4, _y:4;

        uint8_t x() const { return _x; }
        uint8_t y() const { return _y; }

        void set(uint8_t x, uint8_t y) {
                _x = x,
                _y = y;
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
        size_t size() { return count; }
        uint8_t count;

        MoveList() { clear(); }
        void clear() { memset(this, 0, sizeof(MoveList)); }
        Move& operator[](size_t i) { return move[i]; }

        void add(uint8_t x, uint8_t y) {
                assert(count < MAX_MOVES);
                move[count++].set(x, y);
        }
};


#pragma pack(1)
template <size_t N, size_t SIZE>
struct State {
        typedef board::Hex<Color,SIZE> Board;
        typedef typename board::Hex<Color,SIZE>::Direction Direction;

        typedef MoveList<Board::AREA> ML;
        typedef Move M;

        Board color;
        Color _winner:4, _just_played:4;
        bool _game_over:1;

        Color winner() const { return _winner; }
        Color just_played() const { return _just_played; }

        float operator[] (size_t i) {
                uint8_t x, y;
                size_t n=0;
                for (size_t i=0; i < Board::DSIZE; ++i) {
                        x = i % Board::DSIZE;
                        y = i / Board::DSIZE;

                        if (color.valid(x,y)) {
                                if (n == i) {
                                        if (color.get(x,y) == BLACK) return 1.0;
                                        if (color.get(x,y) == WHITE) return -1.0;
                                        return 0.0;
                                }
                                n++;
                        }
                }
                DIE("not reached");
        }

        bool game_over() { return _game_over; }
        void set_game_over() { _game_over = true; }
        Color current() { return other(_just_played); }

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

        void place(Color c, uint8_t x, uint8_t y) {
                assert(color.get(x,y) == NONE);
                color.set(x,y,c);
        }

        float result(Color c) {
                if (_winner == c) return 1.0;
                else if (_winner == other(c)) return 0;
                return 0.5;
        }

        size_t scan(Color c, Direction d, uint8_t x, uint8_t y) {
                if (color.get(x,y) != c)
                        return 0;

                if (color.has_move(d, x, y)) {
                        color.move(d, x, y);
                        return 1+scan(c, d, x, y);
                }

                return 1;
        }

        size_t line_length(Color c, Direction d, uint8_t x, uint8_t y) {
                size_t count = 0;
                count += scan(c, d, x, y);
                count += scan(c, color.reverse(d), x, y);
                return count > 0 ? count-1 : 0;
        }

        bool n_in_row(uint8_t n, uint8_t x, uint8_t y) {
                Color c = color.get(x,y);
                return (line_length(c, Board::EAST, x, y) == n) ||
                       (line_length(c, Board::NE, x, y)   == n) ||
                       (line_length(c, Board::NW, x, y)   == n);
        }

        bool at_least_n_in_row(uint8_t n, uint8_t x, uint8_t y) {
                Color c = color.get(x,y);
                return (line_length(c, Board::EAST, x, y) >= n) ||
                       (line_length(c, Board::NE, x, y)   >= n) ||
                       (line_length(c, Board::NW, x, y)   >= n);
                return false;
        }


        int score(bool maximise) {
                return 0;
        }

        Color complete(uint8_t x, uint8_t y) {
                Color c = color.get(x,y);

                if (n_in_row(N-1, x, y))
                        return other(c);

                if (at_least_n_in_row(N,x,y))
                        return c;

                return NONE;
        }

        string move_str(const Move &move) const {
                stringstream s;
                s << "move(" << ColorStr(other(_just_played)) << ") = "
                            << color.label(move.x(), move.y());
                return s.str();
        }

        void announce(const Move &move) const {
                LOG(move_str(move));
        }

        void move(const Move &m) {
                Color player = current();

                uint8_t x=m.x(), y=m.y();
                place(player, x, y);

                _winner = complete(x, y);
                if (_winner != NONE)
                        _game_over = true;

                _just_played = player;
        }

        bool random_move(ML &ml, M &m) {
                uint8_t x, y;
                for (size_t i=0; i < Board::DSIZE; ++i) {
                        size_t n = random() % Board::DSIZE*Board::DSIZE;
                        x = n % Board::DSIZE;
                        y = n / Board::DSIZE;

                        if (color.valid(x,y) && color.get(x,y) == NONE) {
                                m.set(x,y);
                                return true;
                        }
                }
                ml.clear();
                moves(ml);
                if (!ml.size())
                        return false;
                m = ml.move[random() % ml.size()];
                return true;
        }

        void moves(Color c, ML &ml) {
                for (uint8_t y=0; y < Board::DSIZE; ++y)
                        for (uint8_t x=0; x < Board::DSIZE; ++x) {
                                if (!color.valid(x,y))
                                        break;
                                if (color.get(x,y) == NONE)
                                        ml.add(x, y);
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

        string str() {
                stringstream s;
                for (uint8_t y=0; y < Board::DSIZE; ++y) {
                        for (int i=0; i < (Board::DSIZE-color.width(y)); ++i) {
                                s << ' ';
                        }
                        for (uint8_t x=0; x < Board::DSIZE; ++x) {
                                if (!color.valid(x,y))
                                        break;
                                char c = '-';
                                if (color.get(x,y) == BLACK) c = 'X';
                                else if (color.get(x,y) == WHITE) c = 'O';
                                s << c << ' ';
                        }
                        s << endl;
                }
                return s.str();
        }

        void print() {
                cout << str();
                cout << flush;
        }
};

} // namespace yavalath

#endif // YAVALATH_H
