#include "ui.h"

#include <armadillo>

struct HintonDiagram {
        Window window;

        int margin;

        HintonDiagram(int w, int h)
                : window("Hinton Diagram", w, h),
                  margin(10)
        {}

        arma::mat normalize(arma::mat w) {
                arma::rowvec d =
                        arma::max(arma::join_cols(
                                        arma::max(w,0),
                                        arma::abs(arma::min(w,0))), 0);

                w.each_row() /= d;
                return w;
        }

        void render(arma::mat w1, arma::mat w2) {
                double cols = std::max(w1.n_cols, w2.n_rows),
                       rows = w1.n_rows + w2.n_cols + 1,
                       w    = (window.width -  2*margin) / cols,
                       h    = (window.height - 2*margin) / rows,
                       step = std::min(w,h);

                arma::mat n1 = normalize(w1),
                          n2 = normalize(w2);

                draw(n1, n2.t(), step);
        }


        void draw(arma::mat n1, arma::mat n2, double step) {
                draw_one(n2, step, 0);
                draw_one(n1, step, step*(1+n2.n_rows));
        }

        void draw_one(arma::mat n, double step, double offset) {
                double x=margin, y=margin, sx, sy;
                for (size_t c=0; c < n.n_cols; ++c) {
                        for (size_t r=0; r < n.n_rows; ++r) {

                                double value = n(r, c);

                                if (value > 0)
                                        window.color(1.0, 0.0, 0);
                                else window.color(.49, .15, .80);

                                double d = step - step*value;

                                sx = x+d/2,
                                sy = y+offset+d/2;

                                window.square(sx, sy,
                                              step*value, step*value);

                                y += step;
                        }
                        y = margin;
                        x += step;
                }
        }

        void update(arma::mat w1, arma::mat w2) {
                window.clear();
                render(w1, w2);
                window.swap();
                window.check_running();
                if (!window.running) {
                        window.close();
                        exit(EXIT_SUCCESS);
                }
        }
};
