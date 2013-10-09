#include "contest.h"
#include "uct.h"
#include "game.h"
#include "random.h"
#include "minimax.h"
#include "montecarlo.h"
#include "td.h"
#include "tanbo.h"


//----------------------------------------------------------------------------
//
// Choose your player. Options are:
//
//      TanboUCT
//      Human<TanboState>
//      TanboMinimax
//      TanboNegamax
//      TanboNegascout
//      TanboTD
//      Random<TanboState>
//
//----------------------------------------------------------------------------

#define PLAYER_X TanboUCT
#define PLAYER_Y Random<TanboState> //TanboNegascout

//----------------------------------------------------------------------------
//
// Constants
//
//----------------------------------------------------------------------------

static const size_t SIZE         = 9, // 9, 13, or 19
                    MAX_ITER     = 1000,
                    TRAIN_ITER   = 1000,
                    MAX_PLIES    = 4;


//----------------------------------------------------------------------------
//
// Life saving typedefs
//
//----------------------------------------------------------------------------

typedef tanbo::State<SIZE,SIZE*SIZE> TanboState;

typedef UCT<    TanboState,
                MAX_ITER,
                SIZE*SIZE > TanboUCT;

typedef TD<     TanboState,
                SIZE*SIZE,
                (SIZE*SIZE) / 2> TanboTD;

typedef MonteCarlo<
                TanboState,
                SIZE*SIZE,
                MAX_ITER> TanboMC;

typedef BasicMinimax<
                TanboState,
                Minimax<TanboState>,
                MAX_PLIES > TanboMinimax;

typedef BasicMinimax<
                TanboState,
                Negamax<TanboState>,
                MAX_PLIES > TanboNegamax;

typedef BasicMinimax<
                TanboState,
                Negascout<TanboState>,
                MAX_PLIES > TanboNegascout;

typedef Game<   TanboState,
                PLAYER_X,
                PLAYER_Y > TanboGame;

//----------------------------------------------------------------------------
//
// Static symbols
//
//----------------------------------------------------------------------------

template<> MemoryPool<TanboUCT::Node> TanboUCT::Node::pool(MAX_ITER);

//----------------------------------------------------------------------------
//
// MAIN
//
//----------------------------------------------------------------------------

void *test2(void *) {
        TanboGame game;
        //game.set_param(BLACK, 0.1);
        //game.set_param(WHITE, 0.1);
        while (true) {
                game.black.pretrain(TRAIN_ITER);
                game.play();
                game.state.print();
        }
        game.play();
        while (true) game.state.print();
        return NULL;
}

void test3() {
        Contest<TanboGame> c;
        c.run();
}

int main(int argc, char **argv) {
        LOG("sizeof(UCTNode) = " << sizeof(TanboUCT::Node));
        srandom(time(NULL));
        test2(NULL);
        //test3();
        return 0;
}
