#include "contest.h"
#include "uct.h"
#include "game.h"
#include "random.h"
#include "minimax.h"
#include "congo2.h"

//----------------------------------------------------------------------------
//
// Choose your player. Options are:
//
//      CongoUCT
//      CongoMinimax
//      CongoNegamax
//      CongoNegascout
//      Human<congo::State>
//      Random<congo::State>
//
//----------------------------------------------------------------------------

#define PLAYER_X CongoUCT
#define PLAYER_Y CongoUCT


//----------------------------------------------------------------------------
//
// Constants
//
//----------------------------------------------------------------------------

static const size_t GAME_SIZE = 1000000,
                    NUM_PLIES = 7;


//----------------------------------------------------------------------------
//
// Life saving typedefs
//
//----------------------------------------------------------------------------

typedef UCT<    congo::State,
                GAME_SIZE,
                congo::MAX_MOVES > CongoUCT;

typedef BasicMinimax<
                congo::State,
                Minimax<congo::State>,
                NUM_PLIES> CongoMinimax;

typedef BasicMinimax<
                congo::State,
                Negamax<congo::State>,
                NUM_PLIES> CongoNegamax;

typedef BasicMinimax<
                congo::State,
                Negascout<congo::State>,
                NUM_PLIES> CongoNegascout;

typedef Game<   congo::State,
                PLAYER_X,
                PLAYER_Y > CongoGame;


//----------------------------------------------------------------------------
//
// Static symbols
//
//----------------------------------------------------------------------------

template<> MemoryPool<CongoUCT::Node> CongoUCT::Node::pool(GAME_SIZE+1);


//----------------------------------------------------------------------------
//
// MAIN
//
//----------------------------------------------------------------------------

void *test2(void *) {
        CongoGame game;
        game.play();
        return NULL;
}

void test3() {
        Contest<CongoGame> c;
        c.run();
}

int main(int argc, char **argv) {
        LOG("sizeof(UCTNode) = " << sizeof(CongoUCT::Node));
        srandom(time(NULL));
        congo::populate();
        test2(NULL);
        //test3();
        return 0;
}
