#ifndef TANBO_H
#define TANBO_H
#pragma once

#include <engine/board.h>
#include <engine/memory.h>
#include <engine/state.h>
#include <engine/montecarlo.h>
#include <ui/board.h>

#include <bitset>


namespace tanbo {

#pragma pack(1)
template <size_t SIZE>
struct Move {
        uint16_t index;

        string str() const {
                stringstream s;
                s << "{ (" << (index%SIZE) << ", " << (index/SIZE) << ") }";
                return s.str();
        }

        void debugMove(Color c) const {
                LOG("move(" << ColorStr(c) << ") = " << str());
        }

        bool operator == (Move&r) const { return index == r.index; }
        void set(uint16_t i) { index=i; }
};

#pragma pack(1)
template <typename S, size_t SIZE>
struct MoveList {
        Move<SIZE> moves[SIZE*SIZE];
        uint16_t count;
        Move<SIZE> m;

        size_t size() const { return count; }

        MoveList() { clear(); }

        void clear() {
                count=0;
                m.index=0;
        }

        void add(size_t i) {
                moves[count++].index = i;
        }

        Move<SIZE>& operator[](size_t i) {
                return moves[i];
        }
};

static GBoard WINDOW(500, 500, 19);


#pragma pack(1)
template <size_t SIZE, size_t MAX_MOVES>
struct State : BaseState {
        typedef board::Square<Color,SIZE> Board;
        typedef MoveList<State, SIZE> ML;
        typedef Move<SIZE> M;

        enum Direction { N, S, E, W, NE, NW, SE, SW };
        enum { NUM_DIRECTIONS=SW+1 };


        bitset<SIZE*SIZE> black;
        bitset<SIZE*SIZE> white;

        typedef MCMoveCounter<State, MAX_MOVES> MoveCounter;

        size_t get_index(const M &m) { return m.index; }
        bool valid_index(size_t i) { return !occupied(i); }
        void set_index(M &m, size_t i) { m.index = i; }

        void setup9x9() {
                place(WHITE, 1+1*9);
                place(BLACK, 7+1*9);
                place(BLACK, 1+7*9);
                place(WHITE, 7+7*9);
        }

        void initial() {
                reset();
                black.reset();
                white.reset();
                size_t step=0;
                switch (SIZE) {
                case 19: step = 6; break;
                case 13: step = 4; break;
                case 9:
                         setup9x9();
                         return;
                default: DIE("board size not supported: "  << SIZE);
                }

                Color c = BLACK;
                for (size_t y=0; y < SIZE; y += step) {
                        for (size_t x=0; x < SIZE; x += step) {
                                place(c, x+y*SIZE);
                                c = other(c);
                        }
                        c = other(c);
                }
        }

        float operator[](size_t i) const {
                if (black[i]) return  1.0;
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
                initial();
        }

        void place(Color c, uint16_t i) {
                switch (c) {
                case BLACK: black.set(i); break;
                case WHITE: white.set(i); break;
                default: DIE("bad color provided: " << c);
                }
        }

        void remove(uint16_t i) {
                assert(occupied(i));

                switch (color(i)) {
                case BLACK: black.reset(i); break;
                case WHITE: white.reset(i); break;
                default: DIE("color error: " << i);
                }
        }

        int score(bool maximise) {
                ML ml;
                size_t gb, gw;
                gb = finder.find(BLACK, *this, ml);
                size_t cb = ml.size();
                ml.clear();
                gw = finder.find(WHITE, *this, ml);
                size_t cw = ml.size();
                return (cb-cw) + 10*(gb-gw);
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
                place(player, m.index);
                finder.remove_dead(*this);
                size_t bc = black.count(),
                       wc = white.count();

                if (!bc || !wc) {
                        set_game_over();
                        if (!bc && wc)
                                _winner = WHITE;
                        else if (bc && !wc)
                                _winner = BLACK;
                }
                end_move();
        }

        bool random_move(ML &ml, M &m) {
                ml.clear();
                moves(ml);
                if (ml.count == 0)
                        return false;
                m.index = ml.moves[random() % ml.count].index;
                return true;
        }

        struct MoveFinder {
                bool visited[SIZE*SIZE];
                bool visited_remove[SIZE*SIZE];
                bitset<SIZE*SIZE> occupied;

                void reset() {
                        occupied.reset();
                        memset(visited, 0, sizeof(bool)*SIZE*SIZE);
                }

