#include <engine/symbolic.h>
#define DEEP_COPY
#include <engine/genetic.h>
#include <ui/plot.h>

#include <utility>

#define SIMPLIFY
#define MUTATE

static const size_t NUM_POOLS = 8,
                    POP_SIZE  = 20;


struct PoolData {
        Node::value_t x;
        Node::value_t n;
};


struct Symbolic {
        enum Term {
                V_X,
                C7, C4, C10,
                ADD, SUB, MUL,
                NEG,

                NUM_TERMS,

                DIV,
                C1, C2, C3, C5, C6,
                C8, C9, C20,
                POW, SQRT,
                FLOOR, CEIL,
                SIGN, FMOD,
                CPI, CE,
                EXP, LOGN,
                V_N,
                SIN, COS, TAN,
                DIFF
        };

        static Node::value_t var_x[NUM_POOLS+1];
        static Node::value_t var_n[NUM_POOLS];

        static Node::Arity arity_of(Term t) {
                switch (t) {
                case V_X: case V_N: case C1: case C2: case C3: case C4: case C5: case C6:
                case C7: case C8: case C9: case C10: case C20: case CPI: case CE: 
                        return Node::NULLARY;
                case EXP: case LOGN: case NEG: case SIN: case COS: case TAN: case DIFF: case SQRT:
                case FLOOR: case CEIL: case SIGN:
                        return Node::UNARY;
                case ADD: case SUB: case MUL: case DIV: case POW: case FMOD:
                        return Node::BINARY;
                case NUM_TERMS: break;
                }
                DIE("not reached");
        }

        static Node *make_term(Term t, size_t pool_id) {
                switch (t) {
                case V_X:  return new Variable(&(var_x[pool_id]), "x");
                case V_N:  return new Variable(&(var_n[pool_id]), "n");
                case C1:   return new Constant(1);  case C2:  return new Constant(2);
                case C3:   return new Constant(3);  case C4:  return new Constant(4);
                case C5:   return new Constant(5);  case C6:  return new Constant(6);
                case C7:   return new Constant(7);  case C8:  return new Constant(8);
                case C9:   return new Constant(9);
                case C10:  return new Constant(10); case C20: return new Constant(20);
                case CPI:  return new Constant(M_PI);
                case CE:   return new Constant(M_E);
                case ADD:  return new Add();
                case SUB:  return new Subtract();
                case MUL:  return new Multiply();
                case DIV:  return new Divide();
                case LOGN: return new Log();
                case EXP:  return new Exp();
                case NEG:  return new Negate();
                case POW:  return new Power();
                case SQRT: return new Sqrt();
                case FLOOR: return new Floor();
                case CEIL: return new Ceil();
                case SIGN: return new Sign();
                case FMOD: return new Fmod();
                case SIN:  return new Sin();
                case COS:  return new Cos();
                case TAN:  return new Tan();
                case DIFF: return new Differential();
                case NUM_TERMS: break;
                }
                DIE("not reached");
        }

        static void release(Node *n) {
                if (n->arity == Node::UNARY)
                        delete ((Unary *) n);
                else if (n->arity == Node::BINARY)
                        delete ((Binary *) n);
                else delete n;
        }

        static Node *simplify(Node *n) {
                Node *s = n->simplify();
                release(n);
                return s;
        }

        static Node *random_nullary(size_t pool) {
                Term t = (Term) (::random() % NUM_TERMS);
                while (arity_of(t) != Node::NULLARY)
                        t = (Term) (::random() % NUM_TERMS);

                return make_term(t, pool);
        }

        static Unary *random_unary(size_t pool) {
                Term t = (Term) (::random() % NUM_TERMS);
                while (arity_of(t) != Node::UNARY)
                        t = (Term) (::random() % NUM_TERMS);

                return (Unary*) make_term(t, pool);
        }

