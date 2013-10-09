#include <engine/common.h>
#include <bitset>
#include <cstring>
#include <cstdio>

using namespace std;

static const size_t WIDTH  = 110,
                    HEIGHT = 50,
                    AREA   = WIDTH*HEIGHT,
                    NORTH  = 0,
                    SOUTH  = 1,
                    EAST   = 2,
                    WEST   = 3;

static const double ROBOT_PROB[3]  = { 0.1, 0.8, 0.1 }, // left, straight, right
                    DEFAULT_REWARD = -0.02,
                    GAMMA          = 0.99,      // discount factor
                    EPSILON        = 0.0000000, // stopping error threshold
                    BIG_WIN        =  10,
                    BIG_LOSS       = -10;

struct GridWorld {
        bitset<AREA> wall, fixed_reward;
        double vs[AREA]; // calculated V(s)
        double rs[AREA]; // R(s)

        GridWorld() {
                memset(rs, 0, sizeof(vs));
                memset(vs, 0, sizeof(vs));
                wall.reset();
                fixed_reward.reset();
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
                else if ((size_t) x > (WIDTH-1)) return WIDTH-1;
                return x;
        }

        int cy(int y) {
                if (y < 0) return 0;
                else if ((size_t) y > (HEIGHT-1)) return HEIGHT-1;
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
                return 0;
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

        double value_iterate() {
                double e = 0, n;
                // for all s, let V(s) := R(S) + max(a) gamma * sum(Psa(s')V(s'))
                for (size_t i=0; i < AREA; ++i) {
                        if (!wall[i] && !fixed_reward[i]) {
                                n = rs[i] + GAMMA * best_action_value(i);
                                e += (n-vs[i])*(n-vs[i]);
                                vs[i] = n;
                        }
                }
                return e;
        }

        void print_v() {
                for (int y=0; (size_t) y < HEIGHT; ++y) {
                        for (int x=0; (size_t) x < WIDTH; ++x)
                                printf("%0.2f  ", vs[index(x,y)]);
                        printf("\n");
                }
                printf("\n");
        }

        void print_r() {
                for (int y=0; (size_t) y < HEIGHT; ++y) {
                        for (int x=0; (size_t) x < WIDTH; ++x) {
                                if (fixed_reward[index(x,y)]) {
                                        printf("%0.2f* ", rs[index(x,y)]);
                                } else {
                                        printf("%0.2f  ", rs[index(x,y)]);
                                }
                        }
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
                double max = BIG_LOSS, v;
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
                for (int y=0; (size_t) y < HEIGHT; ++y) {
                        for (int x=0; (size_t) x < WIDTH; ++x) {
                                i = index(x,y);
                                ps = pi_star(i);
                                char c = '-';

                                switch (ps) {
                                case NORTH: c = '^'; break;
                                case SOUTH: c = 'v'; break;
                                case EAST: c = '>'; break;
                                case WEST: c = '<'; break;
                                }

                                if (wall[i]) c = ' ';
                                if (fixed_reward[i]) c = (rs[i] == 0 ? '0' : (rs[i] > 0 ? '+' : '-'));
                                printf("%c ", c);
                        }
                        printf("\n");
                }
                printf("\n");
        }

};

int main() {
        GridWorld w;
        srandomdev();

        for (size_t i=1; i < AREA; ++i) {
                if (random() % 4 == 0)
                        w.set_wall(true, w.x(i), w.y(i));
                else if ((random() % 10) == 0) {
                        w.rs[i] = ((randf()-1.0)/2.0) * (randf()+1.0)*2;
                }
        }
        for (size_t i=0; i < random() % (AREA/10); ++i) {
                size_t c = random() % AREA;
                w.wall[c] = false;
                w.set_reward(BIG_LOSS, w.x(c), w.y(c));
        }

        size_t x = random() % AREA;
        w.set_reward(BIG_WIN, w.x(x), w.y(x));
        w.wall[x] = false;

        w.print_r();
        w.print_v();
        w.print_pi();

        double e = 1.0;
        size_t k=0;
        while (e > EPSILON) {
                e = w.value_iterate();
                LOG("k=" << k << " e=" << e);
                //w.print_v();
                w.print_pi();
                ++k;
        }
}
