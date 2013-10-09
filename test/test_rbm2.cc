#include <engine/rbm.h>
#include <engine/neural.h>
#include <ui/nn.h>

#include "ttt.h"

static const size_t BOARD_SIZE = 4,
                    NUM_INPUTS = BOARD_SIZE*BOARD_SIZE*2+2,
                    NUM_EPOCHS = 9000,
                    NUM_GAMES  = 1000,
                    REDRAW_RATE = NUM_INPUTS > 4000 ? 1 : 4000 * 1 / NUM_INPUTS,
                    MINIBATCH_SIZE = 100;
                        

static const float ERROR_THRESHOLD = 1 * float(MINIBATCH_SIZE) / 100.,
                   LEARNING_RATE = 0.8;

HintonDiagram hinton(1000, 500);

struct Stack {
        RBM input, feature;

        Stack(size_t size, size_t labels)
                : input(size, size, LEARNING_RATE),
                  feature(labels+size, size*3, LEARNING_RATE)
        {
        }

        arma::mat sample(arma::mat data) {
                arma::mat input_out,
                          feature_in,
                          feature_out,
                          label_out;
                
                input_out = input.run_visible(data);
                feature_in = arma::join_rows(arma::zeros(data.n_rows, 2), input_out);
                feature_out = feature.run_visible(feature_in);
                label_out = feature.run_hidden(feature_out);
                return label_out.cols(0, 1);
        }

        arma::mat batch(arma::mat m, size_t bsize, size_t i) {
                return m.rows(i*bsize, (i+1)*bsize-1);
        }

        void train(arma::mat samples, arma::mat labels) {
                const size_t bsize = MINIBATCH_SIZE;
                float error;
                arma::mat data, ldata;

                //for (size_t j=0; j < 2; ++j) {
                        for (size_t i=0; i < samples.n_rows / bsize; ++i)  {
                                LOG("batch " << i << "/" << samples.n_rows / bsize);
                                error = numeric_limits<float>::max();
                                data = batch(samples, bsize, i);
                                while (error > ERROR_THRESHOLD) {
                                        error = input.train(data, NUM_EPOCHS/REDRAW_RATE);
                                        hinton.update(input.weights, feature.weights);
                                        save();
                                }
                        }
                //}

                //}
                hinton.update(input.weights, feature.weights);
                //for (size_t j=0; j < 2; ++j) {
                        for (size_t i=0; i < samples.n_rows / bsize; ++i)  {
                                LOG("batch " << i << "/" << samples.n_rows / bsize);
                                error = numeric_limits<float>::max();
                                data = batch(samples, bsize, i);
                                ldata = batch(labels, bsize, i);
                                while (error > ERROR_THRESHOLD) {
                                        error = feature.train(arma::join_rows(
                                                                ldata,
                                                                input.run_visible(input.run_visible(data))),
                                                                NUM_EPOCHS/(REDRAW_RATE*4));
                                        hinton.update(input.weights, feature.weights);
                                        save();
                                }
                        }
                //}
        }

        void save() {
                input.save("data/input.dat");
                feature.save("data/feature.dat");
        }
};


size_t best_move(bool black, Stack &stack, TTTState &s) {
        size_t best_pos = 0;
        double best_score = numeric_limits<double>::min();

        TTTState test(NUM_INPUTS, BOARD_SIZE);
        arma::mat result;
        for (size_t i=0; i < test.positions; ++i) {
                if (!s.occupied(i)) {
                        test = s;
                        // place on board
                        test.place(black, i);

                        result = arma::zeros(1, 2);
                        for (size_t j=0 ; j < 1000; ++j)
                                result += stack.sample(test.state);
                        result /= 1000;
                        double score = result(0,0) - result(0,1);
                        if (!black) score = -score;
                        LOG("score(" << i << ") = " << score);
                        if (score > best_score) {
                                best_score = score;
                                best_pos = i;
                        }
                }
        }
        LOG("best score = " << best_score);
        return best_pos;
}

void rand_play(Stack &stack) {
        TTTState state(NUM_INPUTS, BOARD_SIZE);
        bool black = true, draw = true;
        size_t pos = 0;
        state.print();

        for (size_t i=0; i < state.positions; ++i) {
                // find next move
                state.set_color(black);
                if (!black)
                        pos = best_move(black, stack, state);
                else { LOG("black rand"); pos = state.rand_move(); }

                // place on board
                state.place(black, pos);
                state.print();

                // check for win
                if (state.win()) {
                        draw = false;
                        break;
                }

                // flip color
                black = !black;
        }

        if (!draw) {
                if (black) { LOG("BLACK wins"); }
                else { LOG("WHITE wins"); }
        } else { LOG("DRAW"); }
}

void rand_game(arma::mat &labels, arma::mat &samples) {
        // initialize row to zeroes (remember to discard)
        arma::mat rows;

        bool black = true, draw = true;
        TTTState state(NUM_INPUTS, BOARD_SIZE);
        for (size_t i=0; i < state.positions; ++i) {
                // find next move
                size_t pos = state.rand_move();

                // place on board
                state.set_color(black);
                state.place(black, pos);

                // append row
                rows = arma::join_cols(rows, state.state);

                // check for win
                if (state.win()) {
                        draw = false;
                        break;
                }

                // flip color
                black = !black;
        }

        // discard initial row
        arma::mat result = arma::zeros(1, 2);
        if (!draw) {
                if (black)
                        result(0,0) = 1;
                else result(0,1) = 1;
        }
        labels  = arma::join_cols(labels, arma::repmat(result, rows.n_rows, 1));
        samples = arma::join_cols(samples, rows);
}

void rand_board(arma::mat &labels, arma::mat &samples, size_t games) {
        for (size_t g=0; g < games; ++g)
                rand_game(labels, samples);
}


void random_queries(Stack &stack) {
        arma::mat result, samples, labels;
        while (true) {
                hinton.update(stack.input.weights, stack.feature.weights);
                rand_board(labels, samples, 1);
                size_t n = random() % samples.n_rows;
                samples.row(n).print("query:");
                result = arma::zeros(1, 2);
                for (size_t i=0 ; i < 1000; ++i)
                        result += stack.sample(samples.row(n));
                result /= 1000;
                result.print("result:");
        }
}

int main() {
        Stack stack(NUM_INPUTS, 2);
        arma::mat samples, labels;

        hinton.update(stack.input.weights, stack.feature.weights);

        LOG("generate training data...");
        rand_board(labels, samples, NUM_GAMES);

        stack.train(samples, labels);
        while (true) {
                rand_play(stack);
        }
}
