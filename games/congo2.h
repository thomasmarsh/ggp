#ifndef CONGO_H
#define CONGO_H

#include "bitboard.h"

namespace congo {

typedef uint64_t u64;

static const char *PIECE_LABEL=".gmelczps";

static const size_t NUM_PIECE_IDS   = 15,
                    NUM_POSITIONS   = 55,
                    NUM_PIECE_TYPES = 7,
                    MAX_MOVES       = 144;


enum Piece { EMPTY, GIRAFFE, MONKEY, ELEPHANT, LION, CROCODILE, ZEBRA, PAWN, SUPERPAWN };


static const u64 INVALID       = 0xFF80808080808080,
                 RIVER         = 0x000000007F000000,
                 BLACK_PROMOTE = 0x007F000000000000,
                 WHITE_PROMOTE = 0x000000000000007F,
                 BLACK_CASTLE  = 0x00000000001C1C1C,
                 WHITE_CASTLE  = 0x001C1C1C00000000;

static u64 ADJACENT_MOVES[NUM_POSITIONS],
                GIRAFFE_JUMPS[NUM_POSITIONS],
                ELEPHANT_MOVES[NUM_POSITIONS],
                BLACK_LION_MOVES[NUM_POSITIONS],
                WHITE_LION_MOVES[NUM_POSITIONS],
                ZEBRA_MOVES[NUM_POSITIONS],
                BLACK_PAWN_MOVES[NUM_POSITIONS],
                WHITE_PAWN_MOVES[NUM_POSITIONS],
                BLACK_SPAWN_MOVES[NUM_POSITIONS],
                WHITE_SPAWN_MOVES[NUM_POSITIONS],
                BLACK_PAWN_BACK[NUM_POSITIONS],
                WHITE_PAWN_BACK[NUM_POSITIONS],
                BLACK_SPAWN_BACK[NUM_POSITIONS],
                WHITE_SPAWN_BACK[NUM_POSITIONS],
                CROC_MOVES[NUM_POSITIONS];

static u64 LION_SIGHT[9][9];


//-----------------------------------------------------------------------------
//
// Move
//
//-----------------------------------------------------------------------------

#pragma pack(1)
struct Move {
        uint8_t a:6, b:6;
        Color c:2;
        bool is_capture:1;
};

#pragma pack(1)
struct MoveList {
        uint8_t count;
        Move move[MAX_MOVES];

        size_t size() { return count; }
        void clear() { memset(this, 0, sizeof(MoveList)); }
        MoveList() { clear(); }
        Move& operator[](size_t i) { return move[i]; }

        void _add(Color c, bitboard bb, int a, int b) {
                move[count].a = a;
                move[count].b = b;
                move[count].c = c;
                move[count].is_capture = ((bb.data & (1ULL << b)) > 0);
                ++count;
        }

        void add(Color c, uint8_t square[], bitboard bb, int k, u64 d) {
                u64 j, n=d;
                while (n) {
                        j = n & (~n+1);
                        _add(c, bb, k, log2(j));
                        n ^= j;
                }
        }

