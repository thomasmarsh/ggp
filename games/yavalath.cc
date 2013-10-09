#include "contest.h"
#include "uct.h"
#include "game.h"
#include "random.h"
#include "minimax.h"
#include "montecarlo.h"
#define HINTON
#include "td.h"
#include "yavalath.h"

//----------------------------------------------------------------------------
//
// Choose your player. Options are:
//
//      YavalathUCT
//      Human<YavalathState>
//      YavalathMinimax
//      YavalathNegamax
//      YavalathNegascout
//      Random<YavalathState>
//
//----------------------------------------------------------------------------

#define PLAYER_X YavalathTD
#define PLAYER_Y YavalathUCT


//----------------------------------------------------------------------------
//
// Constants
//
//----------------------------------------------------------------------------

static const size_t SIZE         = 4, // 5
                    TARGET_N     = 3, // 4
                    MAX_ITER     = 100000,
                    MAX_PLIES    = 9,
                    TRAIN_ITER   = 5000;


//----------------------------------------------------------------------------
//
// Life saving typedefs
//
//----------------------------------------------------------------------------

typedef yavalath::State<TARGET_N, SIZE> YavalathState;

typedef UCT<    YavalathState,
                MAX_ITER,
                YavalathState::Board::AREA > YavalathUCT;

typedef TD<     YavalathState,
                YavalathState::Board::AREA,
                YavalathState::Board::AREA> YavalathTD;

typedef MonteCarlo<
                YavalathState,
                YavalathState::Board::AREA,
                MAX_ITER> YavalathMC;

typedef BasicMinimax<
                YavalathState,
                Minimax<YavalathState>,
                MAX_PLIES > YavalathMinimax;

typedef BasicMinimax<
                YavalathState,
                Negamax<YavalathState>,
                MAX_PLIES > YavalathNegamax;

typedef BasicMinimax<
                YavalathState,
                Negascout<YavalathState>,
                MAX_PLIES > YavalathNegascout;

typedef Game<   YavalathState,
                PLAYER_X,
                PLAYER_Y > YavalathGame;

//----------------------------------------------------------------------------
//
// Static symbols
//
//----------------------------------------------------------------------------

template<> MemoryPool<YavalathUCT::Node> YavalathUCT::Node::pool(MAX_ITER);


//----------------------------------------------------------------------------
//
// MAIN
//
//----------------------------------------------------------------------------

void *test2(void *) {
        YavalathGame game;
#if 0
        // Board size = 5
        game.set_param(BLACK, 11);
        game.set_param(WHITE, 11);
#endif
#if 1
        // Board size = 5
        game.set_param(BLACK, 20);
        game.set_param(WHITE, 20);
#endif
        while (true) {
                LOG("pretrain");
                game.black.learning_rate = 0.3;
                game.black.pretrain(TRAIN_ITER);
                LOG("play");
                game.play();
        }
        return NULL;
}

void test3() {
        Contest<YavalathGame> c;
        c.run();
}

int main(int argc, char **argv) {
        LOG("sizeof(UCTNode) = " << sizeof(YavalathUCT::Node));
        srandom(time(NULL));
        test2(NULL);
        //test3();
        return 0;
}
