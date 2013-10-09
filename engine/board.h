#ifndef BOARD_H
#define BOARD_H

#include "common.h"

#include <vector>
#include <utility>

namespace board {

struct TriHexLookup {
        size_t index(size_t x, size_t y) {
                return index_list[y][x];
        }

        void coord(size_t i, size_t &x, size_t &y) {
                x = coord_list[i].first;
                y = coord_list[i].second;
        }

        typedef std::vector< std::pair<size_t,size_t> > coord_t;
        typedef std::vector< std::vector<size_t> > index_t;

        coord_t coord_list;
        index_t index_list;

        void init(size_t i) {
                size_t y=0, x=0, j=0;

                while (true) {
                        if (x >= y) {
                                x = 0;
                                y++;
                        }
                        coord_list.push_back(std::make_pair(x,y-1));

                        if (index_list.size() < (y))
                                index_list.resize(y);
                        if (index_list[y-1].size() < (x+1))
                                index_list[y-1].resize(x+1);

                        index_list[y-1][x] = j;

                        if (j > (i-1))
                                return;

                        ++j, ++x;
                }
        }
};

//----------------------------------------------------------------------------
//
// Rectangle
//
//----------------------------------------------------------------------------

template <typename T, size_t MAX_X, size_t MAX_Y>
struct Rectangle {
        T data[MAX_Y][MAX_X];
        enum { AREA = MAX_X*MAX_Y };

        bool valid(uint8_t x, uint8_t y) const {
                return x < MAX_X && y < MAX_Y;
        }

        void set(uint8_t x, uint8_t y, const T &value) {
                assert(valid(x,y));
                data[y][x] = value;
        }

        const T& get(uint8_t x, uint8_t y) const {
                assert(valid(x,y));
                return data[y][x];
        }

        T& get(uint8_t x, uint8_t y) {
                assert(valid(x,y));
                return data[y][x];
        }

        string label(uint8_t x, uint8_t y) {
                const char *yl = "abcdefghijlkmnopq",
                           *xl = "123456789ABCDEFGH";

                string s;
                s += xl[x];
                s += yl[y];
                return s;
        }
};


//----------------------------------------------------------------------------
//
// Square (special case of Rectangle)
//
//----------------------------------------------------------------------------

template <typename T, size_t SIZE>
struct Square : Rectangle<T,SIZE,SIZE>
{};


//----------------------------------------------------------------------------
//
// Hex board
//
//----------------------------------------------------------------------------

#pragma pack(1)
template <typename T, size_t SIZE>
struct Hex {
        // centered hexagonal
        enum { AREA = 1+(6*((SIZE*(SIZE-1)))>>1) };
        enum { DSIZE = SIZE*2-1 };

        T data[DSIZE][DSIZE];

        uint8_t width(uint8_t y) const {
                return DSIZE - std::abs((int) SIZE - (int) (y+1));
        }

        bool valid(uint8_t x, uint8_t y) const {
                return y < DSIZE && x < width(y);
        }

        void set(uint8_t x, uint8_t y, const T &value) {
                assert(valid(x,y));
                data[y][x] = value;
        }

        const T &get(uint8_t x, uint8_t y) const {
                return data[y][x];
        }

        T &get(uint8_t x, uint8_t y) {
                return data[y][x];
        }

        string label(uint8_t x, uint8_t y) const {
                const char *yl = "abcdefghijlkmnopqrstuvwxyz";

                stringstream s;
                s << yl[y];
                if (y >= SIZE)
                        s << 1+x+DSIZE-width(y);
                else s << (int) (x+1);
                return s.str();
        }

        // these routines return x (n/s for y is simply +/-1, e/w is simply +/- x)

        bool has_ne(uint8_t x, uint8_t y) const {
                if (y == 0)    return false;
                if (y >= SIZE) return valid(x+1,y-1);
                return valid(x, y-1);
        }

        uint8_t northeast(uint8_t x, uint8_t y) const {
                assert(has_ne(x,y));
                if (y >= SIZE)
                        return x+1;
                return x;
        }

        bool has_se(uint8_t x, uint8_t y) const {
                if (y >= (SIZE-1))  return valid(x,y+1);
                return valid(x+1, y+1);
        }

        uint8_t southeast(uint8_t x, uint8_t y) const {
                assert(has_se(x,y));
                if (y >= (SIZE-1)) {
                        return x;
                }
                return x+1;
        }

        bool has_nw(uint8_t x, uint8_t y) const {
                if (y == 0)    return false;
                if (y >= SIZE) return valid(x,y-1);
                if (x == 0)    return false;
                return valid(x-1, y-1);
        }

        uint8_t northwest(uint8_t x, uint8_t y) const {
                assert(has_nw(x,y));
                if (y >= SIZE)
                        return x;
                return x-1;
        }

        bool has_sw(uint8_t x, uint8_t y) const {
                if (y >= (SIZE-1))  {
                        if (x == 0) return false;
                        return valid(x-1,y+1);
                }
                return valid(x, y+1);
        }

        uint8_t southwest(uint8_t x, uint8_t y) const {
                assert(has_sw(x,y));
                if (y >= (SIZE-1)) {
                        return x-1;
                }
                return x;
        }

        bool has_e(uint8_t x, uint8_t y) const { return valid(x+1,y); }
        bool has_w(uint8_t x, uint8_t y) const { return x > 0; }

        enum Direction { EAST, WEST, NE, SE, NW, SW };

        Direction reverse(Direction d) const {
                switch (d) {
                case EAST: return WEST;
                case WEST: return EAST;
                case NE: return SW;
                case SE: return NW;
                case NW: return SE;
                case SW: return NE;
                }
                DIE("invalid direction: " << d);
        }

        bool has_move(Direction d, uint8_t x, uint8_t y) const {
                switch (d) {
                case EAST: return has_e(x,y);
                case WEST: return has_w(x,y);
                case NE: return has_ne(x,y);
                case NW: return has_nw(x,y);
                case SE: return has_se(x,y);
                case SW: return has_sw(x,y);
                }
                DIE("invalid direction: " << d);
        }

        bool move(Direction d, uint8_t &x, uint8_t &y) const {
                if (!has_move(d, x, y))
                        return true;

                switch (d) {
                case EAST: ++x; break;
                case WEST: --x; break;
                case NE: x = northeast(x,y), --y; break;
                case NW: x = northwest(x,y), --y; break;
                case SW: x = southwest(x,y), ++y; break;
                case SE: x = southeast(x,y), ++y; break;
                }
                return false;
        }
};

template <typename T, size_t SIZE>
struct Y {
        // triangular number
        enum { AREA = (SIZE*(SIZE+1))>>1 };
        T data[AREA];

        T get(size_t i) { return data[i]; }
        const T& get(size_t i) const { return data[i]; }
        void set(size_t i, const T &value) { data[i] = value; }

        uint8_t triangular(uint8_t n) { return (n*(n+1)) >> 1; }

        uint8_t row(size_t i) {
                return ((size_t) sqrt(8*(double)+1)) >> 1;
        }

        uint8_t col(size_t i, uint8_t row) {
                return i-triangular(row);
        }

        uint8_t col(size_t i) { return col(i, row(i)); }
};

} // namespace board

#endif // BOARD_H
