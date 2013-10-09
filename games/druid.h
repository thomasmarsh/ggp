#ifndef DRUID_H
#define DRUID_H
#pragma once

#include "search.h"
#include "board.h"

namespace druid {

enum Type { EMPTY, SARSEN, LINTEL };
enum Direction { XDIR, YDIR };


static uint8_t HIGHEST = 0;
static const size_t MAX_HEIGHT = 256;

#pragma pack(1)
struct Move {
        uint8_t x:4, y:4,
                _type:1,
                _dir:1;

        Type type() const { return _type ? SARSEN : LINTEL; }
        Direction dir() const { return _dir ? XDIR : YDIR; }

        void set(uint8_t _x, uint8_t _y, Type t, Direction d) {
                x = _x,
                y = _y,
                _type = (t == SARSEN) ? 1 : 0,
                _dir = (d == XDIR) ? 1 : 0;
        }


        string str() const {
                stringstream s;
                s << "{ ";
                switch (type()) {
                case SARSEN: s << "SARSEN"; break;
                case LINTEL: s << "LINTEL"; break;
                case EMPTY:  s << "EMPTY"; break;
                default: s << "<unknown>"; break;
                }
                s << " " << (int) x << " " << (int) y;
                if (type() == LINTEL) {
                        s << " ";
                        switch (dir()) {
                        case XDIR: s << "XDIR"; break;
                        case YDIR: s << "YDIR"; break;
                        default: s << "<unknown>"; break;
                        }
                }
                s << " }";
                return s.str();
        }

        bool operator == (Move&r) const {
                return x == r.x &&
                       y == r.y &&
                       type() == r.type() &&
                       (type() == LINTEL ? dir() == r.dir() : true);
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

        void addSarsen(uint8_t x, uint8_t y) {
                assert((count+2U) < MAX_MOVES);
                move[count++].set(x, y, SARSEN, XDIR);
        }

        void addLintel(uint8_t x, uint8_t y, Direction dir) {
                assert((count+2U) < MAX_MOVES);
                move[count++].set(x, y, LINTEL, dir);
        }
};


#pragma pack(1)
template <size_t SIZE,
          size_t MAX_MOVES>
struct State {
        typedef MoveList<MAX_MOVES> ML;
        typedef Move M;

        static SquarePathFinder<SIZE> path_finder;

        board::Square<uint8_t,SIZE> top;
        board::Square<Color,SIZE> color;

        Color _winner:2, _just_played:2;
        bool _game_over:1,
             score_cached:1;

        int score_value;

        bool game_over() { return _game_over; }
        void set_game_over() { _game_over = true; }
        Color winner() { return _winner; }
        Color just_played() { return _just_played; }

        Color current() { return other(_just_played); }

        State() { clear(); }

        State(const State &rhs) { copy_from(rhs); }

        void copy_from(const State &rhs) {
                memcpy(this, &rhs, sizeof(State));
                score_cached = false;
        }

        void clear() {
                memset(this, 0, sizeof(State));
                _just_played = WHITE;
                _winner = NONE;
                score_cached = false;
        }

        void placeSarsen(Color c, uint8_t x, uint8_t y) {
#ifndef NDEBUG
                Color tc = color.get(x,y);
#endif
                assert(tc == NONE || tc == c);
                top.set(x, y, top.get(x,y)+1);
                color.set(x, y, c);
        }

        float result(Color c) {
                if (_winner == c) return 1.0;
                else if (_winner == other(c)) return 0;
                return 0.5;
        }

        int score(bool maximise) {
                if (score_cached)
                        return score_value;

                int cb=0, cw=0;
                for (size_t x=0; x < SIZE; ++x) {
                        for (size_t y=0; y < SIZE; ++y) {
                                switch (color.get(x,y)) {
                                case BLACK: cb += top.get(x,y); break;
                                case WHITE: cw += top.get(x,y); break;
                                default: break;
                                }
                        }
                }
                int lb = path_finder.longest(BLACK, &color),
                    lw = path_finder.longest(WHITE, &color);

                int wb = (_game_over && _winner == BLACK) ?  1 : 0,
                    ww = (_game_over && _winner == WHITE) ? -1 : 0;

                //cb += maximise ? 5 : 0;
                //cb -= maximise ? 0 : 5;

                score_value = (cb-cw) * 1
                            + (lb-lw) * 20
                            + (wb-ww) * 1000;

                score_cached = true;
                return score_value;
        }

        void placeLintel(Color c, uint8_t x, uint8_t y, Direction dir) {
                uint8_t mx = x + (dir == XDIR ? 2 : 0),
                        my = y + (dir == YDIR ? 2 : 0);
                uint8_t z = top.get(x, y);
                for (uint8_t ix=x; ix < (mx+1); ++ix) {
                        for (uint8_t iy=y; iy < (my+1); ++iy) {
                                top.set(ix, iy, z+1);
                                color.set(ix, iy, c);
                        }
                }
        }

        string move_str(const Move &m) {
                stringstream s;
                s << "move(" << ColorStr(other(_just_played)) << ") = " << m.str();
                return s.str();
        }

        void announce(const Move &m) {
                m.debugMove(other(_just_played));
        }

        void move(const Move &m) {
                Color player = current();

                switch (m.type()) {
                case SARSEN: placeSarsen(player, m.x, m.y); break;
                case LINTEL: placeLintel(player, m.x, m.y, m.dir()); break;
                case EMPTY:
                default:
                        assert(m.type() == SARSEN || m.type() == LINTEL);
                        break;
                }

                if (path_finder.connected(player, &color)) {
                        _winner = player;
                        _game_over = true;
                }

                _just_played = player;
        }

