
#include "des.h"
#include <stdlib.h>
#include <string.h>


// Permutación inicial (IP)
static const int IP[64] = {
    58, 50, 42, 34, 26, 18, 10, 2,
    60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6,
    64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9,  1,
    59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5,
    63, 55, 47, 39, 31, 23, 15, 7
};

// Permutación final (FP) - inversa de IP
static const int FP[64] = {
    40, 8, 48, 16, 56, 24, 64, 32,
    39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30,
    37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28,
    35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26,
    33, 1, 41, 9,  49, 17, 57, 25
};

// Permuted Choice 1 (PC-1) - extrae 56 bits de la clave de 64 bits
static const int PC1[56] = {
    57, 49, 41, 33, 25, 17, 9,
    1,  58, 50, 42, 34, 26, 18,
    10, 2,  59, 51, 43, 35, 27,
    19, 11, 3,  60, 52, 44, 36,
    63, 55, 47, 39, 31, 23, 15,
    7,  62, 54, 46, 38, 30, 22,
    14, 6,  61, 53, 45, 37, 29,
    21, 13, 5,  28, 20, 12, 4
};

// Permuted Choice 2 (PC-2) - selecciona 48 bits de los 56
static const int PC2[48] = {
    14, 17, 11, 24, 1,  5,
    3,  28, 15, 6,  21, 10,
    23, 19, 12, 4,  26, 8,
    16, 7,  27, 20, 13, 2,
    41, 52, 31, 37, 47, 55,
    30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53,
    46, 42, 50, 36, 29, 32
};

// Número de rotaciones por ronda
static const int SHIFTS[16] = {
    1, 1, 2, 2, 2, 2, 2, 2,
    1, 2, 2, 2, 2, 2, 2, 1
};

// Expansión E (32 bits -> 48 bits)
static const int E[48] = {
    32, 1,  2,  3,  4,  5,
    4,  5,  6,  7,  8,  9,
    8,  9,  10, 11, 12, 13,
    12, 13, 14, 15, 16, 17,
    16, 17, 18, 19, 20, 21,
    20, 21, 22, 23, 24, 25,
    24, 25, 26, 27, 28, 29,
    28, 29, 30, 31, 32, 1
};

// Permutación P
static const int P[32] = {
    16, 7,  20, 21,
    29, 12, 28, 17,
    1,  15, 23, 26,
    5,  18, 31, 10,
    2,  8,  24, 14,
    32, 27, 3,  9,
    19, 13, 30, 6,
    22, 11, 4,  25
};

// S-boxes (8 cajas de sustitución)
static const int S[8][4][16] = {
    // S1
    {
        {14, 4,  13, 1,  2,  15, 11, 8,  3,  10, 6,  12, 5,  9,  0,  7},
        {0,  15, 7,  4,  14, 2,  13, 1,  10, 6,  12, 11, 9,  5,  3,  8},
        {4,  1,  14, 8,  13, 6,  2,  11, 15, 12, 9,  7,  3,  10, 5,  0},
        {15, 12, 8,  2,  4,  9,  1,  7,  5,  11, 3,  14, 10, 0,  6,  13}
    },
    // S2
    {
        {15, 1,  8,  14, 6,  11, 3,  4,  9,  7,  2,  13, 12, 0,  5,  10},
        {3,  13, 4,  7,  15, 2,  8,  14, 12, 0,  1,  10, 6,  9,  11, 5},
        {0,  14, 7,  11, 10, 4,  13, 1,  5,  8,  12, 6,  9,  3,  2,  15},
        {13, 8,  10, 1,  3,  15, 4,  2,  11, 6,  7,  12, 0,  5,  14, 9}
    },
    // S3
    {
        {10, 0,  9,  14, 6,  3,  15, 5,  1,  13, 12, 7,  11, 4,  2,  8},
        {13, 7,  0,  9,  3,  4,  6,  10, 2,  8,  5,  14, 12, 11, 15, 1},
        {13, 6,  4,  9,  8,  15, 3,  0,  11, 1,  2,  12, 5,  10, 14, 7},
        {1,  10, 13, 0,  6,  9,  8,  7,  4,  15, 14, 3,  11, 5,  2,  12}
    },
    // S4
    {
        {7,  13, 14, 3,  0,  6,  9,  10, 1,  2,  8,  5,  11, 12, 4,  15},
        {13, 8,  11, 5,  6,  15, 0,  3,  4,  7,  2,  12, 1,  10, 14, 9},
        {10, 6,  9,  0,  12, 11, 7,  13, 15, 1,  3,  14, 5,  2,  8,  4},
        {3,  15, 0,  6,  10, 1,  13, 8,  9,  4,  5,  11, 12, 7,  2,  14}
    },
    // S5
    {
        {2,  12, 4,  1,  7,  10, 11, 6,  8,  5,  3,  15, 13, 0,  14, 9},
        {14, 11, 2,  12, 4,  7,  13, 1,  5,  0,  15, 10, 3,  9,  8,  6},
        {4,  2,  1,  11, 10, 13, 7,  8,  15, 9,  12, 5,  6,  3,  0,  14},
        {11, 8,  12, 7,  1,  14, 2,  13, 6,  15, 0,  9,  10, 4,  5,  3}
    },
    // S6
    {
        {12, 1,  10, 15, 9,  2,  6,  8,  0,  13, 3,  4,  14, 7,  5,  11},
        {10, 15, 4,  2,  7,  12, 9,  5,  6,  1,  13, 14, 0,  11, 3,  8},
        {9,  14, 15, 5,  2,  8,  12, 3,  7,  0,  4,  10, 1,  13, 11, 6},
        {4,  3,  2,  12, 9,  5,  15, 10, 11, 14, 1,  7,  6,  0,  8,  13}
    },
    // S7
    {
        {4,  11, 2,  14, 15, 0,  8,  13, 3,  12, 9,  7,  5,  10, 6,  1},
        {13, 0,  11, 7,  4,  9,  1,  10, 14, 3,  5,  12, 2,  15, 8,  6},
        {1,  4,  11, 13, 12, 3,  7,  14, 10, 15, 6,  8,  0,  5,  9,  2},
        {6,  11, 13, 8,  1,  4,  10, 7,  9,  5,  0,  15, 14, 2,  3,  12}
    },
    // S8
    {
        {13, 2,  8,  4,  6,  15, 11, 1,  10, 9,  3,  14, 5,  0,  12, 7},
        {1,  15, 13, 8,  10, 3,  7,  4,  12, 5,  6,  11, 0,  14, 9,  2},
        {7,  11, 4,  1,  9,  12, 14, 2,  0,  6,  10, 13, 15, 3,  5,  8},
        {2,  1,  14, 7,  4,  10, 8,  13, 15, 12, 9,  0,  3,  5,  6,  11}
    }
};

