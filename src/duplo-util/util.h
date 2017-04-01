#ifndef DUPLO_UTIL_UTIL_H_
#define DUPLO_UTIL_UTIL_H_

#include "duplo-util/typedefs.h"

#include "SplitCommit/src/util/util.h" //must be after above typedefs
#include "cryptoTools/Crypto/PRNG.h" //must be after above typedefs

#define DOUBLE(x) _mm_slli_epi64(x,1)

static inline void SetLSB(uint8_t array[], uint8_t bit) {
  SetBit(127, bit, array);
}

static inline void XORLSB(uint8_t array[], uint8_t bit) {
  XORBit(127, bit, array);
}

//Constructs work_size / buffer_size iterations where the last iteration will contain more workload than the rest
static inline void PartitionBufferFixedNum(std::vector<int>& from, std::vector<int>& to, int num_cpus, int work_size) {
  int work_pr_thread = work_size / num_cpus;
  int remaining_work = work_size % num_cpus;

  int offset = 0;
  for (int i = 0; i < num_cpus; ++i) {
    int extra_to = 0;
    if (remaining_work > 0) {
      extra_to++;
    }
    from.emplace_back(i * work_pr_thread + offset);
    to.emplace_back(i * work_pr_thread + offset + work_pr_thread + extra_to);

    if (remaining_work > 0) {
      offset++;
      remaining_work--;
    }
  }
}

//Constructs work_size / buffer_size + 1 iterations where the last iteration will not contain full workload
static inline void PartitionBufferDynNum(std::vector<int>& from, std::vector<int>& to, int buffer_size, int work_size) {
  int num_iterations = work_size / buffer_size;
  int work_last_iteration = work_size % buffer_size;

  for (int i = 0; i < num_iterations; ++i) {
    from.emplace_back(i * buffer_size);
    to.emplace_back(i * buffer_size + buffer_size);
    
  }
  if (work_last_iteration > 0) {
    from.emplace_back(num_iterations * buffer_size);
    to.emplace_back(num_iterations * buffer_size + work_last_iteration);
  }
}

static inline size_t bits_in_byte( uint8_t val) {
  static int const half_byte[] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

  int result1 = half_byte[val & 0x0f];
  int result2 = half_byte[(val >> 4) & 0x0f];

  return result1 + result2;
}

static inline int countSetBits( void* ptr, int start, int end) {
  uint8_t*    first;
  uint8_t*    last;
  int         bits_first;
  int         bits_last;
  uint8_t     mask_first;
  uint8_t     mask_last;

  size_t count = 0;

  // get bits from the first byte
  first = ((uint8_t*) ptr) + (start / 8);
  bits_first = 8 - start % 8;
  mask_first = (1 << bits_first) - 1;
  mask_first = mask_first << (8 - bits_first);


  // get bits from last byte
  last = ((uint8_t*) ptr) + (end / 8);
  bits_last = 1 + (end % 8);
  mask_last = (1 << bits_last) - 1;

  if (first == last) {
    // we only have a range of bits in  the first byte
    count = bits_in_byte( (*first) & mask_first & mask_last);
  }
  else {
    // handle the bits from the first and last bytes specially
    count += bits_in_byte((*first) & mask_first);
    count += bits_in_byte((*last) & mask_last);

    // now we've collected the odds and ends from the start and end of the bit range
    // handle the full bytes in the interior of the range

    for (first = first + 1; first != last; ++first) {
      count += bits_in_byte(*first);
    }
  }

  return count;
}

static inline void PermuteArray(uint32_t array[], int size, uint8_t seed[]) {
  uint32_t tmpidx;
  std::unique_ptr<uint64_t[]> randomness(new uint64_t[size]);
  osuCrypto::PRNG rnd;
  rnd.SetSeed(load_block(seed));
  rnd.get<uint8_t>((uint8_t*) randomness.get(), size * sizeof(uint64_t));
  for (int i = 0; i < size; ++i) {
    tmpidx = randomness[i] % (size - i);
    std::swap(array[i], array[i + tmpidx]);
  }
}

#endif /* DUPLO_UTIL_UTIL_H_ */