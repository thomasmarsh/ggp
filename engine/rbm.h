#ifndef GGP_RBM_H
#define GGP_RBM_H

// NOTE: ported from the python code by Edwin Chen

#include "common.h"

#include <armadillo>


struct RBM {
        size_t num_hidden, num_visible;
        float learning_rate;
        arma::mat weights;

        RBM(size_t vis, size_t hid, float alpha)
                : num_hidden(hid),
                  num_visible(vis),
                  learning_rate(alpha)
        {
                weights = arma::randu(num_visible, num_hidden) * 0.1;

                // insert weights for bias
                weights = prefix_zeros(weights);
        }

        void save(const char *s) {
                weights.save(s);
        }

        arma::mat prefix_zeros(arma::mat m) {
                return arma::join_rows(arma::zeros(m.n_rows+1,1),
                                arma::join_cols(arma::zeros(1,m.n_cols), m));
        }

        arma::mat prefix_bias(arma::mat m) {
                return arma::join_rows(arma::ones(m.n_rows,1), m);
        }

        arma::mat logistic(arma::mat x) {
                return 1/(1+arma::exp(-x));
        }

        arma::mat prob_on(arma::mat probs) {
                return arma::conv_to<arma::mat>::from(
                                probs > arma::randu(probs.n_rows, probs.n_cols));
        }

        float train(arma::mat _data, size_t max_epochs=1000, bool verbose=false) {
                arma::mat data,
                          pos_hidden_activations,
                          pos_hidden_probs,
                          pos_hidden_states,
                          pos_associations,
                          neg_visible_activations,
                          neg_visible_probs,
                          neg_hidden_activations,
                          neg_hidden_probs,
                          neg_associations;

                size_t num_examples = _data.n_rows;
                float error;

                // insert bias unit
                data = prefix_bias(_data);

                for (size_t epoch=0; epoch < max_epochs; ++epoch) {
                        // Positive CD phase ("reality phase")


                        // clamp to the data and sample from the hidden units
                        pos_hidden_activations = data * weights;
                        pos_hidden_probs = logistic(pos_hidden_activations);
                        pos_hidden_states = prob_on(pos_hidden_probs);
                        pos_associations = data.t() * pos_hidden_probs;

                        // Negative CD phase ("daydreaming")
                        
                        // reconstruct the visible units and sample again from
                        // hidden units.
                        neg_visible_activations = pos_hidden_states * weights.t();
                        neg_visible_probs = logistic(neg_visible_activations);
                        // fix bias unit
                        neg_visible_probs.col(0) = arma::ones(neg_visible_probs.n_rows, 1);
                        neg_hidden_activations = neg_visible_probs * weights;
                        neg_hidden_probs = logistic(neg_hidden_activations);
                        neg_associations = neg_visible_probs.t() * neg_hidden_probs;
                        weights += learning_rate
                                * ((pos_associations-neg_associations)
                                / float(num_examples));

                        if (verbose) {
                                error = sum(sum(square(data-neg_visible_probs)));
                                LOG("epoch " << epoch << ": error is " << error);
                        }
                }
                return sum(sum(square(data-neg_visible_probs)));
        }

        // Assuming the RBM has been trained (so that weights for the network have
        // been learned), run the network on a set of visible units, to get a sample
        // of the hidden units.
        //
        // Parameters
        // ----------
        // data: A matrix where each row consists of the states of the visible units.
        //
        // Returns
        // -------
        // hidden_states: A matrix where each row consists of the hidden units activated
        // from the visible units in the data matrix passed in.

        arma::mat run_visible(arma::mat _data) {
                arma::mat data,
                          hidden_states,
                          hidden_activations,
                          hidden_probs;

                size_t num_examples = _data.n_rows;
                // Create a matrix, where each row is to be the hidden units (plus a bias unit)
                // sampled from a training example.
                hidden_states = arma::ones(num_examples, num_hidden+1);
    
                // Insert bias units of 1 into the first column of data.
                data = prefix_bias(_data);

                // Calculate the activations of the hidden units.
                hidden_activations = data * weights;
                // Calculate the probabilities of turning the hidden units on.
                hidden_probs = logistic(hidden_activations);
                // Turn the hidden units on with their specified probabilities.
                hidden_states = prob_on(hidden_probs);
                // Always fix the bias unit to 1.
                // hidden_states.col(0) = arma::ones(hidden_states.n_rows, 1);
  
                // Ignore the bias units.
                return hidden_states.cols(1,hidden_states.n_cols-1);
        }
    
