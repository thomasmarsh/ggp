#ifndef GGP_NEURAL_H
#define GGP_NEURAL_H

#include "armadillo"

#include <cstdio>
#include <vector>

#define D(x)

//-----------------------------------------------------------------------------------
//
// Utility
// 
//-----------------------------------------------------------------------------------

static inline float cross_entropy(float y, float t) {
	return -t * log(y) - (1 - t) * log(1 - y);
}

//-----------------------------------------------------------------------------------
//
// Activation Functions
// 
//-----------------------------------------------------------------------------------

struct Sigmoid {
	static inline arma::mat activ(arma::mat x) {
		return 1/(1+exp(-x));
	}

	static inline arma::mat deriv(arma::mat x) {
		return x%(1-x);
	}
};

struct Tanh {
	static inline arma::mat activ(arma::mat x) {
                return 2 / (1 + arma::exp(-2 * x)) - 1;
                //return arma::tanh(x);
	}

	static inline arma::mat deriv(arma::mat x) {
                return 1-square(x);
	}
};

struct Softplus {
	static inline arma::mat activ(arma::mat x) {
                return arma::log(1+exp(x));
	}

	static inline arma::mat deriv(arma::mat x) {
		return 1/(1+exp(-x));
	}
};

struct Linear {
	static inline arma::mat activ(arma::mat x) { return x; }
	static inline arma::mat deriv(arma::mat x) { return x.ones(); }
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
                Query(size_t size=1)
                        : input(size, NUM_INPUT+1), target(size, NUM_OUTPUT)
                {}

                void set_bias() {
                        std::cout << "set_bias1" << std::endl;
                        input.col(input.n_cols-1) = arma::ones(input.n_rows, 1);
                        std::cout << "set_bias2" << std::endl;
                }

                void randomize() {
                        input.randu();
                        set_bias();
                }

                arma::mat input, hidden, output, target;
        };

        arma::mat w1, // weights
                  w2,
                  dw1_last, // previous weight changes
                  dw2_last;


        float momentum;



	//---------------------------------------------------------------------------
	//
	// Initialization
	// 
	//---------------------------------------------------------------------------

        FFNet() : momentum(.8),
                  w1(NUM_INPUT+1, NUM_HIDDEN),
                  w2(NUM_HIDDEN+1, NUM_OUTPUT),
                  dw1_last(NUM_INPUT+1, NUM_HIDDEN),
                  dw2_last(NUM_HIDDEN+1, NUM_OUTPUT)
        {
                reset();
        }

        void reset() {
                dw1_last.zeros();
                dw2_last.zeros();

                w1.randn();
                w2.randn();

                w1 /= 10;
                w2 /= 10;
        }


	//---------------------------------------------------------------------------
	//
	// Forward propagate
	// 
	//---------------------------------------------------------------------------

        void fprop(Query &q) const {
                q.set_bias();
                q.hidden = FH::activ(q.input * w1);
                q.hidden = arma::join_rows(q.hidden, arma::ones(q.hidden.n_rows, 1));
                q.output = FO::activ(q.hidden * w2);
        }


	//---------------------------------------------------------------------------
	//
	// Back propogate
	// 
	//---------------------------------------------------------------------------

        float train_loop(const Query &q, float threshold=0.01, float learning_rate=0.1) {
                float sse=10.0, new_sse=11;
                while (true) {
                        new_sse = train_one(q, learning_rate);
                        if (new_sse > threshold) {
                                sse = new_sse;
                                break;
                        }
                        if (new_sse == sse)
                                reset();
                        else sse = new_sse;
                        D(std::cout << sse << std::endl);
                }
                return sse;
        }

        float train_one(const Query &query, float learning_rate=0.1) {
                arma::mat dw1, dw2, error, deltas_hid, deltas_out;
                Query q;

                q.input = query.input;
                fprop(q);

                error = query.target - q.output;

                deltas_out = error % FO::deriv(q.output);
                deltas_hid = (deltas_out * w2.t()) % FH::deriv(q.hidden);

                dw1 = learning_rate * q.input.t() * deltas_hid.cols(0, deltas_hid.n_cols-2) + momentum * dw1_last;
                dw2 = learning_rate * q.hidden.t() * deltas_out + momentum * dw2_last;

                w1 += dw1;
                w2 += dw2;

                dw1_last = dw1;
                dw2_last = dw2;

                return arma::trace(error.t()*error);
        }
};

#endif // GGP_NEURAL_H