        static Binary *random_binary(size_t pool) {
                Term t = (Term) (::random() % NUM_TERMS);
                while (arity_of(t) != Node::BINARY)
                        t = (Term) (::random() % NUM_TERMS);

                return (Binary *) make_term(t, pool);
        }

        static Node *random(size_t pool) {
                Term t = (Term) (::random() % NUM_TERMS);
                Node *n = make_term(t, pool);
                if (n->arity == Node::UNARY)
                        ((Unary*) n)->n = random(pool);
                else if (n->arity == Node::BINARY) {
                        ((Binary*) n)->a = random(pool);
                        ((Binary*) n)->b = random(pool);
                }
                return n;
        }


        static void populate(vector<Node*> &elt, const Node *n) {
                elt.push_back(const_cast<Node*>(n));

                if (n->arity == Node::UNARY)
                        populate(elt, ((Unary*) n)->n);
                else if (n->arity == Node::BINARY) {
                        populate(elt, ((Binary*) n)->a);
                        populate(elt, ((Binary*) n)->b);
                }
        }

        static size_t size(const Node *n) {
                size_t s = 1;
                if (n->arity == Node::UNARY)
                        s += size(((Unary*) n)->n);
                else if (n->arity == Node::BINARY) {
                        s += size(((Binary*) n)->a);
                        s += size(((Binary*) n)->b);
                }
                return s;
        }

        static Node *find_random(const Node *n) {
                vector<Node*> elements;
                populate(elements, n);
                return elements[::random() % elements.size()];
        }

        static Node *parent(const Node *n, const Node *x) {
                if (n->arity == Node::UNARY) {
                        Unary *u = (Unary *) n;
                        if (u->n == x)
                                return u;
                        return parent(u->n, x);
                } else if (n->arity == Node::BINARY) {
                        Binary *b = (Binary *) n;
                        if (b->a == x)
                                return b->a;
                        if (b->b == x)
                                return b->b;
                        Node *tmp = parent(b->a, x);
                        if (tmp) return tmp;
                        return parent(b->b, x);
                }
                return NULL;
        }

        static Node *mdescend(Node *n, Node *r, size_t pool) {
                if (r == n) {
                        if (n->arity == Node::UNARY) {
                                Unary *w = random_unary(pool);
                                w->n = ((Unary *) n)->n->deep_copy();
                                return w;
                        }

                        if (n->arity == Node::BINARY) {
                                Binary *w = random_binary(pool);
                                w->a = ((Binary *) n)->a->deep_copy();
                                w->b = ((Binary *) n)->b->deep_copy();
                                return w;
                        }

                        return random_nullary(pool);
                }

                Node *w = n->copy();
                if (w->arity == Node::UNARY) {
                        ((Unary *) w)->n = mdescend(((Unary *) w)->n, r, pool);
                        return w;
                } else if (w->arity == Node::BINARY) {
                        ((Binary *) w)->a = mdescend(((Binary *) w)->a, r, pool);
                        ((Binary *) w)->b = mdescend(((Binary *) w)->b, r, pool);
                        return w;
                }
                return w;
        }

        static Node *mutate(Node *n, size_t pool) {
                Node *m = mdescend(n, find_random(n), pool);
                delete n;
                return m;
        }

        static void cdescend_unary(Unary *n, Node *d, Node *match) {
                if (n->n == match)
                        n->n = d->copy();
                else n->n = n->n->copy();
                cdescend(n->n, d, match);
        }

        static void cdescend_binary(Binary *n, Node *d, Node *match) {
                if (n->a == match)
                        n->a = d->copy();
                else n->a = n->a->copy();
                cdescend(n->a, d, match);

                if (n->b == match)
                        n->b = d->copy();
                else n->b = n->b->copy();
                cdescend(n->b, d, match);
        }

        static void cdescend(Node *c, Node *d, Node *match) {
                if (c->arity == Node::UNARY)
                        cdescend_unary((Unary*) c, d, match);
                else if (c->arity == Node::BINARY)
                        cdescend_binary((Binary*) c, d, match);
        }