        void sarsenMoves(Color c, ML &ml) const {
                for (uint8_t x=0; x < SIZE; ++x) {
                        for (uint8_t y=0; y < SIZE; ++y) {
                                const Color tc = color.get(x, y);
                                if ((tc == NONE || tc == c) && (top.get(x,y) < (SIZE-2)))
                                        ml.addSarsen(x, y);
                        }
                }
        }

        void tryLintel(Color c, ML &ml, uint8_t x, uint8_t y, Direction dir) const {
                const uint8_t nx = x + (dir == XDIR ? 1 : 0),
                              ny = y + (dir == YDIR ? 1 : 0),
                              mx = x + (dir == XDIR ? 2 : 0),
                              my = y + (dir == YDIR ? 2 : 0),
                              to = top.get(x,y),
                              tn = top.get(nx,ny),
                              tm = top.get(mx,my);

                assert(mx < SIZE && my < SIZE);

                if ((to != tm) // unequal ends
                 || (tn > to)  // middle higher
                 || (to == 0)  // can't place on ground level
                 || (to > (SIZE-2)))  // can't exceed MAX_HEIGHT
                        return;

                uint8_t count=0;
                if (color.get(x,y) == c) count++;
                if (color.get(mx,my) == c) count++;
                if (tn == to && color.get(nx,ny) == c) count++;
                if (count != 2) return; // must have two like colors supporting

                ml.addLintel(x, y, dir);
        }

        void lintelMoves(Color c, ML &ml) const {
                for (uint8_t x=0; x < (SIZE-2); ++x) {
                        for (uint8_t y=0; y < (SIZE-2); ++y) {
                                tryLintel(c, ml, x, y, XDIR);
                                tryLintel(c, ml, x, y, YDIR);
                        }
                }

                for (uint8_t x=0; x < (SIZE-2); ++x)
                        for (uint8_t y=(SIZE-2); y < SIZE; ++y)
                                tryLintel(c, ml, x, y, XDIR);

                for (uint8_t x=(SIZE-2); x < SIZE; ++x)
                        for (uint8_t y=0; y < (SIZE-2); ++y)
                                tryLintel(c, ml, x, y, YDIR);
        }

        void moves(ML &ml) {
                if (_game_over)
                        return;

                Color player = other(_just_played);

                sarsenMoves(player, ml);
                lintelMoves(player, ml);

                if (ml.size() > HIGHEST) {
                        HIGHEST = ml.size();
                        if (HIGHEST > (MAX_MOVES-2))
                                LOG("highest = " << (int) HIGHEST);
                }
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

        string repr(bool height, bool header) const {
                stringstream s;
                const char *xl = "ABCDEFGHIJKLMNOPQRTUVWXYZ";
                if (header) {
                        s << "   ";
                        for (uint8_t x=0; x < SIZE; ++x)
                                s << xl[x] << ' ';
                        s << endl;
                }

                for (uint8_t y=0; y < SIZE; ++y) {
                        s << (SIZE-y) << "  ";
                        for (uint8_t x=0; x < SIZE; ++x) {
                                if (height) {
                                        if (top.get(x,y) == 0)
                                                s << '.';
                                        else {
                                                char buf[3];
                                                snprintf(buf, 3, "%01X", top.get(x,y));
                                                s << buf;
                                        }
                                } else {
                                        char c = '.';
                                        if (color.get(x,y) == BLACK) c = 'X';
                                        else if (color.get(x,y) == WHITE) c = 'O';
                                        s << c;
                                }
                                s << ' ';
                        }
                        s << ' ' << (SIZE-y) << endl;
                }

                s << "   ";
                for (uint8_t x=0; x < SIZE; ++x)
                        s << xl[x] << ' ';
                s << endl;
                return s.str();
        }

        string str() const { return repr(false, true); }
        string height() const { return repr(true, false); }

        void print() const {
                //cout << "-----------------------------------" << endl;
                cout << str();
                cout << height();
                cout << flush;
        }


        bool is_number(const std::string& s) const {
                std::string::const_iterator it = s.begin();
                while (it != s.end() && std::isdigit(*it)) ++it;
                return !s.empty() && it == s.end();
        }

        bool parse_move(M &m, const vector<string> &tokens) {
                if (tokens.size() < 3) {
                        cout << "! need at least three arguments" << endl;
                        return false;
                }

                if (tokens[0][0] != 's' && tokens[0][0] != 'l') {
                        cout << "! first argument must be sarsen or lintel" << endl;
                        return false;
                }

                if (tokens[0][0] == 'l' && tokens.size() < 4) {
                        cout << "! lintel requires direction" << endl;
                        return false;
                }

                if (!is_number(tokens[1]) || !is_number(tokens[2])) {
                        cout << "! x and y must be numbers" << endl;
                        return false;
                }

                druid::Direction d = druid::XDIR;
                druid::Type t = (tokens[0][0] == 's') ? druid::SARSEN : druid::LINTEL;
                uint8_t x = atoi(tokens[1].c_str());
                uint8_t y = atoi(tokens[2].c_str());
                if (t == druid::LINTEL) {
                        if (tokens[3][0] == 'x')
                                d = druid::XDIR;
                        else if (tokens[3][0] == 'y')
                                d = druid::YDIR;
                        else {
                                cout << "! direction must be x or y" << endl;
                                return false;
                        }
                }
                m.set(x, y, t, d);
                return true;
        }
};


} // namespace druid

#endif // DRUID_H