        void print() {
                for (size_t i=0; i < count; ++i)
                        cout << move[i].a << " -> " << move[i].b << endl;
        }
};

//-----------------------------------------------------------------------------
//
// Move generators
//
//-----------------------------------------------------------------------------

static inline void gen_adjacent(bitboard &b, int x, int y) {
        b.try_set(x-1, y-1); b.try_set(x,   y-1); b.try_set(x+1, y-1); 
        b.try_set(x-1, y);   b.try_set(x+1, y);
        b.try_set(x-1, y+1); b.try_set(x,   y+1); b.try_set(x+1, y+1);
}

static inline void gen_zebra(bitboard &b, int x, int y) {
        b.try_set(x-2, y-1); b.try_set(x-1, y-2);
        b.try_set(x+1, y-2); b.try_set(x+2, y-1);

        b.try_set(x-2, y+1); b.try_set(x-1, y+2);
        b.try_set(x+1, y+2); b.try_set(x+2, y+1);
}

static inline void gen_elephant(bitboard &b, int x, int y) {
        b.try_set(x-2, y); b.try_set(x-1, y);
        b.try_set(x+1, y); b.try_set(x+2, y);
        b.try_set(x, y-2); b.try_set(x, y-1);
        b.try_set(x, y+1); b.try_set(x, y+2);
}

static inline void gen_giraffe_attack(bitboard &b, int x, int y) {
        b.try_set(x-2, y-2); b.try_set(x,   y-2); b.try_set(x+2, y-2);
        b.try_set(x-2, y);   b.try_set(x+2, y);
        b.try_set(x-2, y+2); b.try_set(x,   y+2); b.try_set(x+2, y+2);
}

static inline void gen_pawn_attack(bitboard &b, int d, int x, int y) {
        b.try_set(x-1, y+d); b.try_set(x,   y+d); b.try_set(x+1, y+d);
}

static inline void gen_pawn_retreat(bitboard &b, int d, int x, int y) {
        switch (d) {
        case 1:
                if (y > 3) {
                        b.try_set(x, y-d);
                        b.try_set(x, y-(2*d));
                }
                break;
        case -1:
                if (y < 3) {
                        b.try_set(x, y-d);
                        b.try_set(x, y-(2*d));
                }
                break;
        }
}

static inline void gen_superpawn_attack(bitboard &b, int d, int x, int y) {
        b.try_set(x-1, y+d); b.try_set(x,   y+d); b.try_set(x+1, y+d);
        b.try_set(x-1, y); b.try_set(x+1, y);
}

static inline void gen_superpawn_retreat(bitboard &b, int d, int x, int y) {
        b.try_set(x, y-d); b.try_set(x, y-(2*d));
        b.try_set(x-1, y-d); b.try_set(x-2, y-(2*d));
        b.try_set(x+1, y-d); b.try_set(x+2, y-(2*d));
}

static inline void gen_croc(bitboard &b, int x, int y) {
        int ty = y-1;
        while (ty > 2) {
                b.try_set(x, ty);
                --ty;
        }
        ty = y+1;
        while (ty < 4) {
                b.try_set(x, ty);
                ++ty;
        }
        if (y == 3) {
                int tx = x-1;
                while (tx > 0) {
                        b.try_set(tx, y);
                        --tx;
                }
                tx = x+1;
                while (tx < 7) {
                        b.try_set(tx, y);
                        ++tx;
                }
        }
}

static inline void pop_lion_sight() {
        for (int b=0; b < 9; ++b) {
                for (int w=0; w < 9; ++w) {
                        int by=  (b/3), bx=2+(b%3),
                            wy=4+(w/3), wx=2+(w%3);

                        bitboard d;
                        if (wx == bx) {
                                for (int y=by+1; y < wy; ++y)
                                        d.set(bx, y);
                        } else if ((b==6 && w==2) || (b==8 && w==0))
                                d.iset(27);


                        LION_SIGHT[b][w] = d.data;
                }
        }
}


static inline void populate() {
        int i;
        for (int y=0; y < 7; ++y) {
                for (int x=0; x < 7; ++x) {
                        i = x + (y<<3);
                        bitboard am, cm, gj, em, zm, bpm, wpm, bsm, wsm;
                        bitboard tmp;

                        gen_adjacent(am, x, y);
                        gen_giraffe_attack(gj, x, y);
                        gen_elephant(em, x, y);
                        gen_zebra(zm, x, y);
                        gen_croc(cm, x, y);

                        ADJACENT_MOVES[i] = am.data;
                        GIRAFFE_JUMPS[i] = gj.data;
                        ELEPHANT_MOVES[i] = em.data;
                        ZEBRA_MOVES[i] = zm.data;
                        CROC_MOVES[i] = cm.data;

                        tmp.data = 0;
                        tmp.iset(i);
                        BLACK_LION_MOVES[i] = (BLACK_CASTLE & tmp.data) ? (am.data & BLACK_CASTLE) : 0;
                        WHITE_LION_MOVES[i] = (WHITE_CASTLE & tmp.data) ? (am.data & WHITE_CASTLE) : 0;

                        gen_pawn_attack(bpm, 1, x, y);
                        gen_pawn_attack(wpm, -1, x, y);
                        gen_superpawn_attack(bsm, 1, x, y);
                        gen_superpawn_attack(wsm, -1, x, y);

                        BLACK_PAWN_MOVES[i] = bpm.data;
                        WHITE_PAWN_MOVES[i] = wpm.data;
                        BLACK_SPAWN_MOVES[i] = bsm.data;
                        WHITE_SPAWN_MOVES[i] = wsm.data;

                        bpm.data = 0; wpm.data = 0;
                        bsm.data = 0; wsm.data = 0;

                        gen_pawn_retreat(bpm, 1, x, y);
                        gen_pawn_retreat(wpm, -1, x, y);
                        gen_superpawn_retreat(bsm, 1, x, y);
                        gen_superpawn_retreat(wsm, -1, x, y);

                        BLACK_PAWN_BACK[i] = bpm.data;
                        WHITE_PAWN_BACK[i] = wpm.data;
                        BLACK_SPAWN_BACK[i] = bsm.data;
                        WHITE_SPAWN_BACK[i] = wsm.data;
                }
        }
        pop_lion_sight();
}


//-----------------------------------------------------------------------------
//
// Game state
//
//-----------------------------------------------------------------------------

#pragma pack(1)
struct State {
        typedef Move M;
        typedef MoveList ML;

#pragma pack(1)
        struct Position {
                bitboard giraffe, monkey, elephants, lion, crocodile, zebra, pawns, superpawns;

