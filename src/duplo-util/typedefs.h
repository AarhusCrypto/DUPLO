#ifndef DUPLO_UTIL_TYPEDEFS_H_
#define DUPLO_UTIL_TYPEDEFS_H_


// #include <mmintrin.h>  //MMX
// #include <xmmintrin.h> //SSE
#include <emmintrin.h> //SSE2
// #include <pmmintrin.h> //SSE3
// #include <tmmintrin.h> //SSSE3
// #include <smmintrin.h> //SSE4.1
// #include <nmmintrin.h> //SSE4.2
// #include <ammintrin.h> //SSE4A
#include <wmmintrin.h> //AES
#include <immintrin.h> //AVX
// #include <zmmintrin.h> //AVX512

#include "SplitCommit/libs/CTPL/ctpl_stl.h"
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <climits>
#include <vector>
#include <array>
#include <unordered_map>
#include <map>
#include <thread>
#include <chrono>
#include <algorithm>
#include <numeric>

#include "duplo-util/filenames.h"
#include "duplo-util/global-constants.h"

//The below allows for tuples as keys for unordered hash_map
// #include <tuple>
// function has to live in the std namespace 
// so that it is picked up by argument-dependent name lookup (ADL).
namespace std {
    namespace
    {

        // Code from boost
        // Reciprocal of the golden ratio helps spread entropy
        //     and handles duplicates.
        // See Mike Seymour in magic-numbers-in-boosthash-combine:
        //     http://stackoverflow.com/questions/4948780

        template <class T>
        inline void hash_combine(std::size_t& seed, T const& v)
        {
            seed ^= hash<T>()(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        }

        // Recursive template code derived from Matthieu M.
        template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
        struct HashValueImpl
        {
          static void apply(size_t& seed, Tuple const& tuple)
          {
            HashValueImpl<Tuple, Index-1>::apply(seed, tuple);
            hash_combine(seed, get<Index>(tuple));
          }
        };

        template <class Tuple>
        struct HashValueImpl<Tuple,0>
        {
          static void apply(size_t& seed, Tuple const& tuple)
          {
            hash_combine(seed, get<0>(tuple));
          }
        };
    }

    template <typename ... TT>
    struct hash<std::tuple<TT...>> 
    {
        size_t
        operator()(std::tuple<TT...> const& tt) const
        {                                              
            size_t seed = 0;                             
            HashValueImpl<std::tuple<TT...> >::apply(seed, tt);    
            return seed;                                 
        }                                              

    };
}

typedef unsigned __int128 uint128_t;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef uint8_t octet;
typedef unsigned int uint;

#include "SplitCommit/src/util/byte-array-vec.h" //must be after above typedefs
#include "cryptoTools/Common/BitVector.h"
#include "cryptoTools/Common/Defines.h"

#endif /* DUPLO_UTIL_TYPEDEFS_H_ */