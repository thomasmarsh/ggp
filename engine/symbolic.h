#ifndef GGP_SYMBOLIC_H
#define GGP_SYMBOLIC_H
#pragma once

#include "common.h"

struct DefaultAllocator {
        template <typename T> static T* alloc() { return new T; }
        template <typename T> static void release(T *n) { delete n; }
};

struct Node {
        typedef long double value_t;

        enum Type {
                CONSTANT, VARIABLE, NEGATE, DIFFERENTIAL, LOGN,
                ADD, SUBTRACT, MULTIPLY, DIVIDE, POWER, SQRT, EXP,
                FLOOR, CEIL, SIGN, FMOD,
                SIN, COS, TAN,
                NUM_TYPES
        };

        enum Arity { NULLARY, UNARY, BINARY, TERNARY }; 
        Arity arity;
        Node(Arity a) : arity(a) {}
        virtual ~Node() {}


        virtual bool has_variable() const { return false; }
        virtual Node *simplify() const { return deep_copy(); }
        virtual value_t eval() const = 0;
        virtual value_t diff() const = 0;
        virtual Type type() const = 0;
        virtual string sexpr() const { return ""; }
        virtual string str() const { return ""; }
        virtual Node *copy() const = 0;
        virtual Node *deep_copy() const = 0;
        virtual bool is_variable() const { return false; }
};

struct Constant : Node {
        Constant() : Node(NULLARY), value(0) {}
        Constant(value_t n) : Node(NULLARY), value(n) {}

        value_t value;
        value_t eval() const { return value; }
        value_t diff() const { return 0; }
        Type type() const { return CONSTANT; }

        virtual string sexpr() const {
                stringstream s;
                s << value;
                return s.str();
        }

        virtual string str() const {
                stringstream s;
                s << value;
                return s.str();
        }


        Node *simplify() const { return copy(); }
        Node *copy() const { return new Constant(value); }
        Node *deep_copy() const { return copy(); }
};

struct Variable : Node {
        Variable() : Node(NULLARY), var(0), name(0) {}
        Variable(value_t *v, const char *n) : Node(NULLARY), var(v), name(n) {}

        value_t *var;
        const char *name;
        Node *simplify() const { return copy(); }
        bool has_variable() const { return true; } 
        value_t eval() const { return *var; }
        value_t diff() const { return 0; }
        Type type() const { return VARIABLE; }
        string sexpr() const { return name; }
        string str() const {return name; }
        Node *copy() const { return new Variable(var, name); }
        Node *deep_copy() const { return copy(); }
        virtual bool is_variable() const { return true; }
};

struct Unary : Node {
        Unary() : Node(UNARY), n(0) {}
        Unary(Node *c) : Node(UNARY), n(c) {}
        virtual ~Unary() { if (n) delete n; }
        Node *n;
        Node *deep_copy() const {
                Unary *x = (Unary*) copy();
                x->n = x->n->deep_copy();
                return x;
        }

        Node *simplify() const {
                Node *sn = n->simplify();
                if (sn->has_variable()) {
                        Unary *s = (Unary*) copy();
                        s->n = sn;
                        return s;
                }
                delete sn;
                Node::value_t v = eval();
                return new Constant(v);
        }
};

struct Binary : Node {
        Binary() : Node(BINARY), a(0), b(0) {}
        Binary(Node *ca, Node *cb) : Node(BINARY), a(ca), b(cb) {}
        virtual ~Binary() {
                if (a) delete a;
                if (b) delete b;
        }
        Node *a, *b;
        Node *deep_copy() const {
                Binary *x = (Binary*) copy();
                x->a = x->a->deep_copy();
                x->b = x->b->deep_copy();
                return x;
        }

        Node *simplify() const {
                Binary *s = (Binary*) copy();
                s->a = a->simplify();
                s->b = b->simplify();
                
                if (!s->a->has_variable() && s->b->has_variable()) {
                        value_t v = s->eval();
                        delete s;
                        return new Constant(v);
                }
                return s;
        }
        bool has_variable() const { return a->has_variable() || b->has_variable(); }
};

#define UN_OP(N, T, E, D, S, S2) \
        struct N : Unary { \
                N() : Unary(NULL) {} \
                N(Node *a) : Unary(a) {} \
                value_t eval() const { return E; }; \
                value_t diff() const { return D; }; \
                Type type() const { return T; } \
                string sexpr() const { stringstream s; s << S; return s.str(); } \
                string str() const { stringstream s; s << S2; return s.str(); } \
                Node *copy() const { return new N(n); } \
        };

