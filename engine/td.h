#ifndef GGP_TD_H
#define GGP_TD_H
#pragma once

#include "common.h"
#include "neural.h"
#include "../ui/nn.h"

#include <stack>
#include <algorithm>

template <typename T>
T sgn_without_zero(T val) {
        return (val < T(0)) ? T(-1) : T(1);
}

template <typename T>
T sgn(T val) {
        return (T(0) < val) - (val < T(0));
}

template <typename S,
          size_t SIZE,
          size_t NUM_HIDDEN,
          typename FH=Sigmoid,
          typename FO=Sigmoid>
struct TD {
        typedef typename S::ML ML;
        typedef typename S::M M;
        typedef FFNet<SIZE,NUM_HIDDEN,1,FH,FO> NN;
        typedef typename NN::Query Q;

        ML ml;
        NN net;
        vector<Q> history;
        vector<double> QV;
        bool training_mode;
        double alpha, lambda, gamma, learning_rate, momentum, greedy, mse;
#ifdef HINTON
        HintonDiagram hinton;
#endif

        TD()
                : training_mode(false),
                  alpha(0.1),
                  lambda(0.3),
                  gamma(0.5),
                  learning_rate(0.05),
                  momentum(0.0),
                  greedy(0.5),
                  mse(0)
#ifdef HINTON
                  , hinton(700, 700)
#endif
        {
        }

        struct Result {
                size_t index;
                Q q;

                double score() { return q.output(0,0) >= 0 ? q.output(0,0) : 0; }
        };

        struct Task {
                size_t index;
                NN *net;
                S child;
                Q q;

                void operator() (Result &result) {

                        for (size_t j=0; j < SIZE; ++j)
                                q.input(0, j) = child[j];

                        net->fprop(q);

                        result.index = index;
                        result.q = q;
                }
        };

        size_t choose(Color c, S &state, ML &ml) {
                assert(ml.size() > 0);

                TaskPool<Task,Result> tasks(NUM_THREADS);

                typename NN::Query q(1);
                for (size_t i=0; i < ml.size(); ++i) {
                        Task task;
                        task.q = q;
                        task.index = i;
                        task.net = &net;
                        task.child.copy_from(state);
                        task.child.move(ml[i]);
                        tasks.push(task);
                }

                tasks.run();

                if (training_mode) {
                        double sum=0;
                        for (size_t i=0; i < ml.size(); ++i)
                                sum += tasks[i].score();
                        double r = (randf()+1/2) * sum;
                        for (size_t i=0; i < ml.size(); ++i) {
                                if (r < tasks[i].score()) {
                                        history.push_back(tasks[i].q);
                                        return tasks[i].index;
                                }
                                r -= tasks[i].score();
                        }
                        DIE("notreached");
                }
                size_t choice = 0;
                size_t highest_idx=0, lowest_idx=0;
                double highest = numeric_limits<double>::min(),
                      lowest  = numeric_limits<double>::max(),
                      best = 0;

                double sum=0;
                for (size_t i=0; i < ml.size(); ++i) {
                        sum += tasks[i].score();
                        if (tasks[i].score() > highest) {
                                highest = tasks[i].score();
                                highest_idx = tasks[i].index;
                        }

                        if (tasks[i].score() < lowest) {
                                lowest = tasks[i].score();
                                lowest_idx = tasks[i].index;
                        }
                }

                if (c == WHITE) {
                        choice = lowest_idx;
                        best = lowest;
                } else {
                        choice = highest_idx;
                        best = highest;
                }

                for (size_t i=0; i < ml.size(); ++i) {
#if 1
                        if (!training_mode && tasks[i].score() > 0)
                                LOG("i=" << i <<
                                    " score=" << (double)tasks[i].score()/sum <<
                                    " move=" << ml[i].str());
#endif
                        vector<size_t> choices;
                        if (tasks[i].score() == best)
                                choices.push_back(tasks[i].index);
                        if (choices.size() > 1)
                                choice = choices[random() % choices.size()];
                }

                if (training_mode) {
                        if (randf() < greedy)
                                choice = random() % ml.size();
                        history.push_back(tasks[choice].q);
                }

                return choice;
        }


        void next(Color c, S &state) {
                ml.clear();
                state.moves(ml);
                if (ml.size() > 0) {
                        size_t choice = choose(c, state, ml);
                        if (!training_mode)
                                state.announce(ml[choice]);
                        state.move(ml[choice]);
                }
        }

        void set_param(double p) {}

        void reset() {
                while (!history.empty()) history.pop_back();
        }

        double learn(Color c, double rw) {

                double mse=0;
                double n = history.size();

                std::stack<double> qv;
                for (size_t i=0; i < history.size(); ++i) {
                        double sum=0;
                        double product=1;
                        for (size_t j=i; j < history.size(); ++j) {
                                double reward = (j == history.size()-1) ? rw : 0;
                                double delta = (j < history.size()-1) ?
                                        reward + gamma*history[j+1].output(0,0)-
                                                       history[j].output(0,0) : reward;
                                sum += product * delta;
                                product *= gamma * lambda;
                        }
                        double q = history[i].output(0,0) + alpha*sum;
                        q = sgn_without_zero(q) * std::min(64.0, fabs(q));
                        if (q < 0) q = 0;
                        qv.push(q);
                }

                while (!history.empty()) {
                        Q q = history.back();
                        assert(qv.size() > 0);
                        q.target(0, 0) = qv.top();
                        LOG("q=" << qv.top());
                        qv.pop();
                        net.momentum = momentum;
                        mse += net.td_train(q, learning_rate);
                        c = other(c);
                }
                reset();
                return mse / n;
        }

        double target(const S &s) const {
                if (s.winner() == NONE)  return  0.0;
                if (s.winner() == BLACK) return  1.0;
                if (s.winner() == WHITE) return  0.0;
                DIE("not reached");
        }

        void train(size_t iter, const S &s) {
                //if (target(s) == 0) return;
                if ((iter % 100) == 0) {
                        LOG("i=" << iter <<
                            " t=" << target(s) <<
                            " a=" << history.back().output(0, 0));
                }
                mse += learn(s.just_played(), target(s));
                if ((iter % 100) == 0) {
                        LOG("mse=" << (mse/100));
                        mse = 0;
                }
#ifdef HINTON
                hinton.update(net.w1, net.w2);
#endif
                //net.save("yavalath.nn");
        }

        void play_game(S &s) {
                while (!s.game_over())
                        next(s.current(), s);
        }

        void pretrain(size_t len) {
                training_mode = true;
                for (size_t i=0; i < len; ++i) {
                        //LOG("train i=" << i);
                        S s;
                        play_game(s);
                        train(i, s);
                }
                training_mode = false;
        }
};


#endif // GGP_TD_H
