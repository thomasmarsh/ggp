#ifndef COMMON_H
#define COMMON_H
#pragma once

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cmath>

#include <iostream>
#include <iomanip>
#include <iterator>
#include <sstream>

#include <limits>

extern "C" {
#include <stdint.h>
}

using namespace std;

struct Timer {
        time_t timestamp;
        Timer() : timestamp(time(NULL)) {}
        time_t elapsed() { return time(NULL)-timestamp; }
        void reset() { timestamp = time(NULL); }
};

static Timer TIMER;

#define DEBUG(x)  //cout << x << endl << flush

#define LOG(x)  { cout << '[' << TIMER.elapsed() << "] " << x << endl << flush; }

#define DIE(x)  { cerr << '[' << TIMER.elapsed() << ' ' \
                       << __FILE__ << ':' << __LINE__ << "] " \
                       x << endl << flush; exit(-1); }

#define ASSERT(c,x) if (!(c)) { DIE(x); }

enum Color { NONE=0, BLACK, WHITE };

static inline Color other(Color c) {
        switch (c) {
        case BLACK: return WHITE;
        case WHITE: return BLACK;
        case NONE: return NONE;
        }
        return NONE;
}

const char *ColorStr(Color c) {
        switch (c) {
        case NONE: return "none";
        case BLACK: return "black";
        case WHITE: return "white";
        }
        return "<unknown>";
}


static inline float randf() {
	return float(random()) / float(RAND_MAX);
}

#endif // COMMON_H
