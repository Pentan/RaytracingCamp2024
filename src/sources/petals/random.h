#ifndef PETALS_RANDOM_H
#define PETALS_RANDOM_H

#include <stdint.h>

namespace Petals {
    
    class Random {
        /*  Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)
         
         To the extent possible under law, the author has dedicated all copyright
         and related and neighboring rights to this software to the public domain
         worldwide. This software is distributed without any warranty.
         
         See <http://creativecommons.org/publicdomain/zero/1.0/>. */
        
        /* This is xoshiro256+ 1.0, our best and fastest generator for floating-point
         numbers. We suggest to use its upper bits for floating-point
         generation, as it is slightly faster than xoshiro256++/xoshiro256**. It
         passes all tests we are aware of except for the lowest three bits,
         which might fail linearity tests (and just those), so if low linear
         complexity is not considered an issue (as it is usually the case) it
         can be used to generate 64-bit outputs, too.
         
         We suggest to use a sign test to extract a random Boolean value, and
         right shifts to extract subsets of bits.
         
         The state must be seeded so that it is not everywhere zero. If you have
         a 64-bit seed, we suggest to seed a splitmix64 generator and use its
         output to fill s. */
        
    private:
        inline uint64_t rotl(const uint64_t x, int k) {
            return (x << k) | (x >> (64 - k));
        }
        
        uint64_t s[4];
        
        uint64_t next(void) {
            const uint64_t result = s[0] + s[3];
            
            const uint64_t t = s[1] << 17;
            
            s[2] ^= s[0];
            s[3] ^= s[1];
            s[1] ^= s[2];
            s[0] ^= s[3];
            
            s[2] ^= t;
            
            s[3] = rotl(s[3], 45);
            
            return result;
        }
        
        
        /* This is the jump function for the generator. It is equivalent
         to 2^128 calls to next(); it can be used to generate 2^128
         non-overlapping subsequences for parallel computations. */
        
        void jump(void) {
            static const uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };
            
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for(int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
                for(int b = 0; b < 64; b++) {
                    if (JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                        s2 ^= s[2];
                        s3 ^= s[3];
                    }
                    next();
                }
            
            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }
        
        
        /* This is the long-jump function for the generator. It is equivalent to
         2^192 calls to next(); it can be used to generate 2^64 starting points,
         from each of which jump() will generate 2^64 non-overlapping
         subsequences for parallel distributed computations. */
        
        void long_jump(void) {
            static const uint64_t LONG_JUMP[] = { 0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };
            
            uint64_t s0 = 0;
            uint64_t s1 = 0;
            uint64_t s2 = 0;
            uint64_t s3 = 0;
            for(int i = 0; i < sizeof LONG_JUMP / sizeof *LONG_JUMP; i++)
                for(int b = 0; b < 64; b++) {
                    if (LONG_JUMP[i] & UINT64_C(1) << b) {
                        s0 ^= s[0];
                        s1 ^= s[1];
                        s2 ^= s[2];
                        s3 ^= s[3];
                    }
                    next();
                }
            
            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }
        
    public:
        Random() {
            setSeed(123456789);
        }
        
        Random(uint64_t seed) {
            setSeed(seed);
        }
        
        void setSeed(uint64_t seed) {
            /* splitmix64 http://xoshiro.di.unimi.it/splitmix64.c */
            /* The state can be seeded with any value. */
            uint64_t x = seed;
            
            // Init seeds
            for(int i = 0; i < 4; i++) {
                x += 0x9e3779b97f4a7c15;
                uint64_t z = x;
                z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
                z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
                s[i] = z ^ (z >> 31);
            }
        }
        
        int nextInt32() {
            uint64_t n = next();
            return (int)(n >> 32);
        }
        
        double nextDoubleCC() {
            return double(next() >> 11) / 9007199254740991.0;
        }
        
        double nextDoubleCO() {
            return double(next() >> 11) / 9007199254740992.0;
        }
        
        double nextDoubleOO() {
            return double(next() >> 12) / 4503599627370496.0;
        }
        
        
    };
}

#endif
