#ifndef GGP_UI_PLOT_H
#define GGP_UI_PLOT_H

#include "ui.h"

struct Plot {
        Window window;

        struct Point { Point(double a, double b) : x(a), y(b) {} double x, y; };

        Point max;
        Point min;

        Plot(int w, int h, double mx, double nx, double my, double ny)
                : window("Plot", w, h),
                  max(mx, my),
                  min(nx, ny)
        {}

        double sy(double y) { return (y-min.y) * (double(window.height) / (max.y-min.y)); }
        double sx(double x) { return (x-min.x) * (double(window.width) / (max.x-min.x)); }

        void color(float r, float g, float b) { window.color(r,g,b); }

        void draw(pair<double,double> &a, pair<double,double> &b) {
                window.line(sx(a.first), sy(a.second), sx(b.first), sy(b.second));
        }

        void render(vector< pair<double,double> > &v) {
                for (size_t i=1; i < v.size(); ++i)
                        draw(v[i-1], v[i]);
        }

        void update(vector< pair<double,double> > &v) {
                window.clear();
                render(v);
                window.swap();
                window.check_running();
                if (!window.running) {
                        window.close();
                        exit(EXIT_SUCCESS);
                }
        }
};

#endif // GGP_UI_PLOT_H
