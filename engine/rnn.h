#ifndef GGP_RNN_H
#define GGP_RNN_H

#include "neural.h"

template <size_t NUM_INPUT,
	  size_t NUM_HIDDEN,
	  size_t NUM_OUTPUT,
          typename FH=Sigmoid,
          typename FO=Sigmoid>
class RNN {
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


        arma::mat w1, w2, // weights
                  h; // last hidden activation (memory)


	//---------------------------------------------------------------------------
	//
	// Initialization
	// 
	//---------------------------------------------------------------------------

        RNN() :
                  w1(NUM_INPUT+NUM_HIDDEN+1, NUM_HIDDEN),
                  w2(NUM_HIDDEN+1, NUM_HIDDEN+NUM_OUTPUT)
        {
                reset();
        }

        void reset() {
                w1.randn();
                w1 /= w1.max();
                w2.randn();
                w2 /= w2.max();

                w1 *= .5;
                w2 *= .5;
        }


	//---------------------------------------------------------------------------
	//
	// Forward propagate
	// 
	//---------------------------------------------------------------------------

        void fprop(Query &q) {
                q.set_bias();
                if (h.n_rows != q.input.n_rows)
                        h.resize(q.input.n_rows, NUM_HIDDEN);

                q.hidden = 
                        arma::join_rows(
                                FH::activation(
                                        arma::join_rows(q.input, h) * w1),
                                arma::ones(q.input.n_rows, 1));

                arma::mat tmp = FO::activation(q.hidden * w2);
                q.output = tmp.cols(0, NUM_OUTPUT-1);
                h = tmp.cols(NUM_OUTPUT, NUM_HIDDEN);
        }
};

#endif // GGP_RNN_H
