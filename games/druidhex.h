#ifndef DRUIDHEX_H
#define DRUIDHEX_H
#pragma once

#include "search.h"
#include "board.h"

//#define COUNT_PIECES

namespace druidhex {

enum Type { EMPTY, SARSEN, LINTEL };

static uint8_t HIGHEST = 0;
static const size_t MAX_HEIGHT = 256;

template <size_t SIZE>
struct HexPathFinder {
        typedef board::Hex<Color,SIZE> Board;
        typedef board::Hex<bool,SIZE> BoolBoard;
        typedef typename Board::Direction Direction;
        typedef typename BoolBoard::Direction BoolDirection;

        enum Side { A, B, C, D, E, F };
        enum Target { ACE, BDF };

        struct Point {
                uint8_t x, y;
                bool operator == (const Point &p) const {
                        return x==p.x && y==p.y;
                }
        };

        Point points[6][SIZE];

        Board *map;
        Color color;
        board::Hex<bool,SIZE> visited;

        bool edge_seen[2];

        void populate_points() {
                BoolDirection d[6] = {
                        BoolBoard::EAST, BoolBoard::SE, BoolBoard::SW,
                        BoolBoard::WEST, BoolBoard::NW, BoolBoard::NE
                };

                uint8_t x=0, y=0;
                while (!visited.valid(x,y))
                        ++x;

                assert(visited.valid(x,y));

                for (size_t s=0; s < 6; ++s) {
                        size_t i=0;
                        for (; i < (SIZE-1); ++i) {
                                points[s][i].x = x;
                                points[s][i].y = y;
                                visited.move(d[s], x, y);
                        }
                        points[s][i].x = x;
                        points[s][i].y = y;
                }
        }

        bool found_connection() {
                return edge_seen[0] && edge_seen[1];
        }

        size_t edge_num(Side s) {
                switch (s) {
                case C: case D: return 0;
                case E: case F: return 1;
                case A: case B: DIE("unexpected initial side: " << s);
                }
                DIE("unexpected side: " << s);
        }

        void mark_edge2(Point & p, size_t i) {
                size_t n = edge_num((Side) i);
                if (!edge_seen[n])
                        for (size_t j=0; j < SIZE; ++j)
                                if (p == points[i][j])
                                        edge_seen[n] = true;
        }

        void mark_edge(Target t, uint8_t x, uint8_t y) {
                Point p;
                p.x = x; p.y = y;
                for (size_t i=(t==BDF?3:2); i < 6; i += 2)
                        mark_edge2(p, i);
        }


        // constructor arg seems necessary; clang bug?
        HexPathFinder(int _dummy) : map(0), color(NONE) { populate_points(); }

        bool seen(uint8_t x, uint8_t y) { return visited.get(x, y); }
        bool mycolor(uint8_t x, uint8_t y) { return map->get(x,y) == color; }

        void scan(Target t, uint8_t x, uint8_t y) {
                visited.set(x, y, true);
                mark_edge(t, x, y);
                if (found_connection())
                        return;

                BoolDirection d;
                uint8_t nx, ny;
                for (size_t i=0; i < 6; ++i) {
                        d = (BoolDirection) i;
                        if (visited.has_move(d, x, y)) {
                                nx = x, ny = y;
                                visited.move(d, nx, ny);
                                if (!seen(nx, ny) && mycolor(nx, ny)) {
                                        scan(t, nx, ny);
                                        if (found_connection())
                                                return;
                                }
                        }
                }
        }

        bool search(Target t) {
                memset(&visited, 0, sizeof(visited));
                edge_seen[0] = false;
                edge_seen[1] = false;
                Point p;
                for (size_t i=0; i < SIZE; ++i) {
                        p = points[t==BDF?1:0][i];
                        if (mycolor(p.x, p.y)) {
                                scan(t, p.x, p.y);
                                if (found_connection())
                                        return true;
                        }
                }
                return false;
        }

        bool connected(Color c, Board *m) {
                map = m;
                color = c;
                return search(ACE) || search(BDF);
        }