                bitboard& board(Piece p) {
                        switch (p) {
                        case GIRAFFE:   return giraffe;
                        case MONKEY:    return monkey;
                        case ELEPHANT:  return elephants;
                        case LION:      return lion;
                        case CROCODILE: return crocodile;
                        case ZEBRA:     return zebra;
                        case PAWN:      return pawns;
                        case SUPERPAWN: return superpawns;
                        case EMPTY:
                        default: DIE("corrupt piece: " << p);
                        }
                }

                void set(Piece p, int i) { board(p).iset(i); }
                void clear(Piece p, int i) { board(p).iclear(i); }
        };

        uint8_t river_black, river_white, river_square[8];
        Position black, white;
        bitboard occupied, occupied_black, occupied_white;
        uint8_t square[NUM_POSITIONS];
        // TODO
        uint8_t black_lion, white_lion;
        Color _winner:2, _just_played:2;

        Color winner() { return _winner; }
        Color just_played() { return _just_played; }
        Color current() { return other(_just_played); }

        // Note: river could be stored as uint32_t, saving about 2 bytes and
        // reducing to simple integer operations.
        //
        //   piece id = 3 bits 
        //   color    = 1 bit
        //
        //  = 4 bits * num_columns(7) = 28 bits total
        // 
        // A current/prev mask could be maintained to check whether the river
        // has changed. Crocs would not be stored on the masks. No memcpy, and
        // no color arrays to maintain.
        //
        // TODO: prevent pieces form avoiding drowing by moving sideways in
        // river.

        void clear() { memset(this, 0, sizeof(State)); }
        void copy_from(const State &rhs) { memcpy(this, &rhs, sizeof(State)); }


        State() {
                clear();
                initial();
        }

        State(const State &rhs) { copy_from(rhs); }

        bool game_over() { return !black.lion.data || !white.lion.data; }
        void set_game_over() { black.lion.data = 0; white.lion.data = 0; }

        void copy_river() {
                memcpy(river_square, square+24, 8*sizeof(uint8_t));
                river_black = (occupied_black.data & RIVER) >> 24;
                river_white = (occupied_white.data & RIVER) >> 24;
        }

        void place(Color c, Piece p, int i) {
                square[i] = p;
                occupied.iset(i);
                switch (c) {
                case BLACK:
                        black.set(p, i);
                        occupied_black.iset(i);
                        black_lion = (p==LION) ? i : black_lion;
                        break;
                case WHITE:
                        white.set(p, i);
                        occupied_white.iset(i);
                        white_lion = (p==LION) ? i : white_lion;
                        break;
                case NONE:
                        DIE("corrupt color: " << c);
                }
        }

        void remove(Color c, Piece p, int i) {
                square[i] = EMPTY;
                occupied.iclear(i);
                switch (c) {
                case BLACK:
                        black.clear(p, i);
                        occupied_black.iclear(i);
                        break;
                case WHITE:
                        white.clear(p, i);
                        occupied_white.iclear(i);
                        break;
                case NONE:
                        DIE("corrupt color: " << c);
                }
        }

