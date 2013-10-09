#include <engine/board.h>

static const size_t SIZE=5;

typedef board::Hex<uint8_t,SIZE> Board;

static const size_t NUM_DIR = 6;

const string dirname(Board::Direction d) {
        switch (d) {
        case Board::EAST: return "EAST";
        case Board::WEST: return "WEST";
        case Board::NE: return "NE";
        case Board::SE: return "SE";
        case Board::NW: return "NW";
        case Board::SW: return "SW";
        }
        return "<none>";
}

void test_point(Board &b, uint8_t x, uint8_t y) {
        cout << "Point: " << b.label(x,y) << " (" << (int)x << ", " << (int)y << ')' << endl;
        for (size_t i=0; i < NUM_DIR; ++i) {
                uint8_t nx=x, ny=y;
                Board::Direction d = (Board::Direction) i;
                bool error = b.move(d, nx, ny);
                cout << dirname(d) << ": ";
                if (error) cout << "N/A";
                else cout << b.label(nx, ny) << " (" << (int)x << ", " << (int)y << ')';
                cout << endl;
        }
        cout << endl << flush;
}

void random_step(Board &b, uint8_t &x, uint8_t &y) {
        Board::Direction d = (Board::Direction) (random() % NUM_DIR);

        cout << b.label(x,y)
             << "("<< (int)x << ", " << (int)y << ')'
             << " --> " << dirname(d) << " --> ";

        bool error = b.move(d, x, y);

        if (error) cout << "N/A" << endl;
        else 
                cout << b.label(x,y)
                     << "("<< (int)x << ", " << (int)y << ')' << endl << flush;
}

int main() {
        srandomdev();
        Board hex;

        for (uint8_t y=0; y < Board::DSIZE; ++y) {
                for (size_t i=0; i < size_t(Board::DSIZE-hex.width(y)); ++i)
                        cout << "  ";
                for (uint8_t x=0; x < Board::DSIZE; ++x) {
                        if (hex.valid(x,y))
                                cout << hex.label(x,y) << "  ";
                }
                cout << endl;
        }

        uint8_t x = SIZE-1,
                y = SIZE-1;

#if 0
        for (uint8_t y=0; y < Board::DSIZE; ++y) {
                for (uint8_t x=0; x < Board::DSIZE; ++x) {
                        if (hex.valid(x,y))
                                test_point(hex, x,y);
                }
        }
#endif
        LOG("start: " << hex.label(x,y));
        for (size_t i=0; i < 3; ++i) {
                random_step(hex, x,y);
        }

        LOG("area: " << Board::AREA);
        LOG("sizeof(hex) == " << sizeof(hex));
}