        void clear(Color c, Board *m) {
                color = c;
                map = m;
                memset(visited, 0, sizeof(bool)*(SIZE*SIZE));
        }

        size_t longest(Color c, Board *m) { return 1; }
};



#pragma pack(1)
template <size_t SIZE>
struct Move {
        typedef board::Hex<Color,SIZE> Board;
        typedef typename Board::Direction D;

        uint8_t _x:4,
                _y:4,
                _type:1,
                _dir:2;

        uint8_t x() const { return _x; }
        uint8_t y() const { return _y; }
        Type type() const { return _type ? SARSEN : LINTEL; }

        D dir() const {
                switch (_dir) {
                case 1: return Board::EAST;
                case 2: return Board::SE;
                case 3: return Board::SW;
                }
                DIE("unhandled case");
        }

        void set(uint8_t nx, uint8_t ny, Type t, D d) {
                _x = nx,
                _y = ny,
                _type = (t==SARSEN) ? 1 : 0;
                switch (d) {
                case Board::EAST: _dir = 1; break;
                case Board::SE: _dir = 2; break;
                case Board::SW: _dir = 3; break;
                default: DIE("unhandled case");
                }
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
                s << " " << (int) x() << " " << (int) y();
                if (type() == LINTEL) {
                        s << " ";
                        switch (dir()) {
                        case Board::EAST: s << "EAST"; break;
                        case Board::SE: s << "SE"; break;
                        case Board::SW: s << "SW"; break;
                        default: s << "<unknown>"; break;
                        }
                }
                s << " }";
                return s.str();
        }

        bool operator == (Move&r) const {
                return x() == r.x() &&
                       y() == r.y() &&
                       type() == r.type() &&
                       (type() == LINTEL ? _dir == r._dir : true);
        }

        void debugMove(Color c) const {
                LOG("move(" << ColorStr(c) << ") = " << str());
        }
};

#pragma pack(1)
template <size_t SIZE, size_t AREA>
struct MoveList {
        typedef board::Hex<Color,SIZE> Board;
        typedef typename Board::Direction D;

        Move<SIZE> move[AREA];
        uint8_t count;

        size_t size() { return count; }

        MoveList() { clear(); }
        void clear() { memset(this, 0, sizeof(MoveList)); }
        Move<SIZE>& operator[](size_t i) { return move[i]; }

        void addSarsen(uint8_t x, uint8_t y) {
                assert((count+2U) < AREA);
                move[count++].set(x, y, SARSEN, Board::EAST);
        }

        void addLintel(uint8_t x, uint8_t y, D dir) {
                assert((count+2U) < AREA);
                move[count++].set(x, y, LINTEL, dir);
        }
};


#pragma pack(1)
template <size_t SIZE,
          size_t AREA,
          size_t NUM_SARSENS,
          size_t NUM_LINTELS>
struct State {
        //--------------------------------------------------------------------
        //
        // Typedefs
        //
        //--------------------------------------------------------------------

        typedef board::Hex<Color,SIZE> Board;
        typedef board::Hex<uint8_t,SIZE> TBoard;
        typedef typename Board::Direction Direction;
        typedef typename TBoard::Direction TDirection;
        typedef MoveList<SIZE, AREA> ML;
        typedef Move<SIZE> M;

        //--------------------------------------------------------------------
        //
        // Static path finder utility
        //
        //--------------------------------------------------------------------

        static HexPathFinder<SIZE> path_finder;


        //--------------------------------------------------------------------
        //
        // Member variables
        //
        //--------------------------------------------------------------------

        board::Hex<uint8_t,SIZE> top;
        board::Hex<Color,SIZE> color;

#ifdef COUNT_PIECES
        uint8_t black_sarsens, white_sarsens,
                black_lintels, white_lintels;
#endif

        Color _winner:2,
              _just_played:2;
        bool _game_over:1;
#ifdef USE_SCORE
        int score_value;
        bool score_cached;
#endif

        Color winner() { return _winner; }
        Color just_played() { return _just_played; }


        //--------------------------------------------------------------------
        //
        // Implementation
        //
        //--------------------------------------------------------------------