        static Node* crossover(Node *a, Node *b) {
                if ((::random() % 2) == 0)
                        std::swap(a,b);

                Node *ra = find_random(a),
                     *rb = find_random(b);

                Node *c = a->copy();
                cdescend(c, rb, ra);

                return c;
        }

        static Node *dup(Node *n) {
                Node *m = n->copy();
                if (m->arity == Node::UNARY)
                        ((Unary*) m)->n = dup(((Unary*) n)->n);
                else if (m->arity == Node::BINARY) {
                        ((Binary*)m)->a = dup(((Binary*) n)->a);
                        ((Binary*)m)->b = dup(((Binary*) n)->b);
                }
                return m;
        }

};

Node::value_t Symbolic::var_x[NUM_POOLS+1];
Node::value_t Symbolic::var_n[NUM_POOLS];
std::string best[NUM_POOLS];
size_t itnum[NUM_POOLS];

double BEST=numeric_limits<double>::max();

Mutex mutex;

void announce(Node *n, double score, size_t pool) {
        Lock l(mutex);
        
        if (score < BEST) {
                SLOG("{" << pool << ':' << itnum[pool] << "} " << score << " "
                        //<< std::setprecision(numeric_limits<Node::value_t>::digits10)
                        //<< std::fixed
                        << n->str());
                BEST = score;
        }
}

void test1_crossover() {
        srandomdev();
        Node *n1, *n2, *c1;

        n1 = Symbolic::random(0);
        n2 = Symbolic::random(0);

        while (true) {
                c1 = Symbolic::crossover(n1, n2);

                LOG("n1 = " << n1->str());
                LOG("n2 = " << n2->str());
                LOG("c1 = " << c1->str());

                delete n1;
                delete n2;

                n1 = c1;
                n2 = Symbolic::random(0);
        }
}

void test_mutate() {
        srandomdev();
        Node *n=0, *m=0;

        while (true) {
                n = Symbolic::random(0);
                LOG("n = " << n->sexpr());
                m = Symbolic::mutate(n, 0);
                LOG("m = " << m->sexpr());
                Symbolic::release(m);
        }
}

static const size_t NUM_TESTS = 33;

struct SolutionRow {
        static double f(double i) {
                double mcol=0, col=0, idx=0;
                while (true) {
                        if (col >= mcol) {
                                col = 0;
                                mcol++;
                        }
                        if (idx > (i-1))
                                return mcol-1;
                        idx++;
                        col++;
                }
                DIE("not reached");
        }

        static double error(const Node *n, size_t pool) {
                double mse=0;
                size_t count=0;
                for (size_t i=100000; i < 100010; ++i) {
                        // set Variable
                        Symbolic::var_x[pool] = i;

                        double z = n->eval();
                        if (isnan(z))
                                return numeric_limits<double>::max();
                        double y = z - f(i);
                        mse += y*y;
                        ++count;
                }
                return mse / double(count);
        }
};

struct SolutionCol {
        static double f(double i) {
                double mcol=0, col=0, idx=0;
                while (true) {
                        if (col >= mcol) {
                                col = 0;
                                mcol++;
                        }
                        if (idx > (i-1))
                                return col;
                        idx++;
                        col++;
                }
                DIE("not reached");
        }

        static double error(const Node *n, size_t pool) {
                double mse=0;
                size_t count=0;
                for (size_t i=0; i < (19*(19+1)/2); ++i) {
                        // set Variable
                        Symbolic::var_x[pool] = i;

                        double z = n->eval();
                        if (isnan(z))
                                return numeric_limits<double>::max();
                        double y = z - f(i);
                        mse += y*y;
                        ++count;
                }
                return mse / double(count);
        }
};

struct NodePolicy {
        static void randomize(Node *& n, size_t pool) { n = Symbolic::random(pool); }
        static Node::value_t function(Node::value_t x) {
                //return 1/(1+exp(-x));
                //return abs(sin(x));
                //return x*x - 18*x + 29;
                return 7*x*x - 10*x + 4;
                //return tanh(x);
        }