// Obtener bit en posición pos (1-indexed como en las tablas DES)
static inline int get_bit(const uint8_t *data, int pos) {
    int byte_idx = (pos - 1) / 8;
    int bit_idx = 7 - ((pos - 1) % 8);
    return (data[byte_idx] >> bit_idx) & 1;
}

// Establecer bit en posición pos
static inline void set_bit(uint8_t *data, int pos, int value) {
    int byte_idx = (pos - 1) / 8;
    int bit_idx = 7 - ((pos - 1) % 8);
    if (value) {
        data[byte_idx] |= (1 << bit_idx);
    } else {
        data[byte_idx] &= ~(1 << bit_idx);
    }
}

// Aplicar una permutación
static void permute(const uint8_t *in, uint8_t *out, const int *table, int n) {
    memset(out, 0, (n + 7) / 8);
    for (int i = 0; i < n; i++) {
        int bit = get_bit(in, table[i]);
        set_bit(out, i + 1, bit);
    }
}

// Rotar a la izquierda 28 bits
static void rotate_left_28(uint8_t *data, int shifts) {
    for (int s = 0; s < shifts; s++) {
        int first_bit = get_bit(data, 1);
        for (int i = 1; i < 28; i++) {
            int bit = get_bit(data, i + 1);
            set_bit(data, i, bit);
        }
        set_bit(data, 28, first_bit);
    }
}

static void generate_subkeys(const uint8_t *key, uint8_t subkeys[16][6]) {
    uint8_t permuted_key[7];  // 56 bits
    uint8_t C[4], D[4];       // 28 bits cada uno
    
    // Aplicar PC-1
    permute(key, permuted_key, PC1, 56);
    
    // Dividir en C0 y D0
    memset(C, 0, 4);
    memset(D, 0, 4);
    for (int i = 0; i < 28; i++) {
        int bit = get_bit(permuted_key, i + 1);
        set_bit(C, i + 1, bit);
    }
    for (int i = 0; i < 28; i++) {
        int bit = get_bit(permuted_key, 28 + i + 1);
        set_bit(D, i + 1, bit);
    }
    
    // Generar 16 subclaves
    for (int round = 0; round < 16; round++) {
        // Rotar C y D
        rotate_left_28(C, SHIFTS[round]);
        rotate_left_28(D, SHIFTS[round]);
        
        // Combinar C y D
        uint8_t CD[7];
        memset(CD, 0, 7);
        for (int i = 0; i < 28; i++) {
            set_bit(CD, i + 1, get_bit(C, i + 1));
        }
        for (int i = 0; i < 28; i++) {
            set_bit(CD, 28 + i + 1, get_bit(D, i + 1));
        }
        
        // Aplicar PC-2 para obtener subclave de 48 bits
        permute(CD, subkeys[round], PC2, 48);
    }
}

