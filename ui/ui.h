#ifndef GGP_UI_H
#define GGP_UI_H

#include "../engine/common.h"

#include <GL/glfw.h>

// temporary

struct Window;
static Window *WINDOW_INSTANCE=0;

struct Window {
        static void size_cb(int w, int h) { WINDOW_INSTANCE->resize(w, h); }


        bool running;
        int height, width;

        Window() : running(true) {
                init();
        }

        Window(const char *title, size_t w, size_t h)
                : running(true)
        {
                init();
                open(title, w, h);
        }

        void init() {
                if (!glfwInit()) {
                        error("could not initialize GLFW");
                        exit(EXIT_FAILURE);
                }
                WINDOW_INSTANCE = this;
        }

        void open(const char *title, size_t w, size_t h) {
                glfwSetWindowTitle(title);
                height = h, width = w;
                glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
                if (!glfwOpenWindow(h, w,
                                    0, 0, 0, // R/G/B bits
                                    0, 0, 0, // alpha/depth/stencil
                                    GLFW_WINDOW))
                        die("could not open window");
                glfwSetWindowSizeCallback(Window::size_cb);

                glEnable(GL_LINE_SMOOTH);
                glEnable(GL_POLYGON_SMOOTH);
                glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
                glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        void clear_color(float r, float g, float b) {
                glClearColor(r, g, b, 0);
        }

        void line_width(float w) {
                glLineWidth(w);
        }


        void circle(float cx, float cy, float r, size_t num_segments, bool fill, bool rotate=false) { 
                float theta = 2 * 3.1415926 / float(num_segments); 
                float c = cosf(theta); //precalculate the sine and cosine
                float s = sinf(theta);
                float t;

                float x = 0; //we start at angle = 0 
                float y = r;

                if (rotate) {
                        x = r;
                        y = 0;
                }

                if (fill) {
                        glBegin(GL_TRIANGLE_FAN);
                        glVertex2f(cx, height-cy);
                        ++num_segments;
                } else glBegin(GL_LINE_LOOP); 

                for (size_t i=0; i < num_segments; ++i) { 
                        glVertex2f(x+cx, height-(y+cy)); // output vertex 

                        // apply the rotation matrix
                        t = x;
                        x = c * x - s * y;
                        y = s * t + c * y;
                } 
                glEnd(); 
        }
                        
        void color(float r, float g, float b) { glColor3f(r, g, b); }

        void square(float x1, float y1, float w, float h) {
                glBegin(GL_QUADS);
                glVertex2f(x1, height-y1);
                glVertex2f(x1+w, height-y1);
                glVertex2f(x1+w, height-(y1+h));
                glVertex2f(x1, height-(y1+h));
                glEnd();
        }

        void line(float x1, float y1, float x2, float y2) {
                glBegin(GL_LINES);
                glVertex2f(x1, height-y1);
                glVertex2f(x2, height-y2);
                glEnd();
        }

        void resize(size_t w, size_t h) {
                height = h, width = w;
                glViewport(0, 0, width, height);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluOrtho2D(0, width, 0, height);
        }

        template <typename T>
        void loop(T &r) {
                while (running) {
                        clear();
                        r.render(*this);
                        swap();
                        check_running();
                }
        }

        void check_running() {
                running = !glfwGetKey(GLFW_KEY_ESC) &&
                           glfwGetWindowParam(GLFW_OPENED);
        }

        void clear() { glClear(GL_COLOR_BUFFER_BIT); }
        void swap() { glfwSwapBuffers(); }
        void close() { glfwTerminate(); }

        void die(const std::string &msg) {
                error(msg);
                close();
                exit(EXIT_FAILURE);
        }

        void error(const std::string &s) {
                std::cerr << s << std::endl;
        }
};

#endif // GGP_UI_H
