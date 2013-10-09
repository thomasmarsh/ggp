#ifndef RBM_TTT_H
#define RBM_TTT_H

#include <engine/common.h>

#include <armadillo>

struct TTTState {
        arma::mat state;
        size_t size, psize, positions, board_size;

        TTTState(size_t s, size_t b)
                : size(s),
                  psize(size-2),
                  positions(psize/2),
                  board_size(b)
        {
                state = arma::zeros(1, size);
        }

        size_t rand_move() {
                size_t pos=0;
                do {
                        pos = random() % positions;
                } while (occupied(pos));
                return pos;
        }

        void set_color(bool black) {
                state(0, black ? 0 : 1) = 1;
                state(0, black ? 1 : 0) = 0;
        }

        void place(bool black, size_t i) {
                state(0, cpos(black, i)) = 1;
        }

        bool three_in_row(arma::mat v) {
                if (arma::sum(v.diag()) == board_size
                 || arma::sum(v.diag(1)) == board_size)
                        return true; 

                for (size_t i=0; i < board_size; ++i) {
                        if (arma::sum(v.col(i)) == board_size
                         || arma::sum(v.row(i)) == board_size)
                                return true;
                }
                return false;
        }

        size_t cpos(bool black, size_t i) { return black ? bpos(i) : wpos(i); }
        size_t bpos(size_t i) { return 2+i*2; }
        size_t wpos(size_t i) { return 3+i*2; }

        arma::mat to_mat(size_t offset) {
                arma::mat m = arma::zeros(1, positions);
                for (size_t i=0; i < positions; ++i)
                        m(0, i) = state[2+offset+i*2];
                return reshape(m, board_size, board_size);
        }

        arma::mat black_mat() { return to_mat(0); }
        arma::mat white_mat() { return to_mat(1); }

        bool win() { return three_in_row(black_mat()) || three_in_row(white_mat()); }

        bool occupied(size_t pos) { return state(bpos(pos)) || state(wpos(pos)); }
        bool is_black(size_t pos) { return state(bpos(pos)); }
        bool is_white(size_t pos) { return state(wpos(pos)); }

        void print() {
                for (size_t i=0; i < positions; ++i) {
                        if (occupied(i)) {
                                cout << (is_black(i) ? "X" : "O");
                        } else cout << "-";
                        if (((i+1) % board_size) == 0)
                                cout << endl;
                }
        }
};

#endif // RBM_TTT_H
