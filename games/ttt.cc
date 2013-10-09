#include "contest.h"
#include "uct.h"
#include "game.h"
#include "random.h"
#include "montecarlo.h"
#include "minimax.h"
#include "human.h"
#define HINTON
#include "td.h"
#include "ttt.h"

//----------------------------------------------------------------------------
//
// Choose your player. Options are:
//
//      TTTTD
//      TTTUCT
//      Human<TTTState>
//      TTTMinimax
//      TTTNegamax
//      TTTNegascout
//      Random<TTTState>
//
//----------------------------------------------------------------------------

#define PLAYER_X TTTNegascout
#define PLAYER_Y TTTNegascout


//----------------------------------------------------------------------------
//
// Constants
//
//----------------------------------------------------------------------------

static const size_t SIZE         = 3,
                    MAX_ITER     = 1000000,
                    TRAIN_ITER   = 2000,
                    MAX_PLIES    = 9;


//----------------------------------------------------------------------------
//
// Life saving typedefs
//
//----------------------------------------------------------------------------

typedef ttt::State<SIZE> TTTState;

typedef UCT<    TTTState,
                MAX_ITER,
                TTTState::Board::AREA > TTTUCT;

typedef TD<     TTTState,
                SIZE*SIZE*2,
                SIZE*SIZE,
                Sigmoid,
                Sigmoid> TTTTD;

typedef MonteCarlo<
                TTTState,
                SIZE*SIZE,
                MAX_ITER> TTTMC;

typedef BasicMinimax<
                TTTState,
                Minimax<TTTState>,
                MAX_PLIES > TTTMinimax;

typedef BasicMinimax<
                TTTState,
                Negamax<TTTState>,
                MAX_PLIES > TTTNegamax;

typedef BasicMinimax<
                TTTState,
                Negascout<TTTState>,
                MAX_PLIES > TTTNegascout;

typedef Game<   TTTState,
                PLAYER_X,
                PLAYER_Y > TTTGame;

//----------------------------------------------------------------------------
//
// Static symbols
//
//----------------------------------------------------------------------------

template<> MemoryPool<TTTUCT::Node> TTTUCT::Node::pool(MAX_ITER);


//----------------------------------------------------------------------------
//
// MAIN
//
//----------------------------------------------------------------------------

void *test2(void *) {
        TTTGame game;
#if 1
        while (true) {
                //game.black.pretrain(TRAIN_ITER);
                game.play();
        }
#else
        game.play();
#endif
        return NULL;
}

void test3() {
        Contest<TTTGame> c;
        c.run();
}

int main(int argc, char **argv) {
        LOG("sizeof(UCTNode) = " << sizeof(TTTUCT::Node));
        srandom(time(NULL));
        test2(NULL);
        //test3();
        return 0;
}
