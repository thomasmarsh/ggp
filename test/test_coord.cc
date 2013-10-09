#include <engine/board.h>

int main() {
        board::TriHexLookup tl;

        tl.init(1000);
        size_t x=0, y=0;
        for (size_t i=0; i < 1000; ++i) {
                tl.coord(i, x, y);
                LOG(i << " -> (" << x << ", " << y << "); " <<
                    "(" << x << ", " << y << ") -> " << tl.index(x,y));
        }
}
