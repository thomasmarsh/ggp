#include "contest.h"
#include "uct.h"
#include "game.h"
#include "druidhex.h"
#include "random.h"
//#include "display.h"
#include "minimax.h"

//----------------------------------------------------------------------------
//
// Choose your player. Options are:
//
//      DruidHexUCT
//      DruidHexMinimax
//      DruidHexNegamax
//      DruidHexNegascout
//      Human<DruidHexState>
//      Random<DruidHexState>
//
//----------------------------------------------------------------------------

#define PLAYER_X DruidHexUCT
#define PLAYER_Y DruidHexUCT


//----------------------------------------------------------------------------
//
// Constants
//
//----------------------------------------------------------------------------

static const size_t SIZE         = 5,
                    NUM_SARSENS  = 254,
                    NUM_LINTELS  = (NUM_SARSENS >> 1)+1,
                    MAX_MOVES    = 254,
                    GAME_SIZE    = 50000,
                    NUM_PLIES    = 6;


//----------------------------------------------------------------------------
//
// Life saving typedefs
//
//----------------------------------------------------------------------------

// DruidHex

typedef druidhex::State<
                SIZE,
                MAX_MOVES,
                NUM_SARSENS,
                NUM_LINTELS > DruidHexState;

typedef UCT<    DruidHexState,
                GAME_SIZE,
                MAX_MOVES > DruidHexUCT;

typedef BasicMinimax<
                DruidHexState,
                Minimax<DruidHexState>,
                NUM_PLIES > DruidHexMinimax;

typedef BasicMinimax<
                DruidHexState,
                Negamax<DruidHexState>,
                NUM_PLIES > DruidHexNegamax;

typedef BasicMinimax<
                DruidHexState,
                Negascout<DruidHexState>,
                NUM_PLIES > DruidHexNegascout;

typedef Game<   DruidHexState,
                PLAYER_X,
                PLAYER_Y > DruidHexGame;


//----------------------------------------------------------------------------
//
// Static symbols
//
//----------------------------------------------------------------------------

template<> MemoryPool<DruidHexUCT::Node> DruidHexUCT::Node::pool(GAME_SIZE+1);
template<> druidhex::HexPathFinder<SIZE> DruidHexState::path_finder(0);


//----------------------------------------------------------------------------
//
// MAIN
//
//----------------------------------------------------------------------------

void *test2(void *) {
        DruidHexGame game;
        game.play();
        return NULL;
}

void test3() {
        Contest<DruidHexGame> c;
        c.run();
}

int main(int argc, char **argv) {
        LOG("sizeof(UCTNode) = " << sizeof(DruidHexUCT::Node));
        srandom(time(NULL));
        test2(NULL);
        //test3();
        return 0;
}
