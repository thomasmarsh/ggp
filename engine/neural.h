#ifndef GGP_NEURAL_H
#define GGP_NEURAL_H

#include "common.h"
#include "thread.h"

#include <armadillo>

#include <cstdio>
#include <vector>

//-----------------------------------------------------------------------------------
//
// Utility
// 
//-----------------------------------------------------------------------------------

static inline double cross_entropy(double y, double t) {
	return -t * log(y) - (1 - t) * log(1 - y);
}

//-----------------------------------------------------------------------------------
//
// activationation Functions
// 
//-----------------------------------------------------------------------------------

struct Sigmoid {
	static inline arma::mat activation(arma::mat x) {
		return 1/(1+exp(-x));
	}

	static inline arma::mat derivative(arma::mat x) {
		return x%(1-x);
	}
};

struct Tanh {
	static inline arma::mat activation(arma::mat x) {
                //return 2 / (1 + arma::exp(-2 * x)) - 1;
                return arma::tanh(x);
	}

	static inline arma::mat derivative(arma::mat x) {
                return 1-arma::square(x);
	}
};

struct Softplus {
	static inline arma::mat activation(arma::mat x) {
                return arma::log(1+exp(x));
	}

	static inline arma::mat derivative(arma::mat x) {
		return 1/(1+exp(-x));
	}
};

struct Linear {
	static inline arma::mat activation(arma::mat x) { return x; }
	static inline arma::mat derivative(arma::mat x) { return x.ones(); }
};


//-----------------------------------------------------------------------------------
//
// Class FFNet
// 
//-----------------------------------------------------------------------------------

template <size_t NUM_INPUT,
	  size_t NUM_HIDDEN,
	  size_t NUM_OUTPUT,
          typename FH=Sigmoid,
          typename FO=Sigmoid>
class FFNet {
public:
	//---------------------------------------------------------------------------
	//
	// Data
	// 
	//---------------------------------------------------------------------------

        struct Query {
                Query() {}
                Query(size_t size)
                        : input(size, NUM_INPUT+1), target(size, NUM_OUTPUT)
                {}

                void set_bias() {
                        input.col(input.n_cols-1) = arma::ones(input.n_rows, 1);
                }

                void randomize() {
                        input.randn();
                        set_bias();
                }

                arma::mat input, hidden, output, target;
        };


        double momentum;

        arma::mat w1, // weights
                  w2,
                  dw1_last, // previous weight changes
                  dw2_last;


	//---------------------------------------------------------------------------
	//
	// Initialization
	// 
	//---------------------------------------------------------------------------

        FFNet() : momentum(0.3),
                  w1(NUM_INPUT+1, NUM_HIDDEN),
                  w2(NUM_HIDDEN+1, NUM_OUTPUT),
                  dw1_last(NUM_INPUT+1, NUM_HIDDEN),
                  dw2_last(NUM_HIDDEN+1, NUM_OUTPUT)
        {
                reset();
        }


        void randomize(arma::mat &m) {
                m.randn();
                m /= m.max();
                m *= 0.5;
        }

        void reset() {
                dw1_last.zeros();
                dw2_last.zeros();

                randomize(w1);
                randomize(w2);
        }


	//---------------------------------------------------------------------------
	//
	// Forward propagate
	// 
	//---------------------------------------------------------------------------

        void fprop(Query &q) const {
                q.set_bias();
                q.hidden = arma::join_rows(
                                FH::activation(q.input * w1),
                                arma::ones(q.input.n_rows, 1));
                q.output = FO::activation(q.hidden * w2);
        }


	//---------------------------------------------------------------------------
	//
	// Back propogate
	// 
	//---------------------------------------------------------------------------

        double train_loop(Query &q, double threshold, double learning_rate, size_t max) {
                double sse=10.0, new_sse=11;
                size_t i=0;
                while (i < max) {
                        new_sse = train_one(q, learning_rate);
                        if (new_sse < threshold) {
                                sse = new_sse;
                                break;
                        }
                        sse = new_sse;
                        ++i;
                }
                return sse;
        }

        double train_one(Query &q, double learning_rate) {
                fprop(q);
                return backprop(q, q.target-q.output, learning_rate);
        }

        double backprop(Query &q, arma::mat error, double learning_rate) {
                arma::mat dw1, dw2, deltas_hid, deltas_out;

                deltas_out = error % FO::derivative(q.output);
                deltas_hid = (deltas_out * w2.t()) % FH::derivative(q.hidden);

                dw1 = q.input.t() * deltas_hid.cols(0, deltas_hid.n_cols-2);
                dw2 = q.hidden.t() * deltas_out;

                w1 += momentum * dw1_last + learning_rate * dw1;
                w2 += momentum * dw2_last + learning_rate * dw2;

                dw1_last = dw1;
                dw2_last = dw2;

                return arma::trace(error.t()*error);
        }

        double td_train(Query &q, double learning_rate) {
                return backprop(q, q.target-q.output, learning_rate);
        }
};

template <size_t NUM_NN,
          size_t NUM_INPUT,
	  size_t NUM_HIDDEN,
	  size_t NUM_OUTPUT,
          typename FH=Sigmoid,
          typename FO=Sigmoid>
struct EnsembleFFNet {
        typedef FFNet<NUM_INPUT,NUM_HIDDEN,NUM_OUTPUT,FH,FO> NN;
        typedef typename NN::Query Query;

        NN net[NUM_NN];
        double momentum;

        struct Task {
                EnsembleFFNet *n;
                size_t idx;
                Query q;
                double threshold;
                double learning_rate;
                size_t max;
                double momentum;

                void operator() (double &result) {
                        n->net[idx].momentum = momentum;
                        result = n->net[idx].train_loop(q, threshold, learning_rate, max);
                }
        };


        double train_loop(Query &q, double threshold, double learning_rate, size_t max) {
                TaskPool<Task,double> tasks(NUM_NN);
                for (size_t i=0; i < NUM_NN; ++i) {
                        Task t;
                        t.n = this;
                        t.idx = i;
                        t.q = q;
                        t.threshold = threshold;
                        t.learning_rate = learning_rate;
                        t.max = max;
                        t.momentum = momentum;
                        tasks.push(t);
                }

                tasks.run();
                double mse = 0;
                for (size_t i=0; i < NUM_NN; ++i)
                        mse += tasks[i];
                return mse / double(NUM_NN);
        }

        double train_one(Query &query, double learning_rate) {
                double mse = 0;
                for (size_t i=0; i < NUM_NN; ++i)
                        mse += net[i].train_one(query, learning_rate);
                return mse / double(NUM_NN);
        }

        void fprop(Query &q) const {
                q.output = arma::zeros(q.input.n_rows, NUM_OUTPUT);
                Query p;
                p = q;
                for (size_t i=0; i < NUM_NN; ++i) {
                        net[i].fprop(p);
                        q.output += p.output;
                }
                q.output /= double(NUM_NN);
        }
};

#endif // GGP_NEURAL_H
