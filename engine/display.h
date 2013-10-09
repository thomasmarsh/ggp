#ifndef DISPLAY_H
#define DISPLAY_H
#pragma once

#include "common.h"

#define GL_GLEXT_PROTOTYPES

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif // __APPLE__

#include <list>

void display_func();

struct NullDisplay {
        void addMove(Color color, uint8_t top, Move &m) {}
};

struct Display {
        struct Piece {
                uint8_t x, y, z;
                Type type;
                Direction dir;
                Color color;

                void set(uint8_t _x, uint8_t _y, uint8_t _z, Type _t, Direction d, Color c) {
                        x = _x, y = _y, z = _z, dir = d, color = c;
                }
        };

        list<Piece> MOVES;
        Mutex mutex;

        void addMove(Color color, uint8_t top, Move &m) {
                Piece p;
                p.set(m.x(),
                      m.y(),
                      top,
                      m.type(),
                      p.dir,
                      color);

                { Lock lock(mutex);
                        MOVES.push_back(p);
                }

                glutPostRedisplay();
        }

        void drawPiece(Piece &p) {
                glBegin(GL_POLYGON);
                switch (p.color) {
                case BLACK: glColor3f(0.3, 0.3, 0.3); break;
                case WHITE: glColor3f(1.0, 1.0, 1.0); break;
                default: assert(1==0);
                }

                float fx = ((float) p.x / 9.0) - .5,
                      fy = ((float) p.y / 9.0) - .5,
                      fz = ((float) p.z / 9.0) - .5,
                      d  = 1.0 / 9.0,
                      dx = (p.dir == XDIR && p.type == LINTEL) ? d*3 : 0,
                      dy = (p.dir == YDIR && p.type == LINTEL) ? d*3 : 0;

                glVertex3f( fx-d,  fy-d,  fz-d);
                glVertex3f( fx-d,  fy+dy, fz-d);
                glVertex3f( fx+dx, fy+dy, fy-d);
                glVertex3f( fx+dx, fy-d,  fy-d);
                glEnd();
        }


        void draw() {
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                glLoadIdentity();
                glRotatef(2, 0, 0, 0);

                list<Piece>::iterator i;
                {
                        Lock lock(mutex);
                        for (i = MOVES.begin(); i != MOVES.end(); i++)
                                drawPiece(*i);
                }
                glFlush();
                glutSwapBuffers();
        }

        void initialize(int *argc, char **argv) {
                glutInit(argc, argv);
                glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
                glutCreateWindow("Druid");
                glEnable(GL_DEPTH_TEST);
                glutDisplayFunc(display_func);
                //glutSpecialFunc(specialKeys);

                //pthread_t thread;
                //assert(0==pthread_create(&thread, NULL, test2, NULL));
        }

        void loop() {
                glutMainLoop();
        }
};

Display DISPLAY;

void display_func() { DISPLAY.draw(); }

#endif // DISPLAY_H
