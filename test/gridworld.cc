#include <engine/common.h>
#include <bitset>
#include <cstring>
#include <cstdio>

using namespace std;

static const size_t WIDTH  = 4,
                    HEIGHT = 3,
                    AREA   = WIDTH*HEIGHT,
                    NORTH  = 0,
                    SOUTH  = 1,
                    EAST   = 2,
                    WEST   = 3;

static const double ROBOT_PROB[3]  = { 0.1, 0.8, 0.1 }, // left, straight, right
                    DEFAULT_REWARD = -0.02,
                    GAMMA          = 0.99;

struct GridWorld {
        bitset<AREA> wall, fixed_reward;
        double vs[AREA]; // calculated V(s)
        double rs[AREA]; // R(s)

        GridWorld() {
                memset(rs, 0, sizeof(vs));
                memset(vs, 0, sizeof(vs));
                for (size_t i=0; i < AREA; ++i) {
                        vs[i] = 0;
                        rs[i] = DEFAULT_REWARD;
                }
        }

        size_t index(int x, int y) { return x + y*WIDTH; }

        int x(size_t i) { return i % WIDTH; }
        int y(size_t i) { return i / WIDTH; }

        int cx(int x) {
                if (x < 0) return 0;
                else if (x > (WIDTH-1)) return WIDTH-1;
                return x;
        }

        int cy(int y) {
                if (y < 0) return 0;
                else if (y > (HEIGHT-1)) return HEIGHT-1;
                return y;
        }

        int wi(size_t i, size_t ip) {
                if (wall[ip])
                        return i;
                return ip;
        }

        size_t west(size_t i) { return wi(i, index(cx(x(i)-1), y(i))); }
        size_t east(size_t i) { return wi(i, index(cx(x(i)+1), y(i))); }
        size_t south(size_t i)  { return wi(i, index(x(i), cy(y(i)+1))); }
        size_t north(size_t i)  { return wi(i, index(x(i), cy(y(i)-1))); }

        void set_wall(bool w, int x, int y) {
                wall[index(x,y)] = w;
                vs[index(x,y)] = 0;
        }

        void set_reward(double r, int x, int y) {
                vs[index(x,y)] = r;
                rs[index(x,y)] = r;
                fixed_reward[index(x,y)] = true;
        }

        double av(size_t i, size_t dir) {
                switch (dir) {
                case NORTH:
                        return ROBOT_PROB[0] * vs[west(i)] +
                               ROBOT_PROB[1] * vs[north(i)] +
                               ROBOT_PROB[2] * vs[east(i)];
                case SOUTH:
                        return ROBOT_PROB[0] * vs[west(i)] +
                               ROBOT_PROB[1] * vs[south(i)] +
                               ROBOT_PROB[2] * vs[east(i)];
                case EAST:
                        return ROBOT_PROB[0] * vs[north(i)] +
                               ROBOT_PROB[1] * vs[east(i)] +
                               ROBOT_PROB[2] * vs[south(i)];
                case WEST:
                        return ROBOT_PROB[0] * vs[north(i)] +
                               ROBOT_PROB[1] * vs[west(i)] +
                               ROBOT_PROB[2] * vs[south(i)];
                }
        }

        double best_action_value(size_t i) {
                double best = -1.0, v;;
                for (size_t d=0; d < 4; ++d) {
                        v = av(i, d);
                        if (v > best)
                                best = v;
                }
                return best;
        }

        void value_iterate() {
                // for all s, let V(s) := R(S) + max(a) gamma * sum(Psa(s')V(s'))
                for (size_t i=0; i < AREA; ++i) {
                        if (!wall[i] && !fixed_reward[i]) {
                                vs[i] = rs[i] + GAMMA * best_action_value(i);
                        }
                }
        }

        void print_v() {
                for (int y=0; y < HEIGHT; ++y) {
                        for (int x=0; x < WIDTH; ++x)
                                printf("%0.2f  ", vs[index(x,y)]);
                        printf("\n");
                }
                printf("\n");
        }

        bool valid_dir(size_t i, size_t dir) {
                switch (dir) {
                case NORTH: return north(i) != i;
                case SOUTH: return south(i) != i;
                case EAST: return east(i) != i;
                case WEST: return west(i) != i;
                }
                return false;
        }

        // pi*(s) = argmax_a sum(Psa(s') V*(s'))
        size_t pi_star(size_t i) {
                double max = -100.0, v;
                size_t best=5;
                for (size_t d=0; d < 4; ++d) {
                        if (valid_dir(i, d)) {
                                v = av(i, d);
                                if (v > max) {
                                        max = v;
                                        best = d;
                                }
                        }
                }
                return best;
        }

        void print_pi() {
                size_t ps = 0;
                size_t i;
                for (int y=0; y < HEIGHT; ++y) {
                        for (int x=0; x < WIDTH; ++x) {
                                i = index(x,y);
                                ps = pi_star(i);
                                char c = '-';

                                switch (ps) {
                                case NORTH: c = '^'; break;
                                case SOUTH: c = 'v'; break;
                                case EAST: c = '>'; break;
                                case WEST: c = '<'; break;
                                }

                                if (wall[i] || fixed_reward[i])
                                        c = '-';
                                printf("%c ", c);
                        }
                        printf("\n");
                }
                printf("\n");
        }

};

int main() {
        GridWorld w;

        w.set_wall(true, 1, 1);

        w.set_reward( 1.0, 3, 0);
        w.set_reward(-1.0, 3, 1);

        w.print_v();

        for (size_t k=0; k < 12; ++k) {
                LOG("k = " << k);
                w.value_iterate();
                w.print_v();
                w.print_pi();
        }
}
