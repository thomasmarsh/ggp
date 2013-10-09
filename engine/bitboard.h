#ifndef BITBOARD_H
#define BITBOARD_H

#include "common.h"

//-----------------------------------------------------------------------------
//
// Utility
//
//-----------------------------------------------------------------------------

template <unsigned long long I>
struct Log2 {
        static const int result = 1 + Log2<I/2>::result;
};

template <> struct Log2<1> { static const int result = 0; };


// least significant bit index

#if 1 // raw assembly version
static inline uint64_t log2(const uint64_t x) {
        uint64_t y;
        asm ( "\tbsrq %1, %0\n"
            : "=r"(y)
            : "r" (x)
            );
        return y;
}

#else // gcc builtin
#define log2(n) __builtin_ctzl(n)
#endif


// population count

#if 0 // slow K&R loop
static inline int popcount(uint64_t x) {
        int c = 0;
        while (x) {
                ++c;
                x &= x-1; // reset LS1B
        }
        return c;
}
#else // gcc builtin
#define popcount(n) __builtin_popcountl(n)
#endif


//-----------------------------------------------------------------------------
//
// Bitfield
//
//-----------------------------------------------------------------------------

template <typename T>
struct bitfield {
        T data;

        bitfield() : data(0) {}
        bitfield(T n) : data(n) {}
        bitfield(bitfield &other) : data(other.data) {}

        T ones() const { return ~0; }
        void invert() { data ^= ones(); }

        void iset(int i)   { data |=   1ULL << (uint64_t) i;  }
        void iclear(int i) { data &= ~(1ULL << (uint64_t) i); }
        int iis_set(int i) const { return (int) (bool) ((data >> i) & 1ULL); }

        void randomize() {
                data = 0;
                int bits = std::numeric_limits<T>::digits;
                int rand_bits = Log2<RAND_MAX>::result;
                while (bits > 0) {
                        int r = random();
                        while (r >= (1<<rand_bits))
                                r = random();
                        data <<= rand_bits;
                        data += r;
                        bits -= rand_bits;
                }
        }


        //-------------------------------------------------------------------------------
        //
        // representation
        //
        //-------------------------------------------------------------------------------
       
        string bin_str() const {
                T x = data;
                stringstream s;
                for (int i=(sizeof(T)<<3)-1; i >= 0; --i)
                        s << iis_set(i);
                return s.str();
        }
        
        string square_str() const {
                T x = data;
                stringstream s;
                for (int i=0; i < (sizeof(T)<<3); ++i) {
                        s << iis_set(i);
                        if (((i+1) % 8) == 0)
                                s << endl;
                }
                return s.str();
        }

        string square_str(int q) const {
                T x = data;
                stringstream s;
                for (int i=0; i < (sizeof(T)<<3); ++i) {
                        if (i == q)
                                s << 'x';
                        else s << (iis_set(i) ? '1' : '.');
                        if (((i+1) % 8) == 0)
                                s << endl;
                }
                return s.str();
        }

        string hex_str() const {
                stringstream s;
                s << "0x" << hex << setw(16) << setfill('0') << uppercase << data;
                return s.str();
        }
};


//-----------------------------------------------------------------------------
//
// bitboard
//
//-----------------------------------------------------------------------------

struct bitboard : public bitfield<uint64_t> {
        int  index(int x, int y) const { return x+(y<<3); }
        void set(int x, int y) { iset(index(x,y)); }
        void clear(int x, int y) { iclear(index(x,y)); }
        int  is_set(int x, int y) const { return iis_set(index(x,y)); }
        // congo specific...
        bool valid(int x, int y) const { return x >= 0 && y >= 0 && x < 7 && y < 7; }
        void try_set(int x, int y) { if (valid(x, y)) set(x, y); }
};

#endif // BITBOARD_H
