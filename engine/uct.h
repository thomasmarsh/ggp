#ifndef UCT_H
#define UCT_H
#pragma once

#include "common.h"
#include "thread.h"
#include "memory.h"

static const size_t NUM_THREADS = 8;

#pragma pack(1)
template <typename S, size_t MAX_MOVES>
struct UCTNode {
        typedef typename S::ML ML;
        static MemoryPool<UCTNode> pool;

        S state;
        uint32_t move, untried;
        UCTNode *parent, *next, *child, *last;
        ML moves;
        bool tried[MAX_MOVES];
        float wins;
        uint32_t visits;
        uint32_t amaf;

        UCTNode() { clear(); }
        ~UCTNode() {}
        

        void init(S &state, size_t m, UCTNode *p) {
                clear();
                move = m;
                parent = p;
                state.copy_from(state);
                state.moves(moves);
                untried = moves.size();
        }

        void clear() {
                state.clear();
                move = untried = visits = amaf = 0;
                parent = next = child = last = 0;
                moves.clear();
                memset(&tried, 0, sizeof(bool)*MAX_MOVES);
                wins = 0;
                /*memset(this, 0, sizeof(UCTNode));*/
        }

        float uct(float Cp, UCTNode *n) {
                /*
                 *   Q(v')           / 2 ln N(v) \
                 *   -----  + c sqrt|  ---------  |
                 *   N(v')           \   N(v')   /
                 *
                 */

                float Q = n->wins / (float) n->visits;
                float U = sqrt(2*log(visits) / (float) n->visits);
                return Q + Cp * U;
        }


        UCTNode *select(float Cp) {
                float best = (float) std::numeric_limits<int>::min();
                UCTNode *result = 0;
                UCTNode *n = 0;
                for (n = child; n != NULL; n = n->next) {
                        float score = uct(Cp, n);
                        if (score > best) {
                                best = score;
                                result = n;
                        }
                }
                return result;
        }

        uint32_t expand() {
                uint32_t choice = random() % untried;
                uint32_t c = untried;
                assert(c > 0);
                for (size_t i=0; i < moves.size(); i++) {
                        if (tried[i] == false) {
                                c--;
                                if (c == choice) {
                                        tried[i] = true;
                                        untried--;
                                        return i;
                                }
                        }
                }
                assert(1==0);
                return -1;
        }

        UCTNode *add(size_t m, S &state) {
                UCTNode *n = pool.alloc();
                n->init(state, m, this);
                tried[m] = true;
                if (!child) {
                        child = n;
                        last = n;
                } else {
                        last->next = n;
                        last = n;
                }
                assert(last->next == 0);
                return n;
        }

        void update(float result) {
                visits++;
                wins += result;
        }

        typename S::M &get_move() {
                UCTNode *n = this;
                if (parent) n = parent; 
                return n->moves[move];
        }

        void make_move(S &state) {
                state.move(get_move());
        }
};

template <typename S, size_t MAX_ITER, size_t MAX_MOVES>
struct UCT {
        typedef typename S::ML ML;
        float Cp;
        Mutex mutex;

        typedef UCTNode<S,MAX_MOVES> Node;

        UCT() : Cp(sqrt(2)) {}

        Node* select(S &state, Node *node) {
                DEBUG("SELECT");
                while (node && node->untried == 0 && node->child != 0) {
                        node = node->select(Cp);
                        if (node)
                                node->make_move(state);
                }
                return node;
        }

        Node* expand(S &state, Node *node) {
                DEBUG("EXPAND");
                if (node->untried > 0) {
                        uint32_t m = node->expand();
                        state.move(node->moves[m]);
                        node = node->add(m, state);
                }
                return node;
        }

        void rollout(S &state) {
                DEBUG("ROLLOUT");
                ML ml;
                typename S::M m;
                while (!state.game_over()) {
                        if (state.random_move(ml, m))
                                state.move(m);
                }
        }

        void backprop(S &state, Node *node) {
                DEBUG("BACKPROPAGATE");
                Color sc = other(state.current());
                while (node) {
                        node->update(state.result(sc));
                        node = node->parent;
                        sc = other(sc);
                }
        }

        void iterate(Node *root, S &state) {
                DEBUG("INIT");
                Node *node = root;
                S child;
                child.copy_from(state);

                { Lock lock(mutex);
                        node = select(child, node);
                        if (!node) return;
                        node = expand(child, node);
                }

                rollout(child);

                { Lock lock(mutex);
                        backprop(child, node);
                }
        }

        void move(Node *root, S &state) {
                DEBUG("MOVE");
                Node *result=root->child;

                if (!result) { state.set_game_over(); return; }
                assert(result);
                uint32_t best=result->visits;

                for (Node *n=root->child; n != NULL; n=n->next) {
                        //LOG("{ " << n->visits << ' ' << ((float) n->visits / (float) MAX_ITER) << " " << state.move_str(n->get_move()) << " }");
                        if (n->visits > best) {
                                best = n->visits;
                                result = n;
                        }
                }
                if (root->child != NULL) {
                        assert(result);
                        state.announce(result->get_move());
                        state.move(result->get_move());
                }
                else state.set_game_over();
        }


        struct Task {
                UCT *uct;
                size_t iter;
                Node *root;
                S *state;

                void operator () (int dummy) {
                        for (size_t i=0; i < iter; ++i)
                                uct->iterate(root, *state);
                }
        };


        void next(Color c, S &state) {
                Node::pool.clear();
                Node root;
                root.init(state, 0, 0);

                TaskPool<Task> tasks(NUM_THREADS);

                Task task;
                task.uct = this;
                task.root = &root;
                task.iter = MAX_ITER / NUM_THREADS;
                task.state = &state;

                for (size_t i=0; i < NUM_THREADS; ++i)
                        tasks.push(task);

                tasks.run();

                move(&root, state);
        }
        void set_param(float p) { Cp = p; }
        void pretrain(size_t i) {}
};

#endif // UCT_H