        bool game_over() { return _game_over; }
        void set_game_over() { _game_over = true; }

        Color current() { return other(_just_played); }

        State() { clear(); }

        State(const State &rhs) { copy_from(rhs); }

        void copy_from(const State &rhs) {
                memcpy(this, &rhs, sizeof(State));
#ifdef USE_SCORE
                score_cached = false;
#endif
        }

        void clear() {
                memset(this, 0, sizeof(State));
                _just_played = WHITE;
                _winner = NONE;
#ifdef USE_SCORE
                score_cached = false;
#endif
#ifdef COUNT_PIECES
                white_sarsens = (NUM_SARSENS >> 1)+1;
                black_sarsens = (NUM_SARSENS >> 1)+1;
                white_lintels = (NUM_LINTELS >> 1)+1;
                black_lintels = (NUM_LINTELS >> 1)+1;
#endif
        }

        void placeSarsen(Color c, uint8_t x, uint8_t y) {
#ifndef NDEBUG
                Color tc = color.get(x,y);
#endif
                assert(tc == NONE || tc == c);
                top.set(x, y, top.get(x,y)+1);
                color.set(x, y, c);
#ifdef COUNT_PIECES
                switch (c) {
                case BLACK: assert(black_sarsens > 0); black_sarsens--; break;
                case WHITE: assert(white_sarsens > 0); white_sarsens--; break;
                default: DIE("bad color: " << c);
                }
#endif
        }

        float result(Color c) {
                if (_winner == c) return 1.0;
                else if (_winner == other(c)) return 0;
                return 0.5;
        }

        int score(bool maximise) {
#ifdef USE_SCORE
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
#else
                return 0;
#endif
        }

        void placeLintel(Color c, uint8_t x, uint8_t y, Direction dir) {
                uint8_t z = 1+top.get(x,y),
                        ix = x,
                        iy = y;

                for (int i=0; i < 3; ++i) {
                        top.set(ix, iy, z);
                        color.set(ix, iy, c);
                        color.move(dir, ix, iy);
                }

#ifdef COUNT_PIECES
                switch (c) {
                case BLACK: assert(black_lintels > 0); black_lintels--; break;
                case WHITE: assert(white_lintels > 0); white_lintels--; break;
                default: assert(1 == 0);
                }
#endif
        }

        string move_str(const M &m) const {
                stringstream s;
                s << "move(" << ColorStr(other(_just_played)) << ") = " << color.label(m.x(), m.y());

                if (m.type() == LINTEL) {
                        uint8_t bx=m.x(), by=m.y();
                        color.move(m.dir(), bx, by);
                        color.move(m.dir(), bx, by);
                        s << '-' << color.label(bx, by);
                }
                return s.str();
        }

        void announce(const M &m) const {
                LOG(move_str(m));
        }

        void move(const M &m) {
                Color player = current();

                switch (m.type()) {
                case SARSEN: placeSarsen(player, m.x(), m.y()); break;
                case LINTEL: placeLintel(player, m.x(), m.y(), m.dir()); break;
                case EMPTY:
                default:
                        assert(m.type() == SARSEN || m.type() == LINTEL);
                        break;
                }

                if (path_finder.connected(player, &color)) {
                        _winner = player;
                        _game_over = true;
                }

#ifdef COUNT_PIECES
                if ((black_sarsens == 0 && black_lintels == 0) ||
                    (white_sarsens == 0 && white_lintels == 0))
                        _game_over = true;
#endif

                _just_played = player;
        }

        void sarsenMoves(Color c, ML &ml) const {
#ifdef COUNT_PIECES
                switch (c) {
                case BLACK: if (black_sarsens == 0) return; break;
                case WHITE: if (white_sarsens == 0) return; break;
                default: assert(1==0);
                }
#endif
                for (uint8_t y=0; y < Board::DSIZE; ++y) {
                        for (uint8_t x=0; x < Board::DSIZE; ++x) {
                                if (!color.valid(x,y))
                                        break;

                                const Color tc = color.get(x, y);
                                if ((tc == NONE || tc == c) && (top.get(x,y) < (SIZE-2)))
                                        ml.addSarsen(x, y);
                        }
                }
        }

