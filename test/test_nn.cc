#include <engine/neural.h>
#include <ui/nn.h>

static const size_t BATCH_SIZE = 10,
                    BOARD_AREA = 10000,
                    MAX_EPOCH = 10;

static const double THRESHOLD     = 0.000001,
                    //LEARNING_RATE = 0.1;
                    LEARNING_RATE = 0.005;
                    //LEARNING_RATE = 0.001;

//#define PATTERN AbsSine
#define PATTERN BoardEval

struct AbsSine {
        typedef FFNet<1,5,1> NN;
        NN net;
        NN::Query query;

        AbsSine() : query(BATCH_SIZE) {}

        void randomize(size_t s) {
                query.input(s, 0) = randf() * (M_PI/2);
                query.target(s, 0) = abs(sin(query.input(s, 0)));
        }

        void debug(double mse) {
                net.fprop(query);
                LOG("mse=" << mse << " t=" << query.target(0,0) << " o=" << query.output(0, 0));
        }
};

struct BoardEval {
        typedef EnsembleFFNet<4,BOARD_AREA,BOARD_AREA/10,1,Sigmoid,Sigmoid> NN;
        NN net;
        NN::Query query;

        BoardEval() : query(BATCH_SIZE) {}

        void randomize(size_t s) {
                size_t cb = random() % BOARD_AREA,
                       cw = (cb > 0) ? (random() % (BOARD_AREA-cb))
                                     : (random() % BOARD_AREA);

                size_t b=cb, w=cw;
                int t = 0;
                while (b != 0) {
                        size_t pos = rand() % BOARD_AREA;
                        if (query.input(s, pos) == 0) {
                                query.input(s,pos) = 1;
                                b--; t++;
                        }
                }
                while (w != 0) {
                        size_t pos = rand() % BOARD_AREA;
                        if (query.input(s, pos) == 0) {
                                query.input(s,pos) = -1;
                                w--; t--;
                        }
                }
                query.target(s, 0) = to_target(t);
                //LOG("t=" << t << " target=" << to_target(t));
        }

        double to_target(double s) {
                return (double(s) / double(BOARD_AREA) + 1.0) / 2.0;
        }

        double to_score(double t) {
                return (t * 2 * BOARD_AREA) - BOARD_AREA;
        }

        void debug(double mse) {
                net.fprop(query);
                double t = to_score(query.target(0,0));
                double o = to_score(query.output(0,0));
                LOG("mse=" << mse << " t=" << t << " o=" << o);
        }
};


template <typename T>
void test() {
        HintonDiagram d(500,500);
        T test;
        double mse;

        while (true) {
                d.update(test.net.net[0].w1, test.net.net[0].w2);
                test.query.input.zeros();
                for (size_t s=0; s < BATCH_SIZE; ++s)
                        test.randomize(s);
                test.query.set_bias();
                mse = test.net.train_loop(test.query, THRESHOLD, LEARNING_RATE, MAX_EPOCH);
                test.debug(mse);
        }
}

int main() {
        test<PATTERN>();
}
