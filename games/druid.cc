#include "contest.h"
#include "uct.h"
#include "game.h"
#include "druid.h"
#include "random.h"
//#include "display.h"
#include "minimax.h"
#include "human.h"
#include "montecarlo.h"

//----------------------------------------------------------------------------
//
// Choose your player. Options are:
//
//      DruidUCT
//      DruidMC
//      DruidMinimax
//      DruidNegamax
//      DruidNegascout
//      Human<DruidState>
//      Random<DruidState>
//
//----------------------------------------------------------------------------

#define PLAYER_X DruidUCT
#define PLAYER_Y Human<DruidState>
//#define PLAYER_X DruidUCT
//#define PLAYER_Y DruidNegascout


//----------------------------------------------------------------------------
//
// Constants
//
//----------------------------------------------------------------------------

static const size_t SIZE         = 5,
                    MAX_MOVES    = 120,
                    MAX_ITER     = 6000000,
                    NUM_PLIES    = 6;


//----------------------------------------------------------------------------
//
// Life saving typedefs
//
//----------------------------------------------------------------------------

// Druid

typedef druid::State<
                SIZE,
                MAX_MOVES > DruidState;

typedef UCT<    DruidState,
                MAX_ITER,
                MAX_MOVES > DruidUCT;

typedef MonteCarlo<
                DruidState,
                MAX_MOVES,
                MAX_ITER> DruidMC;

typedef BasicMinimax<
                DruidState,
                Minimax<DruidState>,
                NUM_PLIES > DruidMinimax;

typedef BasicMinimax<
                DruidState,
                Negamax<DruidState>,
                NUM_PLIES > DruidNegamax;

typedef BasicMinimax<
                DruidState,
                Negascout<DruidState>,
                NUM_PLIES > DruidNegascout;

typedef Game<   DruidState,
                PLAYER_X,
                PLAYER_Y > DruidGame;


//----------------------------------------------------------------------------
//
// Static symbols
//
//----------------------------------------------------------------------------

template<> MemoryPool<DruidUCT::Node> DruidUCT::Node::pool(MAX_ITER);
template<> SquarePathFinder<SIZE> DruidState::path_finder(0);


//----------------------------------------------------------------------------
//
// MAIN
//
//----------------------------------------------------------------------------

void *test2(void *) {
        DruidGame game;
        game.set_param(BLACK, 3.8);
        game.set_param(WHITE, 3.8);
        game.play();
        return NULL;
}

void test3() {
        Contest<DruidGame> c;
        c.run();
}

int main(int argc, char **argv) {
        LOG("sizeof(UCTNode) = " << sizeof(DruidUCT::Node));
        srandom(time(NULL));
        test2(NULL);
        //test3();
        return 0;
}
