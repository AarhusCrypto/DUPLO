#ifndef DUPLO_UTIL_GLOBAL_CONSTANTS_H_
#define DUPLO_UTIL_GLOBAL_CONSTANTS_H_

//Behavior
// #define DUPLO_PRINT

static uint8_t duplo_constant_seeds[2][16] = {
  {0x43, 0x73, 0x98, 0x41, 0x70, 0x12, 0x38, 0x78, 0xAB, 0x45, 0x78, 0xFF, 0xEA, 0xD3, 0xFF, 0x00},
  {0x43, 0x73, 0x98, 0x41, 0x70, 0x12, 0x38, 0x78, 0x43, 0x73, 0x98, 0x41, 0x66, 0x19, 0xAA, 0xFE}
};

#define CSEC 128
#define CSEC_BYTES 16
#define HASH 160
#define HASH_BYTES 20
#define SSEC 40
#define SSEC_BYTES 5

static uint8_t global_aes_key[CSEC_BYTES] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};

#define NUM_COMMIT_SEED_OT CODEWORD_BITS

//ThreadPool
#define TP_MUL_FACTOR 1

#define single_bucket_param_table_size 22
static uint32_t single_bucket_param_table[single_bucket_param_table_size][4] = {
  {1, 41, 1, 0},
  {2, 21, 1, 0},
  {3, 16, 1, 0},
  {4, 14, 1, 0},
  {5, 12, 1, 0},
  {7, 11, 1, 0},
  {8, 10, 1, 0},
  {12, 9, 1, 0},
  {19, 8, 1, 0},
  {34, 7, 1, 0},
  {84, 6, 1, 0},
  {307, 6, 2, 0},
  {333, 5, 1, 0},
  {1261, 5, 2, 0},
  {3151, 4, 1, 0},
  {12583, 4, 2, 0},
  {37500, 4, 3, 0},
  {102507, 4, 4, 0},
  {275000, 4, 5, 0},
  {280132, 3, 1, 0},
  {1305011, 3, 2, 0},
  {4480146, 3, 3, 0}
};

#define majority_bucket_param_table_size 28
static uint32_t majority_bucket_param_table[majority_bucket_param_table_size][4] = {
  {2, 87, 1, 0},
  {3, 53, 1, 0},
  {4, 43, 1, 0},
  {5, 37, 1, 0},
  {6, 33, 1, 0},
  {7, 31, 1, 0},
  {8, 29, 1, 0},
  {9, 27, 1, 0},
  {10, 25, 1, 0},
  {13, 23, 1, 0},
  {16, 21, 1, 0},
  {22, 19, 1, 0},
  {31, 17, 1, 0},
  {50, 15, 1, 0},
  {95, 13, 1, 0},
  {255, 11, 1, 0},
  {671, 11, 2, 0},
  {905, 9, 1, 0},
  {2773, 9, 2, 0},
  {7057, 9, 3, 0},
  {8443, 7, 1, 0},
  {27502, 7, 2, 0},
  {76251, 7, 3, 0},
  {200116, 7, 4, 0},
  {515179, 7, 5, 0},
  {755227, 5, 1, 0},
  {2820064, 5, 2, 0},
  {8915033, 5, 3, 0}
};

#define majority_auth_param_table_size 33
static uint32_t majority_auth_param_table[majority_auth_param_table_size][4] = {
  {5, 85, 1, 0},
  {6, 67, 1, 0},
  {7, 57, 1, 0},
  {8, 51, 1, 0},
  {9, 47, 1, 0},
  {10, 43, 1, 0},
  {11, 41, 1, 0},
  {12, 37, 1, 0},
  {14, 35, 1, 0},
  {15, 33, 1, 0},
  {17, 31, 1, 0},
  {19, 29, 1, 0},
  {22, 27, 1, 0},
  {26, 25, 1, 0},
  {32, 23, 1, 0},
  {42, 21, 1, 0},
  {57, 19, 1, 0},
  {83, 17, 1, 0},
  {136, 15, 1, 0},
  {281, 13, 1, 0},
  {645, 13, 2, 0},
  {671, 11, 1, 0},
  {1765, 11, 2, 0},
  {2773, 9, 1, 0},
  {7057, 9, 2, 0},
  {17587, 9, 3, 0},
  {27502, 7, 1, 0},
  {76251, 7, 2, 0},
  {200116, 7, 3, 0},
  {515179, 7, 4, 0},
  {1310012, 7, 5, 0},
  {2820064, 5, 1, 0},
  {8915033, 5, 2, 0}
};



#endif /* DUPLO_UTIL_GLOBAL_CONSTANTS_H_ */