        void tryLintel(Color c, ML &ml, uint8_t x, uint8_t y, Direction d) const {
                uint8_t nx=x, ny=y;
                color.move(d, nx, ny);

                if (!color.has_move(d, nx,ny))
                        return;

                uint8_t mx=nx, my=ny;
                color.move(d, mx, my);

                if (!color.has_move(d, mx,my))
                        return;

                const uint8_t to = top.get(x,y),
                              tn = top.get(nx,ny),
                              tm = top.get(mx,my);


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

                ml.addLintel(x, y, d);
        }

        void lintelMoves(Color c, ML &ml) const {
#ifdef COUNT_PIECES
                switch (c) {
                case BLACK: if (black_lintels == 0) return; break;
                case WHITE: if (white_lintels == 0) return; break;
                default: DIE("bad color: " << c);
                }
#endif

                // TODO: optimize
                for (uint8_t y=0; y < SIZE; ++y) {
                        for (uint8_t x=0; x < SIZE; ++x) {
                                if (!color.valid(x,y))
                                        break;

                                tryLintel(c, ml, x, y, Board::EAST);
                                tryLintel(c, ml, x, y, Board::SE);
                                tryLintel(c, ml, x, y, Board::SW);
                        }
                }
        }

        void moves(ML &ml) {
                if (_game_over)
                        return;

                Color player = other(_just_played);

                sarsenMoves(player, ml);
                lintelMoves(player, ml);

                if (ml.size() > HIGHEST) {
                        HIGHEST = ml.size();
                        if (HIGHEST > (AREA-2))
                                LOG("highest = " << (int) HIGHEST);
                }
                if (ml.size() == 0)
                        _game_over = true;
        }

        bool random_move(ML &ml, M &m) {
                ml.clear();
                moves(ml);
                if (!ml.size())
                        return false;
                m = ml.move[random() % ml.size()];
                return true;
        }

        string repr(bool height) const {
                stringstream s;
                const char *yl = "abcdefghijklmnopq";
                for (int i=0; i < (Board::DSIZE-color.width(0)); ++i)
                        s << ' ';
                s << "       ";
                for (int i=0; i < color.width(0); ++i) s << (int) (i+1) << ' ';
                s << endl;
                for (int i=0; i < (Board::DSIZE-color.width(0)); ++i) s << ' ';
                s << "      ";
                for (int i=0; i < color.width(0); ++i) s << "/ ";
                s << (SIZE+1) << endl;
                s << "     ";
                for (size_t i=0; i < (Board::DSIZE+SIZE); ++i) s << ' ';
                s << "/ " << (2+SIZE) << endl;
                for (uint8_t y=0; y < Board::DSIZE; ++y) {
                        for (int i=0; i < (Board::DSIZE-color.width(y)); ++i)
                                s << ' ';
                        s << yl[y] << "-  ";
                        for (uint8_t x=0; x < Board::DSIZE; ++x) {
                                if (!color.valid(x,y))
                                        break;

                                if (height) {
                                        if (top.get(x,y) == 0)
                                                s << ". ";
                                        else {
                                                char buf[3];
                                                snprintf(buf, 3, "%01X", top.get(x,y));
                                                s << buf << ' ';
                                        }
                                } else {
                                        char c = '.';
                                        if (color.get(x,y) == BLACK) c = 'X';
                                        else if (color.get(x,y) == WHITE) c = 'O';
                                        s << c << ' ';
                                }
                        }
                        if (y < (SIZE-2)) s << "  /";
                        if (y < (SIZE-3)) s << " " << (SIZE+y+3);
                        s << endl;
                }
                return s.str();
        }

        string str()    const { return repr(false); }
        string height() const { return repr(true); }

        void print() const {
                //cout << "-----------------------------------" << endl;
                cout << str();
                cout << height();
                cout << flush;
        }
};

} // namespace druidhex

#endif // DRUIDHEX_H
