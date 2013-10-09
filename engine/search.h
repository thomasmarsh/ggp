#ifndef SEARCH_H
#define SEARCH_H
#pragma once

#include "common.h"
#include "board.h"

template <size_t SIZE>
struct SquarePathFinder {
        typedef board::Square<Color,SIZE> Board;
        Color color;
        Board *map;
        bool visited[SIZE][SIZE];
        enum Direction { XDIR, YDIR };
        Direction direction;

        // constructor arg seems necessary; clang bug?
        SquarePathFinder(int _dummy) : color(NONE), map(0) {}

        bool isTarget(uint8_t x, uint8_t y) const {
                switch (direction) {
                case YDIR: return y == (SIZE-1); break;
                case XDIR: return x == (SIZE-1); break;
                default: DIE("invalid direction: " << direction);
                }
                return false;
        }

        bool empty(uint8_t x, uint8_t y) { return map->get(x,y) == NONE; }
        bool occupied(uint8_t x, uint8_t y) { return !empty(x,y); }
        bool seen(uint8_t x, uint8_t y) { return visited[x][y]; }
        bool mycolor(uint8_t x, uint8_t y) { return map->get(x,y) == color; }
        bool enemy(uint8_t x, uint8_t y) { return map->get(x,y) == other(color); }

        bool search(uint8_t x, uint8_t y) {
                if (isTarget(x,y))
                        return true;

                visited[x][y] = true;

#define SEARCH(x,y,cond) \
                if ((cond) && !seen(x,y) && mycolor(x,y)) \
                        if (search(x, y)) return true;

                SEARCH(x-1, y, x > 0);
                SEARCH(x+1, y, x < (SIZE-1));
                SEARCH(x, y-1, y > 0);
                SEARCH(x, y+1, y < (SIZE-1));
#undef SEARCH

                return false;
        }

        bool searchEastWest() {
                direction = XDIR;
                for (uint8_t y=0; y < SIZE; ++y)
                        if (!seen(0,y) && mycolor(0,y))
                                if (search(0, y)) return true;
                return false;
        }

        bool searchNorthSouth() {
                direction = YDIR;
                for (uint8_t x=0; x < SIZE; ++x)
                        if (!seen(x,0) && mycolor(x,0))
                                if (search(x, 0)) return true;
                return false;
        }

        void clear(Color c, Board *m) {
                color = c;
                map = m;
                memset(visited, 0, sizeof(bool)*(SIZE*SIZE));
        }

        bool connected(Color c, Board *m) {
                clear(c, m);

                if (color == WHITE) {
                        if (searchEastWest()) return true;
                } else {
                        if (searchNorthSouth()) return true;
                }
                return false;
        }

        size_t length(uint8_t x, uint8_t y) {
                size_t len = 1;
                visited[x][y] = true;

#define SEARCH(x,y,cond) \
                if ((cond) && !seen(x,y) && mycolor(x,y)) \
                        len += length(x,y);

                SEARCH(x-1, y, x > 0);
                SEARCH(x+1, y, x < (SIZE-1));
                SEARCH(x, y-1, y > 0);
                SEARCH(x, y+1, y < (SIZE-1));

#undef SEARCH

                return len;
        }

        size_t longest(Color c, Board *m) {
                clear(c, m);
                size_t best = 0;

                for (size_t x=0; x < SIZE; ++x) {
                        for (size_t y=0; y < SIZE; ++y) {
                                if (!seen(x,y) && mycolor(x,y)) {
                                        size_t len = length(x, y);
                                        if (len > best)
                                                best = len;
                                }
                        }
                }
                return best;
        }

        bool connected_any(Color c, Board *m) {
                clear(c, m);
                if (searchEastWest()) return true;
                if (searchNorthSouth()) return true;
                return false;
        }

        size_t liberty_count(uint8_t x, uint8_t y) {
                visited[x][y] = true;

                if (empty(x,y))
                        return 1;

                if (enemy(x,y))
                        return 0;

                size_t count = 0;

#define SEARCH(x,y,cond) \
                if ((cond) && !seen(x,y)) \
                        count += liberty_count(x, y);

                SEARCH(x-1, y, x > 0);
                SEARCH(x+1, y, x < (SIZE-1));
                SEARCH(x, y-1, y > 0);
                SEARCH(x, y+1, y < (SIZE-1));
#undef SEARCH
                return count;

        }

        size_t liberties(uint8_t x, uint8_t y, Color(*m)[SIZE][SIZE]) {
                clear((*m)[x][y], m);
                return liberty_count(x,y);
        }
};


#endif // SEARCH_H
