#include <engine/common.h>

#include <vector>
#include <bitset>

struct Tetris {
        enum Piece { Empty, I, J, L, O, S, T, Z };
        size_t width, height;
        size_t score, pile_height;
        Piece current, next;
        std::vector<Piece> piece_matrix;


        Tetris(size_t w, size_t h)
                : width(w),
                  height(h),
                  score(0),
                  pile_height(0),
                  current(rand_piece()),
                  next(rand_piece()),
                  piece_matrix(w*h, Empty)
        {}

        Piece rand_piece() { return (Piece) (1+(random() % T)); }

        void set_color(Piece p) {
                switch (p) {
                case Empty: break; // black
                case I: break; // cyan
                case J: break; // blue
                case L: break; // orange
                case O: break; // yellow
                case S: break; // green
                case T: break; // purple
                case Z: break; // red
                }
        }

        size_t rotations(Piece p) {
                switch (p) {
                case Empty: return 0; // not reached
                case O: return 1;
                case I: return 2;
                case J: case L: case S: case T: case Z:
                        return 4;
                }
                return 0;
        }
};