static void feistel_f(const uint8_t *R, const uint8_t *subkey, uint8_t *output) {
    uint8_t expanded[6];  // 48 bits
    
    // Expansión E: 32 bits -> 48 bits
    permute(R, expanded, E, 48);
    
    // XOR con subclave
    for (int i = 0; i < 6; i++) {
        expanded[i] ^= subkey[i];
    }
    
    // Aplicar S-boxes: 48 bits -> 32 bits
    uint8_t sbox_out[4];
    memset(sbox_out, 0, 4);
    
    for (int box = 0; box < 8; box++) {
        // Extraer 6 bits para esta S-box
        int start_bit = box * 6 + 1;
        int row = (get_bit(expanded, start_bit) << 1) | get_bit(expanded, start_bit + 5);
        int col = (get_bit(expanded, start_bit + 1) << 3) |
                  (get_bit(expanded, start_bit + 2) << 2) |
                  (get_bit(expanded, start_bit + 3) << 1) |
                  get_bit(expanded, start_bit + 4);
        
        int sval = S[box][row][col];
        
        // Escribir 4 bits de salida
        for (int b = 0; b < 4; b++) {
            int bit = (sval >> (3 - b)) & 1;
            set_bit(sbox_out, box * 4 + b + 1, bit);
        }
    }
    
    // Aplicar permutación P
    permute(sbox_out, output, P, 32);
}


static void des_block(const uint8_t *input, const uint8_t subkeys[16][6], 
                      uint8_t *output, int decrypt) {
    uint8_t block[8];
    uint8_t L[4], R[4], temp[4];
    
    // Permutación inicial
    permute(input, block, IP, 64);
    
    // Dividir en L0 y R0
    memcpy(L, block, 4);
    memcpy(R, block + 4, 4);
    
    // 16 rondas Feistel
    for (int round = 0; round < 16; round++) {
        int key_idx = decrypt ? (15 - round) : round;
        
        memcpy(temp, R, 4);
        
        // R_new = L XOR f(R, K)
        uint8_t f_output[4];
        feistel_f(R, subkeys[key_idx], f_output);
        
        for (int i = 0; i < 4; i++) {
            R[i] = L[i] ^ f_output[i];
        }
        
        memcpy(L, temp, 4);
    }
    
    // Intercambiar L y R (swap de 32 bits)
    uint8_t final_block[8];
    memcpy(final_block, R, 4);
    memcpy(final_block + 4, L, 4);
    
    // Permutación final
    permute(final_block, output, FP, 64);
}

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

int des_encrypt(const uint8_t *in, size_t n,
                const uint8_t *key, size_t klen,
                uint8_t **out, size_t *outn) {
    if (!in || !key || !out || !outn) return -1;
    if (klen < 8) return -1;
    
    // Generar subclaves
    uint8_t subkeys[16][6];
    generate_subkeys(key, subkeys);
    
    // Calcular padding PKCS#7
    size_t pad_len = 8 - (n % 8);
    size_t total_len = n + pad_len;
    
    uint8_t *outbuf = malloc(total_len);
    if (!outbuf) return -1;
    
    // Procesar bloques completos
    size_t blocks = n / 8;
    for (size_t i = 0; i < blocks; i++) {
        des_block(in + i * 8, subkeys, outbuf + i * 8, 0);
    }
    
    // Último bloque con padding
    uint8_t last_block[8];
    size_t remaining = n % 8;
    if (remaining > 0) {
        memcpy(last_block, in + blocks * 8, remaining);
    }
    for (size_t i = remaining; i < 8; i++) {
        last_block[i] = (uint8_t)pad_len;
    }
    des_block(last_block, subkeys, outbuf + blocks * 8, 0);
    
    *out = outbuf;
    *outn = total_len;
    return 0;
}

int des_decrypt(const uint8_t *in, size_t n,
                const uint8_t *key, size_t klen,
                uint8_t **out, size_t *outn) {
    if (!in || !key || !out || !outn) return -1;
    if (klen < 8 || n == 0 || n % 8 != 0) return -1;
    
    // Generar subclaves
    uint8_t subkeys[16][6];
    generate_subkeys(key, subkeys);
    
    uint8_t *outbuf = malloc(n);
    if (!outbuf) return -1;
    
    // Descifrar todos los bloques
    size_t blocks = n / 8;
    for (size_t i = 0; i < blocks; i++) {
        des_block(in + i * 8, subkeys, outbuf + i * 8, 1);
    }
    
    // Remover padding PKCS#7
    uint8_t pad_len = outbuf[n - 1];
    if (pad_len < 1 || pad_len > 8) {
        free(outbuf);
        return -1;
    }
    
    // Verificar padding
    for (size_t i = 0; i < pad_len; i++) {
        if (outbuf[n - 1 - i] != pad_len) {
            free(outbuf);
            return -1;
        }
    }
    
    *out = outbuf;
    *outn = n - pad_len;
    return 0;
}
