#include "ui.h"

#include <vector>

static const double SQRT_3 = 1.73205080757;

struct GBoard {
        Window window;

        enum Type { GO, HEXHEX, SQUAREHEX, SQUAREHEX_ROT };

        typedef std::vector<size_t> player_t;

        double stone_size, bsize;

        GBoard(int w, int h, double size)
                : window("Game Board", w, h),
                  stone_size(10),
                  bsize(size)
        {
                window.clear_color(1, 1, 1);
        }

        void set_stone_size() {
                stone_size = std::min(window.height,window.width) / (bsize+3);
        }

        void boardcolor() { window.color(.95, .69, .43); }
        void lightgrey() { window.color(.8, .8, .8); }
        void black() { window.color(0, 0, 0); }
        void white() { window.color(1, 1, 1); }

        void draw_square_board() {
                set_stone_size();
                boardcolor();
                double a=stone_size, b=stone_size*(bsize+1);
                window.square(a, a, b, b);
                window.line_width(2);
                black();
                glBegin(GL_LINE_LOOP);
                glVertex2f(a,   a);
                glVertex2f(a+b, a);
                glVertex2f(a+b, a+b);
                glVertex2f(a,   a+b);
                glEnd();
                window.line_width(1);

                double sx = 0,
                       sy = 0,
                       ax = stone_size*2,
                       ay = stone_size*2,
                       bx = stone_size*2 + stone_size*(bsize-1),
                       by = stone_size*2 + stone_size*(bsize-1);

                for (size_t x=0; x < bsize; ++x) {
                        for (size_t y=0; y < bsize; ++y) {
                                sx = stone_size*2 + stone_size*x,
                                sy = stone_size*2 + stone_size*y;

                                window.line(ax, sy, bx, sy);
                                window.line(sx, ay, sx, by);
                        }
                }
        }

        void circles(player_t c, float r, bool fill=true) {
                double ax = stone_size*2,
                       ay = stone_size*2;

                for (size_t i=0; i < c.size(); ++i) {
                        double x = c[i] % (int) bsize,
                               y = c[i] / (int) bsize;
                        window.circle(ax+x*stone_size, ay+y*stone_size, r, 30, fill);
                }
        }

        void draw_go_stones(player_t &b, player_t &w) {
                white();
                circles(w, stone_size/2-.9);

                black();

                window.line_width(2);
                circles(w, stone_size/2-.8, false);
                window.line_width(1);

                circles(b, stone_size/2);
        }


        double hex_x(int x, int y, double r, bool rotated) {
                if (!rotated)
                        return double(x)*SQRT_3*r + ((y%2)==1 ? SQRT_3*r/2 : 0);
                return double(x)*3.0*r/2.0;
        }

        double hex_y(int x, int y, double r, bool rotated) {
                if (!rotated)
                        return double(y)*3.0*r/2.0;
                return double(y)*SQRT_3*r + ((x%2)==1 ? SQRT_3*r/2 : 0);
        }

        void set_hex_stone_size(bool rotated) {
                double horizontal=0,
                       vertical=0;
                if (!rotated) {
                        horizontal = window.width  / (SQRT_3*bsize/2+SQRT_3/2+2);
                        vertical   = window.height / (bsize*.75+2.25);
                } else {
                        horizontal = window.width  / (bsize*.75+2.25);
                        vertical   = window.height / (SQRT_3*bsize/2+SQRT_3/2+2);
                }

                stone_size = std::min(horizontal, vertical);
        }

        void draw_squarehex_board(bool rotated) {
                set_hex_stone_size(rotated);

                double r  = stone_size/2,
                       R  = 2*r/SQRT_3,
                       ax = stone_size,
                       ay = ax;

                black();

                for (size_t y=0; y < bsize; ++y) {
                        for (size_t x=0; x < bsize; ++x) {
                                boardcolor();
                                window.circle(hex_x(x, y, R, rotated) + ax,
                                              hex_y(x, y, R, rotated) + ay,
                                              R,
                                              6, true, rotated);
                                black();
                                window.circle(hex_x(x, y, R, rotated) + ax,
                                              hex_y(x, y, R, rotated) + ay,
                                              R,
                                              6, false, rotated);
                        }
                }
        }

        void hex_circles(player_t c, float r, bool fill, bool rotated) {
                double ax = stone_size,
                       ay = ax,
                       R  = stone_size/SQRT_3;

                for (size_t i=0; i < c.size(); ++i) {
                        double x = c[i] % (int) bsize,
                               y = c[i] / (int) bsize;
                        window.circle(ax+hex_x(x,y,R, rotated), ay+hex_y(x,y,R, rotated), r*.85, 30, fill);
                }
        }

        void draw_hex_stones(player_t &b, player_t &w, bool rotated) {
                white();
                hex_circles(w, stone_size/2-.9, true, rotated);

                black();

                window.line_width(2);
                hex_circles(w, stone_size/2-.8, false, rotated);
                window.line_width(1);

                hex_circles(b, stone_size/2, true, rotated);
        }

        void update(Type t, player_t &b, player_t &w) {
                window.clear();

                switch (t) {
                case GO:
                        draw_square_board();
                        draw_go_stones(b, w);
                        break;
                case HEXHEX:
                        //draw_hexhex_board();
                        //draw_hex_stones(b, w);
                        break;
                case SQUAREHEX:
                        draw_squarehex_board(false);
                        draw_hex_stones(b, w, false);
                        break;
                case SQUAREHEX_ROT:
                        draw_squarehex_board(true);
                        draw_hex_stones(b, w, true);
                        break;
                }

                window.swap();
                window.check_running();
                if (!window.running) {
                        window.close();
                        exit(EXIT_SUCCESS);
                }
        }
};
