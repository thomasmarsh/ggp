namespace chad {

class State {
        enum { SIZE=12, AREA=SIZE*SIZE };

        bitset<AREA> black_rook;
        bitset<AREA> white_rook;
        uint8_t black_king, white_king;



