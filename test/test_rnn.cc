#include <engine/rnn.h>
#include <engine/genetic.h>
#include <ui/nn.h>

#include <set>

// ESP logic
//
//  for n hidden neurons:
//
//  1. create n population pools
//
//  2. select random neuron from each pool to create a network; score is added
//     to cumulative fitness for each participating neuron
//
//  3. goto to, until each neuron has reached an average of e.g., 10 trials
//
//  4. average fitness (by number of trials that neuron participated in)
//
//  5. rank by average fitness
//
//  6. crossover top quartile with higher ranked neurons, offspring replace
//     lowest half of population
//
//  7. mutate probability 0.2 (replace one randomly chosen weight)
//
//  8. goto 2



static const size_t NUM_INPUT  = 1,
                    NUM_HIDDEN = 8,
                    NUM_OUTPUT = 1,
                    
                    RUNLEN = 50,
                    THRESHOLD = 40;

typedef RNN<NUM_INPUT, NUM_HIDDEN, NUM_OUTPUT, Tanh, Tanh> NN;
typedef NN::Query Q;

// arithmetic crossover
void crossover(arma::mat P1, arma::mat P2, arma::mat &O1, arma::mat &O2) {
        double a = (randf() + 1.0) / 2;

        O1 = a*P1 + (1-a)*P2;
        O2 = (1-a)*P1 + a*P2;
}

double myround(double x, size_t n) {
        return round(x * pow(10, double(n))) / pow(10, double(n));
}


static const double WEIGHT_RANGE = 0.6;

struct NodePolicy {
        struct Neuron {
                Neuron(bool n=false) : null(n), score(0) {}

                bool null;
                int score;

                arma::mat w_in, w_out;

                void reset() {
                        w_in.resize(NUM_INPUT+NUM_HIDDEN+1, 1);
                        w_out.resize(1, NUM_HIDDEN+NUM_OUTPUT);

                        w_in.randn();
                        w_out.randn();

                        w_in  /= w_in.max();
                        w_out /= w_out.max();

                        w_in  *= 2*WEIGHT_RANGE; 
                        w_out *= 2*WEIGHT_RANGE;

                        w_in  -= WEIGHT_RANGE;
                        w_out -= WEIGHT_RANGE;
                }

                bool operator == (const Neuron &rhs) const {
                        return null == rhs.null;
                }

        };


        struct TestNet {
                NN net;
                size_t indices[NUM_HIDDEN+1];

                void set_neuron(size_t i, Neuron &n) {
                        net.w1 = arma::join_rows(net.w1, n.w_in);
                        net.w2 = arma::join_cols(net.w2, n.w_out);
                }
        };

        static Neuron null() { return Neuron(true); }

        static void randomize(Neuron &n, size_t pool) {
                n.reset();
        }

        static void set_pool(Neuron &n, size_t pool) {}
        static void release(Neuron &n) {}

        static bool compare(const Neuron &a, const Neuron &b, size_t pool) {
                return a.score > b.score;
        }

        static void iterate(Pool<Neuron,NodePolicy> &p, Pool<Neuron,NodePolicy> &t, size_t pool) {
        }
};


typedef Population<NodePolicy::Neuron, NodePolicy> Pop;

void search() {
        Pop pop(20, NUM_HIDDEN+1);
        pop.iterate();
}

int main() {
        srand(time(NULL));

        HintonDiagram hinton(400, 400);
        NN rnn;

        Q q(1);
        q.randomize();
        size_t i=0;
        std::set<double> out;
        std::vector<double> ov;
        while (true) {
                if ((i % RUNLEN) == 0) {
                        if (out.size() > THRESHOLD) {
                                hinton.update(rnn.w1, rnn.w2);
                                LOG("out.size() = " << out.size());
                                for (size_t j=0; j < ov.size(); ++j) {
                                        LOG(" - " << ov[j]);
                                }
                        }
                        q.randomize();
                        rnn.reset();
                        out.erase(out.begin(), out.end());
                        ov.erase(ov.begin(), ov.end());
                }
                rnn.fprop(q);
                out.insert(myround(q.output(0,0), 3));
                ov.push_back(myround(q.output(0,0), 3));
                ++i;
        }
}