                bool adjacent_occupied(State &s, Color c, Direction d, uint16_t m, uint16_t i) {
                        uint16_t k=m;
                        if (!s.valid(d, k))
                                return false;
                        s.move_dir(d, k);
                        if (k != i && s.color(k) == c)
                                return true;
                        return false;
                }

                bool valid_move(Color c, State &s, uint16_t m, uint16_t i) {
                        Direction dir[4] = { N, S, E, W };
                        for (size_t d=0; d < 4; ++d) {
                                if (adjacent_occupied(s, c, dir[d], m, i))
                                        return false;
                        }
                        return true;
                }

                void find_moves(Color c, State &s, ML &ml, size_t i) {
                        uint16_t k;
                        Direction dir[4] = { N, S, E, W };
                        for (size_t d=0; d < 4; ++d) {
                                k = i;
                                if (s.valid(dir[d], k)) {
                                        s.move_dir(dir[d], k);
                                        if (!visited[k] && !occupied[k] && valid_move(c, s, k, i)) {
                                                ml.add(k);
                                                visited[k] = true;
                                        }
                                }
                        }
                }

                size_t trace(Color c, State &s, ML &ml, size_t i) {
                        Direction dir[4] = { N, S, E, W };
                        uint16_t k;

                        visited[i] = true;
                        size_t count = 1;
                        find_moves(c, s, ml, i);

                        // pursue group connection
                        for (size_t d=0; d < 4; ++d) {
                                if (s.valid(dir[d], i)) {
                                        k = i;
                                        s.move_dir(dir[d], k);
                                        if (!visited[k] && s.color(k) == c)
                                                count += trace(c, s, ml, k);
                                }
                        }
                        return count;
                }

                size_t find(State &s, ML &ml) {
                        reset();
                        occupied = s.black | s.white;
                        size_t groups=0;
                        for (size_t i=0; i < SIZE*SIZE; ++i)
                                if (occupied[i] && !visited[i] && s.color(i) == s.current()) {
                                        trace(s.color(i), s, ml, i);
                                        groups++;
                                }
                        return groups;
                }

                size_t find(Color c, State &s, ML &ml) {
                        reset();
                        occupied = s.black | s.white;
                        size_t groups=0;
                        for (size_t i=0; i < SIZE*SIZE; ++i)
                                if (occupied[i] && !visited[i] && s.color(i) == c) {
                                        trace(c, s, ml, i);
                                        groups++;
                                }
                        return groups;
                }

                void remove_adjacent(Color c, State &s, Direction d, uint16_t i) {
                        uint16_t k = i;
                        
                        s.move_dir(d, k);
                        if (visited_remove[k] || s.color(k) != c)
                                return;

                        remove_group(c, s, k);
                }

                void remove_group(Color c, State &s, uint16_t i) {
                        visited_remove[i] = true;
                        Direction dir[4] = { N, S, E, W };
                        for (size_t d=0; d < 4; ++d) {
                                if (s.valid(dir[d], i))
                                        remove_adjacent(c, s, dir[d], i);
                        }
                        s.remove(i);
                }

                void remove_dead(State &s) {
                        reset();
                        occupied = s.black | s.white;
                        ML ml;
                        vector<uint16_t> to_remove;

                        for (uint16_t i=0; i < SIZE*SIZE; ++i) {
                                ml.clear();
                                if (occupied[i] && !visited[i] && s.color(i) == s.current()) {
                                        trace(s.color(i), s, ml, i);
                                        memset(visited_remove, 0, sizeof(bool)*SIZE*SIZE);
                                        if (ml.size() == 0) {
                                                to_remove.push_back(i);
                                        }
                                }
                        }
                        for (uint16_t i=0; i < to_remove.size(); ++i)
                                remove_group(s.color(to_remove[i]), s, to_remove[i]);
                }
        };

        MoveFinder finder;

        void moves(ML &ml) {
                ml.clear();
                if (game_over())
                        return;

                finder.find(*this, ml);

                if (ml.count == 0)
                        set_game_over();
        }

        void display() {
                WINDOW.bsize = SIZE;
                GBoard::player_t b, w;
                for (uint16_t i=0; i < Board::AREA; ++i) {
                        if (color(i) == BLACK) b.push_back(i);
                        else if (color(i) == WHITE) w.push_back(i);
                }
                WINDOW.update(GBoard::GO, b, w);
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

        void print() {
                display();
                return;
                cout << str();
                cout << flush;
        }
};

} // namespace connect6

#endif // TANBO_H
