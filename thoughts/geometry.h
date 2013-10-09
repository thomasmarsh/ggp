namespace tiling {

// regular tiling
struct Hex {
        enum { ADJACENT = 6 };
        enum { INDIRECT = 0 };

        enum Direction { N, NE, SE, S, SW, NW };
};

struct Triangle {
        enum { ADJACENT = 3 };
        enum { INDIRECT = 9 };

        enum Direction { A, B, C };
        enum Orientation { UP, DOWN };
};

struct Square {
        enum { ADJACENT = 4 };
        enum { INDIRECT = 4 };

        enum Direction { N, E, S, W };
};

}

namespace shape {
struct Triangle {};
struct Square {};
struct Hex {};
struct Rhombus {};
struct Trapezium {};
struct Boardless {};
};


template <typename T, typename TILING, typename SHAPE, size_t S1, size_t S2=0>
struct Board {
};

// Tiling       Shape           Area
// ------       -----           -----
// Square       Square          S1*S2
// Square       Hex             7*S1*S1 - 10*S1 + 4
// Square       Triangle        triangular<S1>

// Hex          Square          S1*S2
// Hex          Hex             centered_hexagonal<S1>
// Hex          Triangle        triangular<S1>

// Triangle     Square          S1*S2
// Triangle     Hex             6 * S1*S1
// Triangle     Triangle        S1*S1