        // same as run_visible, but return the raw probabilities as softmax
        arma::mat run_visible_p(arma::mat _data) {
                arma::mat data,
                          hidden_states,
                          hidden_activations,
                          hidden_probs;

                size_t num_examples = _data.n_rows;
                // Create a matrix, where each row is to be the hidden units (plus a bias unit)
                // sampled from a training example.
                hidden_states = arma::ones(num_examples, num_hidden+1);
    
                // Insert bias units of 1 into the first column of data.
                data = prefix_bias(_data);

                // Calculate the activations of the hidden units.
                hidden_activations = data * weights;
                // Calculate the probabilities of turning the hidden units on.
                hidden_probs = logistic(hidden_activations);

                // Ignore the bias units.
                return hidden_probs.cols(1,hidden_probs.n_cols-1);
        }
    
        arma::mat run_hidden(arma::mat _data) {
                // Assuming the RBM has been trained (so that weights for the
                // network have been learned), run the network on a set of hidden
                // units, to get a sample of the visible units.
                //
                // Parameters
                // ----------
                // data: A matrix where each row consists of the states of the
                // hidden units.
                //
                // Returns
                // -------
                // visible_states: A matrix where each row consists of the visible
                // units activated from the hidden  units in the data matrix passed in.

                arma::mat data,
                          visible_states,
                          visible_activations,
                          visible_probs;


                size_t num_examples = _data.n_rows;

                // Create a matrix, where each row is to be the visible units
                // (plus a bias unit) sampled from a training example.
                visible_states = arma::ones(num_examples, num_visible+1);

                // Insert bias units of 1 into the first column of data.
                data = prefix_bias(_data);

                // Calculate the activations of the visible units.
                visible_activations = data * weights.t();
                // Calculate the probabilities of turning the visible units on.
                visible_probs = logistic(visible_activations);
                // Turn the visible units on with their specified probabilities.
                visible_states = prob_on(visible_probs);
                // Always fix the bias unit to 1.
                // visible_states[:,0] = 1

                // Ignore the bias units.
                return visible_states.cols(1,visible_states.n_cols-1);
        }
    
        arma::mat run_hidden_p(arma::mat _data) {
                // same as run_hidden, but return raw probabilities
                arma::mat data,
                          visible_states,
                          visible_activations,
                          visible_probs;


                size_t num_examples = _data.n_rows;

                // Create a matrix, where each row is to be the visible units
                // (plus a bias unit) sampled from a training example.
                visible_states = arma::ones(num_examples, num_visible+1);

                // Insert bias units of 1 into the first column of data.
                data = prefix_bias(_data);

                // Calculate the activations of the visible units.
                visible_activations = data * weights.t();
                // Calculate the probabilities of turning the visible units on.
                visible_probs = logistic(visible_activations);

                // Ignore the bias units.
                return visible_probs.cols(1, visible_probs.n_cols-1);
        }

        arma::mat daydream(size_t num_samples) {
                // Randomly initialize the visible units once, and start running
                // alternating Gibbs sampling steps (where each step consists of
                // updating all the hidden units, and then updating all of the
                // visible units), taking a sample of the visible units at each step.
                // Note that we only initialize the network *once*, so these samples
                // are correlated.
                //
                // Returns
                // -------
                // samples: A matrix, where each row is a sample of the visible units
                // produced while the network was daydreaming.


                arma::mat samples,
                          visible,
                          hidden_activations,
                          hidden_probs,
                          hidden_states,
                          visible_activations,
                          visible_probs,
                          visible_states;

                // Create a matrix, where each row is to be a sample of of the
                // visible units (with an extra bias unit), initialized to all ones.
                samples = arma::ones(num_samples, num_visible+1);

                // Take the first sample from a uniform distribution.
                samples.row(0).cols(1,num_visible) = arma::randu(num_visible).t();

                // Start the alternating Gibbs sampling.
                // Note that we keep the hidden units binary states, but leave the
                // visible units as real probabilities. See section 3 of Hinton's
                // "A Practical Guide to Training Restricted Boltzmann Machines"
                // for more on why.

                for (size_t i=1; i < num_samples; ++i) {
                        visible = samples.row(i-1);

                        // Calculate the activations of the hidden units.
                        hidden_activations = visible * weights;
                        // Calculate the probabilities of turning the hidden units on.
                        hidden_probs = logistic(hidden_activations);
                        // Turn the hidden units on with their specified probabilities.
                        hidden_states = prob_on(hidden_probs);
                        // Always fix the bias unit to 1.
                        hidden_states(0) = 1;

                        // Recalculate the probabilities that the visible units are on.
                        visible_activations = hidden_states * weights.t();
                        visible_probs = logistic(visible_activations);
                        visible_states = prob_on(visible_probs);
                        samples.row(i) = visible_states;
                }

                // Ignore the bias units (the first column), since they're always
                // set to 1.
                return samples.cols(1,samples.n_cols-1);
        }
};

#endif // GGP_RBM_H
