#include <ui/board.h>

static const size_t BOARD_AREA = 5;

void randomvec(size_t size, GBoard::player_t &b, GBoard::player_t &w) {
        for (size_t i=0; i < size; ++i) {
                size_t r = random() % 3;
                if (r == 1) b.push_back(i);
                if (r == 2) w.push_back(i);
        }
}


void testsquare() {
        GBoard d(1000,1000, 9);

        std::vector<size_t> b, w;
        randomvec(81, b, w);

        while (true) {
                d.update(GBoard::GO, b, w);
        }
}

void testsquarehex() {
        GBoard d(400,400, 9);
        std::vector<size_t> b, w;

        //randomvec(81, b, w);
        b.push_back(0);
        w.push_back(1);
        while (true) {
                d.update(GBoard::SQUAREHEX_ROT, b, w);
        }
}

int main() {
        testsquarehex();
        //testsquare();
}