        Color color(int i) {
                return occupied_black.iis_set(i) ? BLACK :
                                (occupied_white.iis_set(i) ? WHITE : NONE);
        }

        void promote(Color c, u64 promo, u64 data, int x) {
                u64 j, n=data&promo;
                int i;
                while (n) {
                        j = n & (~n+1);
                        i = log2(j);
                        remove(c, PAWN, i);
                        place(c, SUPERPAWN, i);
                        n ^= j;
                }
        }

        void promote() {
                promote(BLACK, BLACK_PROMOTE, black.pawns.data, 48);
                promote(WHITE, WHITE_PROMOTE, white.pawns.data, 0);
        }

        void drown(Color played) {
                u64 b = river_black << 24,
                         w = river_white << 24;

                bitboard tmp;
                if ((b & (RIVER & occupied_black.data)) || (w & (RIVER & occupied_white.data))) {
                        for (int i=0,j=24; i < 7; ++i,++j) {
                                Color c = color(j);
                                if (square[j] != CROCODILE && c == played) {
                                        tmp.data = (c == BLACK) ? b : w;
                                        if (tmp.iis_set(j) && (river_square[i] == square[j]))
                                                remove(c, (Piece) square[j], j);
                                }
                        }
                }
                copy_river();
        }

        void check_win_cond() {
                if (!black.lion.data) _winner = WHITE;
                else if (!white.lion.data) _winner = BLACK;
        }

        string move_str(const Move &m) const {
                const char *xl="abcdefg",
                           *yl="7654321";

                stringstream s;
                Color player = other(_just_played);
                s << ColorStr(player) << " "
                  << label(player, (Piece) square[m.a]) << " "
                  << xl[m.a%8] << yl[m.a/8] << '-'
                  << xl[m.b%8] << yl[m.b/8];
                //if (m.capture)
                 //       s << " x" << label(_just_played, (Piece) square[m.b]);
                return s.str();
        }

        void announce(const Move &m) const {
                LOG(move_str(m));
        }

        void move(const Move &m) {
                Color ca = color(m.a),
                      cb = color(m.b);

                if (occupied.iis_set(m.b)) {
                        remove(cb, (Piece) square[m.b], m.b);
                }
                place(ca, (Piece) square[m.a], m.b);
                remove(ca, (Piece) square[m.a], m.a);
                promote();
                drown(ca);
                check_win_cond();
                _just_played = ca;
        }

        u64 lion_sight(Color c) {
                if (white_lion < 34 || black_lion > 20)
                        return 0;

                int b = ((black_lion%8)-2)+(black_lion>>3)*3,
                    w = ((white_lion%8)-2)+((white_lion-32)>>3)*3;

                u64 p = LION_SIGHT[b][w];
                if ((p>0) && (p==(~occupied.data&p)))
                        return (c==BLACK) ? white.lion.data : black.lion.data;
                return 0;
        }

        u64 moves(Color c, int i) {
                u64 tmp;
                switch (square[i]) {
                case GIRAFFE:
                        return (ADJACENT_MOVES[i] & ~occupied.data) | GIRAFFE_JUMPS[i];

                case MONKEY:
                        return (ADJACENT_MOVES[i] & ~occupied.data);
                        // TODO : attacks
                        break;

                case ELEPHANT:
                        return ELEPHANT_MOVES[i];

                case LION:
                        switch (c) {
                        case BLACK: return lion_sight(c) | BLACK_LION_MOVES[i];
                        case WHITE: return lion_sight(c) | WHITE_LION_MOVES[i];
                        case NONE: DIE("no color passed");
                        default: DIE("corrupt color: " << c);
                        }
                        break;

                case CROCODILE:
                        tmp = CROC_MOVES[i];
                        if (tmp != (tmp & ~occupied.data))
                                tmp = 0;
                        return tmp | ADJACENT_MOVES[i];
                        // TODO : rook slides with intervening pieces
                        break;

                case ZEBRA:
                        return ZEBRA_MOVES[i];

                case PAWN:
                        // TODO : retreat with intervening pieces
                        switch (c) {
                        case BLACK:
                                tmp = BLACK_PAWN_BACK[i];
                                if (tmp != (tmp & ~occupied.data))
                                        tmp = 0;
                                return BLACK_PAWN_MOVES[i] | tmp;
                        case WHITE:
                                tmp = WHITE_PAWN_BACK[i];
                                if (tmp != (tmp & ~occupied.data))
                                        tmp = 0;
                                return WHITE_PAWN_MOVES[i] | tmp;
                        case NONE: DIE("no color passed");
                        default: DIE("corrupt color: " << c);
                        }
                        break;
                case SUPERPAWN:
                        // TODO : retreat with intervening pieces
                        switch (c) {
                        case BLACK:
                                tmp = BLACK_SPAWN_BACK[i];
                                if (tmp != (tmp & ~occupied.data))
                                        tmp = 0;
                                return BLACK_SPAWN_MOVES[i] | tmp;
                        case WHITE:
                                tmp = WHITE_SPAWN_BACK[i];
                                if (tmp != (tmp & ~occupied.data))
                                        tmp = 0;
                                return WHITE_SPAWN_MOVES[i] | tmp;
                        case NONE: DIE("no color passed");
                        default: DIE("corrupt color: " << c);
                        }
                        break;
                }
                return 0;
        }