        static void set_pool(Node *n, size_t pool) {
                if (n->is_variable()) {
                        Variable *v = (Variable*) n;
                        if (v->name[0] == 'x') {
                                v->var = &(Symbolic::var_x[pool]);
                        } else if (v->name[0] == 'n') {
                                v->var = &(Symbolic::var_n[pool]);
                        } else {
                                LOG("v->name = " << v->name);
                                DIE("not reached");
                        }
                } else if (n->arity == Node::UNARY) {
                        set_pool(((Unary *) n)->n, pool);
                } else if (n->arity == Node::BINARY) {
                        set_pool(((Binary *) n)->a, pool);
                        set_pool(((Binary *) n)->b, pool);
                }
        }

        typedef SolutionCol T;

        static bool compare(const Node *a, const Node *b, size_t pool) {
                const Node::value_t ea=T::error(a, pool), eb=T::error(b, pool);
                return ea == eb ? (Symbolic::size(a) < Symbolic::size(b)) : ea < eb;
        }

        static Node *null() { return 0; }
        static void release(Node *&n) { if (n) delete n; }

        static void iterate(Pool<Node*, NodePolicy> &p, Pool<Node*,NodePolicy> &t, size_t pool) {
                if (best[pool].compare(p.pop[0]->str())) {
                        best[pool] = p.pop[0]->str();
                        announce(p.pop[0], NodePolicy::T::error(p.pop[0], pool), pool);
                }
                itnum[pool]++;

                // keep two best
                t.pop[0] = Symbolic::dup(p.pop[0]);
                t.pop[1] = Symbolic::dup(p.pop[1]);
#ifdef SIMPLIFY
                t.pop[0] = Symbolic::simplify(t.pop[0]);
                t.pop[1] = Symbolic::simplify(t.pop[1]);
#endif

                // crossover 4
                Node *a, *b;
                
                a=b=p.pop[0]; while (a==b) b=p.choice();
                t.pop[2] = Symbolic::crossover(a, b);
                a=b=p.pop[0]; while (a==b) b=p.choice();
                t.pop[3] = Symbolic::crossover(a, b);


                a=b=p.pop[1]; while (a==b) b=p.choice();
                t.pop[4] = Symbolic::crossover(a, b);
                a=b=p.pop[1]; while (a==b) b=p.choice();
                t.pop[5] = Symbolic::crossover(a, b);

                // crossover random
                for (size_t i=6; i < p.pop.size()/2; ++i) {
                        a=p.choice(), b=p.choice();
                        while (a==b)
                                a=p.choice(), b=p.choice();

                        t.pop[i] = Symbolic::crossover(a, b);

#ifdef MUTATE
                        if (randf() > .3)
                                t.pop[i] = Symbolic::mutate(t.pop[i], pool);
#endif // MUTATE
                }
                // fill random
                for (size_t i=p.pop.size()/2; i < p.pop.size(); ++i)
                        t.pop[i] = Symbolic::random(pool);

                p.clear();
                p = t;
        }
};


void test_big() {
        srandomdev();
        typedef Population<Node*, NodePolicy> Pop;

        Pop p(POP_SIZE, NUM_POOLS);
        //for (size_t i=0; i < NUM_POOLS; ++i) {
                //delete p.pools[i].pop[0];
                //p.pools[i].pop[0] = (Node*) new Ceil(new Subtract(new Sqrt(new Multiply(new Constant(2), new Variable(&(Symbolic::var_x[i]), "x"))),
                 //       new Sqrt(new Sqrt(new Constant(5)))));
        //}
        p.iterate();
}

void setup_globals() {
        for (size_t i=0; i < NUM_POOLS; ++i) {
                Symbolic::var_x[i] = 0;
                best[i] = "";
                itnum[i] = 0;
        }
}

int main() {
        setup_globals();
        test_big();
}
