#include "contest.h"
#include "uct.h"
#include "game.h"
#include "human.h"
#include "random.h"
#include "montecarlo.h"
#include "td.h"
#include "minimax.h"
#include "connect4.h"

//----------------------------------------------------------------------------
//
// Choose your player. Options are:
//
//      Connect4UCT
//      Connect4MC
//      Human<Connect4State>
//      Connect4Minimax
//      Connect4Negamax
//      Connect4Negascout
//      Random<Connect4State>
//
//----------------------------------------------------------------------------

#define PLAYER_X Connect4TD // Human<Connect4State>
#define PLAYER_Y Random<Connect4State> //Connect4UCT// Human<Connect4State>


//----------------------------------------------------------------------------
//
// Constants
//
//----------------------------------------------------------------------------

// standard size = 7x6
static const size_t MAX_X        = 7,
                    MAX_Y        = 6,
                    MAX_ITER     = 1000000,
                    TRAIN_ITER   = 5000,
                    MAX_PLIES    = 9;


//----------------------------------------------------------------------------
//
// Life saving typedefs
//
//----------------------------------------------------------------------------

typedef connect4::State<MAX_X,MAX_Y> Connect4State;

typedef UCT<    Connect4State,
                MAX_ITER,
                MAX_X > Connect4UCT;

typedef TD<     Connect4State,
                MAX_X*MAX_Y*2,
                MAX_X*MAX_Y + MAX_X*MAX_Y/2> Connect4TD;


typedef MonteCarlo<
                Connect4State,
                MAX_X,
                MAX_ITER> Connect4MC;

typedef BasicMinimax<
                Connect4State,
                Minimax<Connect4State>,
                MAX_PLIES > Connect4Minimax;

typedef BasicMinimax<
                Connect4State,
                Negamax<Connect4State>,
                MAX_PLIES > Connect4Negamax;

typedef BasicMinimax<
                Connect4State,
                Negascout<Connect4State>,
                MAX_PLIES > Connect4Negascout;

typedef Game<   Connect4State,
                PLAYER_X,
                PLAYER_Y > Connect4Game;

//----------------------------------------------------------------------------
//
// Static symbols
//
//----------------------------------------------------------------------------

template<> MemoryPool<Connect4UCT::Node> Connect4UCT::Node::pool(MAX_ITER);


//----------------------------------------------------------------------------
//
// MAIN
//
//----------------------------------------------------------------------------

void *test2(void *) {
        Connect4Game game;
        game.set_param(BLACK, .5+sqrt(2));
        game.set_param(WHITE, .5+sqrt(2));
#if 1
        while (true) {
                game.black.pretrain(TRAIN_ITER);
                game.play();
        }
#else
        game.play();
#endif
        return NULL;
}

void test3() {
        Contest<Connect4Game> c;
        c.run();
}

int main(int argc, char **argv) {
        LOG("sizeof(UCTNode) = " << sizeof(Connect4UCT::Node));
        srandom(time(NULL));
        test2(NULL);
        //test3();
        return 0;
}