        bool random_move(MoveList &ml, Move &m) {
                ml.clear();
                moves(ml);
                if (!ml.size())
                        return false;
                memcpy(&m, &ml.move[random() % ml.size()], sizeof(Move));
                return true;
        }

        void moves(MoveList &l, Color c) {
                u64 j, friendly=((c==BLACK)?occupied_black.data:occupied_white.data),
                         n = occupied.data;
                int i;
                while (n) {
                        j = n & (~n+1);
                        i = log2(j);
                        if (color(i) == c)
                                l.add(c, square, occupied, i, moves(c, i) & ~friendly);
                        n ^= j;
                }
        }

        void moves(MoveList &ml) {
                Color player = other(_just_played);
                moves(ml, player);
        }

        int mobility() {
                MoveList b, w;
                moves(b, BLACK);
                moves(w, WHITE);
                return b.size() - w.size();
        }

        int diff(Piece p) {
                return popcount(black.board(p).data) - popcount(white.board(p).data);
        }

        float result(Color c) {
                if (_winner == c) return 1.0;
                else if (_winner == other(c)) return 0;
                return 0.5;
        }

        int score(bool maximise) {
                return 1000 * diff(LION)
                     +    2 * diff(PAWN)
                     +    3 * diff(ZEBRA)
                     +    4 * diff(GIRAFFE)
                     +    4 * diff(ELEPHANT)
                     +    4 * diff(MONKEY)
                     +    4 * diff(CROCODILE)
                     +    4 * diff(SUPERPAWN)
                     +    (mobility() >> 1);
        }

        void initial() {
                clear();
                place(BLACK, GIRAFFE,   0);
                place(BLACK, MONKEY,    1);
                place(BLACK, ELEPHANT,  2);
                place(BLACK, LION,      3);
                place(BLACK, ELEPHANT,  4);
                place(BLACK, CROCODILE, 5);
                place(BLACK, ZEBRA,     6);

                for (int i=8; i < 15; ++i)
                        place(BLACK, PAWN, i);
                for (int i=40; i < 47; ++i)
                        place(WHITE, PAWN, i);

                place(WHITE, GIRAFFE,   48);
                place(WHITE, MONKEY,    49);
                place(WHITE, ELEPHANT,  50);
                place(WHITE, LION,      51);
                place(WHITE, ELEPHANT,  52);
                place(WHITE, CROCODILE, 53);
                place(WHITE, ZEBRA,     54);

                _winner = NONE;
                _just_played = WHITE;
        }


        char label(Color c, Piece p) const {
                if (c == BLACK)
                        return toupper(PIECE_LABEL[p]);
                return PIECE_LABEL[p];
        }

        string str() {
                stringstream s;
                for (int y=0; y < 7; ++y) {
                        for (int x=0; x < 7; ++x) {
                                int i = x + y*8;
                                if (occupied_black.iis_set(i))
                                        s << label(BLACK, (Piece) square[i]);
                                else if (occupied_white.iis_set(i))
                                        s << label(WHITE, (Piece) square[i]);
                                else  s << ((y == 3) ? '~' : '-');
                        }
                        s << endl;
                }
                return s.str();
        }

        void print() {
                cout << str() << flush;
        }
};

} // namespace congo

#endif // CONGO_H