UN_OP(Negate, NEGATE,
      -n->eval(),
      -n->diff(),
      "(neg " << n->sexpr() << ')',
      "(-(" << n->str() << "))");

UN_OP(Differential, DIFFERENTIAL,
      n->diff(),
      0,
      "(diff " << n->sexpr() << ')',
      "diff(" << n->str() << ')');

UN_OP(Floor, FLOOR,
      floor(n->eval()),
      0, // discontinous
      "(floor " << n->sexpr() << ')',
      "floor(" << n->str() << ')');

UN_OP(Ceil, CEIL,
      ceil(n->eval()),
      0, // discontinous
      "(ceil " << n->sexpr() << ')',
      "ceil(" << n->str() << ')');

template <typename T> int sgn(T val) {
            return (T(0) < val) - (val < T(0));
}

UN_OP(Sign, SIGN,
      sgn(n->eval()),
      0, // discontinous
      "(floor " << n->sexpr() << ')',
      "floor(" << n->str() << ')');

UN_OP(Sqrt, SQRT,
      sqrt(n->eval()),
      1/(2*sqrt(n->eval())),
      "(sqrt " << n->sexpr() << ')',
      "sqrt(" << n->str() << ')');


UN_OP(Log, LOGN,
      log(n->eval()),
      1/n->eval(),
      "(log " << n->sexpr() << ')',
      "log(" << n->str() << ')');

UN_OP(Exp, EXP,
      exp(n->eval()),
      exp(n->eval()),
      "(exp " << n->sexpr() << ')',
      "exp(" << n->str() << ')');

UN_OP(Sin, SIN,
      sin(n->eval()),
      cos(n->eval()),
      "(sin " << n->sexpr() << ')',
      "sin(" << n->str() << ')');

UN_OP(Cos, COS,
      cos(n->eval()),
      -sin(n->eval()),
      "(cos " << n->sexpr() << ')',
      "cos(" << n->str() << ')');

UN_OP(Tan, TAN,
      tan(n->eval()),
      1+pow(tan(n->eval()),2),
      "(tan " << n->sexpr() << ')',
      "tan(" << n->str() << ')');

#define BIN_OP(N, T, E, D, S, S2) \
        struct N : Binary { \
                N() : Binary(NULL, NULL) {} \
                N(Node *a, Node *b) : Binary(a,b) {} \
                value_t eval() const { return E; }; \
                value_t diff() const { return D; }; \
                Type type() const { return T; }; \
                string sexpr() const { stringstream s; s << S; return s.str(); } \
                string str() const { stringstream s; s << S2; return s.str(); } \
                Node *copy() const { return new N(a,b); } \
        };

BIN_OP(Add, ADD,
       a->eval() + b->eval(),
       a->diff() + b->diff(),
       "(+ " << a->sexpr() << ' ' << b->sexpr() << ')',
       '(' << a->str() << "+" << b->str() << ')');

BIN_OP(Subtract, SUBTRACT,
       a->eval() - b->eval(),
       a->diff() - b->diff(),
       "(- " << a->sexpr() << ' ' << b->sexpr() << ')',
       '(' << a->str() << "-" << b->str() << ')');

BIN_OP(Multiply, MULTIPLY,
       a->eval() * b->eval(),
       a->diff() * b->diff(),
       "(* " << a->sexpr() << ' ' << b->sexpr() << ')',
       '(' << a->str() << "*" << b->str() << ')');

BIN_OP(Divide, DIVIDE,
       a->eval() / b->eval(),
       a->diff() / b->diff(),
       "(/ " << a->sexpr() << ' ' << b->sexpr() << ')',
       '(' << a->str() << "/" << b->str() << ')');

BIN_OP(Power, POWER,
       pow(a->eval(), b->eval()),
       b->eval()*pow(a->eval(), b->eval()-1),
       "(pow " << a->sexpr() << ' ' << b->sexpr() << ')',
       '(' << a->str() << "^" << b->str() << ')');

BIN_OP(Fmod, FMOD,
       fmod(a->eval(), b->eval()),
       0, // discontinous
       "(fmod " << a->sexpr() << ' ' << b->sexpr() << ')',
       "fmod(" << a->str() << "," << b->str() << ')');

#endif // SYMBOLIC_H
