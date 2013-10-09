#include <engine/rbm.h>
#include <ui/nn.h>

static const size_t NUM_VISIBLE = 64*2,
                    NUM_HIDDEN  = NUM_VISIBLE,
                    NUM_SAMPLES = 5,
                    NUM_DREAMS  = 10000;

static const float LEARNING_RATE = .1;

arma::mat rand_ones(size_t rows, size_t cols) {
        return arma::conv_to<arma::mat>::from(
                        (arma::ones(rows,cols)*.5) > arma::randu(rows, cols));
}

#define ROW(a,b,c,d,e,f) a << b << c << d << e << f << arma::endr

arma::mat test_data6() {
        assert(NUM_VISIBLE == 6);

        arma::mat m;

        m << ROW(1,1,1,0,0,0)
          << ROW(1,0,1,0,0,0)
          << ROW(1,1,1,0,0,0)
          << ROW(0,0,1,1,1,0)
          << ROW(0,0,1,1,0,0)
          << ROW(0,0,1,1,1,0);

        return m;
}

HintonDiagram hinton(500, 500);

void test6() {
        RBM rbm(6, 2, 0.1);

        arma::mat a = test_data6();
        for (size_t i=0; i < 500; ++i) {
                rbm.train(a, 10, true);
                hinton.update(rbm.weights, rbm.weights);
        }

        arma::mat user, out;
        user << ROW(0,0,0,1,1,0);
        out = rbm.run_visible(user);
        rbm.weights.print("weights:");
        user.print("user:");
        out.print("out:");
}


arma::mat rand_board() {
        arma::mat m = arma::zeros(NUM_SAMPLES, NUM_VISIBLE);

        for (size_t s=0; s < NUM_SAMPLES; ++s) {
                size_t turn = random() % NUM_VISIBLE;

                bool is_black = true;
                for (size_t i=0; i < (turn+1); ++i) {
                        size_t pos;
                        do {
                                pos = random() % (NUM_VISIBLE/2);
                        } while (m(s, pos*2) != 0 || m(s, pos*2+1) != 0);

                        if (is_black) m(s, pos) = 1;
                        else m(s, pos+1) = 1;
                        is_black = !is_black;
                }
        }
        return m;
}


void test_rbm() {
        RBM rbm(NUM_VISIBLE, NUM_HIDDEN, LEARNING_RATE);

        arma::mat a = rand_board();

        LOG("TRAIN");
        for (size_t i=0; i < 50; ++i) {
                rbm.train(a, 100, true);
                hinton.update(rbm.weights, rbm.weights);
        }

        LOG("RUN_VISIBLE");
        arma::mat b = rbm.run_visible(a);

        LOG("RUN_HIDDEN");
        arma::mat c = rbm.run_hidden(b);

        LOG("RUN_DAYDREAM");
        arma::mat d = rbm.daydream(NUM_DREAMS);
        LOG("done.");

        a.print("a:");
        b.print("b:");
        c.print("c:");
        //d.print("d:");
}

int main() {
        srand(time(NULL));
        test_rbm();
}
