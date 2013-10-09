#include "contest.h"
#include "uct.h"
#include "game.h"
#include "random.h"
#include "minimax.h"
#include "montecarlo.h"
#include "td.h"
#include "connect6.h"


//----------------------------------------------------------------------------
//
// Choose your player. Options are:
//
//      Connect6UCT
//      Human<Connect6State>
//      Connect6Minimax
//      Connect6Negamax
//      Connect6Negascout
//      Connect6TD
//      Random<Connect6State>
//
//----------------------------------------------------------------------------

#define PLAYER_X Connect6TD
#define PLAYER_Y Connect6MC

//----------------------------------------------------------------------------
//
// Constants
//
//----------------------------------------------------------------------------

static const size_t SIZE         = 19,
                    MAX_MOVES    = binomial_coeff<SIZE*SIZE,2>::result,
                    MAX_ITER     = 8000,
                    TRAIN_ITER   = 100,
                    MAX_PLIES    = 9;


//----------------------------------------------------------------------------
//
// Life saving typedefs
//
//----------------------------------------------------------------------------

typedef connect6::State<SIZE,MAX_MOVES> Connect6State;

typedef UCT<    Connect6State,
                MAX_ITER,
                MAX_MOVES > Connect6UCT;

typedef TD<     Connect6State,
                SIZE*SIZE,
                (SIZE*SIZE) / 2> Connect6TD;

typedef MonteCarlo<
                Connect6State,
                MAX_MOVES,
                MAX_ITER> Connect6MC;

typedef BasicMinimax<
                Connect6State,
                Minimax<Connect6State>,
                MAX_PLIES > Connect6Minimax;

typedef BasicMinimax<
                Connect6State,
                Negamax<Connect6State>,
                MAX_PLIES > Connect6Negamax;

typedef BasicMinimax<
                Connect6State,
                Negascout<Connect6State>,
                MAX_PLIES > Connect6Negascout;

typedef Game<   Connect6State,
                PLAYER_X,
                PLAYER_Y > Connect6Game;

//----------------------------------------------------------------------------
//
// Static symbols
//
//----------------------------------------------------------------------------

template<> MemoryPool<Connect6UCT::Node> Connect6UCT::Node::pool(MAX_ITER);

//----------------------------------------------------------------------------
//
// MAIN
//
//----------------------------------------------------------------------------

void *test2(void *) {
        Connect6Game game;
        //game.set_param(BLACK, 0.1);
        //game.set_param(WHITE, 0.1);
        while (true) {
                game.black.pretrain(TRAIN_ITER);
                game.play();
        }
        return NULL;
}

void test3() {
        Contest<Connect6Game> c;
        c.run();
}

int main(int argc, char **argv) {
        LOG("sizeof(UCTNode) = " << sizeof(Connect6UCT::Node));
        srandom(time(NULL));
        test2(NULL);
        //test3();
        return 0;
}
