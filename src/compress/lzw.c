/*
 * Implementación del algoritmo LZW (Lempel-Ziv-Welch)
 * Algoritmo de compresión sin pérdida basado en diccionario dinámico
 * 
 * Características:
 * - Diccionario de 12 bits (4096 entradas)
 * - Inicializado con 256 símbolos de un byte
 * - Códigos de salida empaquetados en bytes
 */

#include "lzw.h"
#include <stdlib.h>
#include <string.h>

#define LZW_MAX_CODE 4095       // Máximo código de 12 bits
#define LZW_INIT_CODES 256      // Códigos iniciales (0-255)
#define LZW_BITS 12             // Bits por código

// Nodo para la tabla hash del compresor
typedef struct lzw_node {
    int code;
    uint8_t byte;
    struct lzw_node *next;
    struct lzw_node *children[256];
} lzw_node_t;

// Tabla del descompresor
typedef struct {
    uint8_t *data;
    int len;
} lzw_entry_t;

// ============================================================================
// FUNCIONES AUXILIARES PARA ESCRITURA/LECTURA DE BITS
// ============================================================================

typedef struct {
    uint8_t *data;
    size_t capacity;
    size_t byte_pos;
    int bit_pos;  // posición del bit dentro del byte actual (0-7)
} bitwriter_t;

static void bw_init(bitwriter_t *bw) {
    bw->capacity = 1024;
    bw->data = malloc(bw->capacity);
    bw->byte_pos = 0;
    bw->bit_pos = 0;
}

static void bw_ensure(bitwriter_t *bw, size_t extra) {
    while (bw->byte_pos + extra >= bw->capacity) {
        bw->capacity *= 2;
        bw->data = realloc(bw->data, bw->capacity);
    }
}

static void bw_write_bits(bitwriter_t *bw, uint32_t value, int nbits) {
    bw_ensure(bw, 4);
    for (int i = nbits - 1; i >= 0; i--) {
        int bit = (value >> i) & 1;
        if (bw->bit_pos == 0) {
            bw->data[bw->byte_pos] = 0;
        }
        bw->data[bw->byte_pos] |= (bit << (7 - bw->bit_pos));
        bw->bit_pos++;
        if (bw->bit_pos == 8) {
            bw->bit_pos = 0;
            bw->byte_pos++;
        }
    }
}

static size_t bw_finalize(bitwriter_t *bw) {
    if (bw->bit_pos > 0) {
        bw->byte_pos++;
    }
    return bw->byte_pos;
}

typedef struct {
    const uint8_t *data;
    size_t size;
    size_t byte_pos;
    int bit_pos;
} bitreader_t;

static void br_init(bitreader_t *br, const uint8_t *data, size_t size) {
    br->data = data;
    br->size = size;
    br->byte_pos = 0;
    br->bit_pos = 0;
}

static int br_read_bits(bitreader_t *br, int nbits, uint32_t *out) {
    uint32_t value = 0;
    for (int i = 0; i < nbits; i++) {
        if (br->byte_pos >= br->size) {
            return -1;  // fin de datos
        }
        int bit = (br->data[br->byte_pos] >> (7 - br->bit_pos)) & 1;
        value = (value << 1) | bit;
        br->bit_pos++;
        if (br->bit_pos == 8) {
            br->bit_pos = 0;
            br->byte_pos++;
        }
    }
    *out = value;
    return 0;
}

// ============================================================================
// COMPRESIÓN LZW
// ============================================================================

static lzw_node_t* create_node(int code, uint8_t byte) {
    lzw_node_t *node = calloc(1, sizeof(lzw_node_t));
    node->code = code;
    node->byte = byte;
    return node;
}

static void free_tree(lzw_node_t *node) {
    if (!node) return;
    for (int i = 0; i < 256; i++) {
        free_tree(node->children[i]);
    }
    free(node);
}

int lzw_compress(const uint8_t *in, size_t n, uint8_t **out, size_t *outn) {
    if (!in || !out || !outn) return -1;
    if (n == 0) {
        *out = NULL;
        *outn = 0;
        return 0;
    }

    // Inicializar árbol de búsqueda (diccionario)
    lzw_node_t *root = create_node(-1, 0);
    for (int i = 0; i < 256; i++) {
        root->children[i] = create_node(i, (uint8_t)i);
    }

    bitwriter_t bw;
    bw_init(&bw);

    int next_code = LZW_INIT_CODES;
    lzw_node_t *current = root;
    
    for (size_t i = 0; i < n; i++) {
        uint8_t byte = in[i];
        
        // Buscar en el diccionario
        if (current->children[byte] != NULL) {
            current = current->children[byte];
        } else {
            // Emitir código del prefijo
            bw_write_bits(&bw, current->code, LZW_BITS);
            
            // Agregar nueva entrada si hay espacio
            if (next_code <= LZW_MAX_CODE) {
                current->children[byte] = create_node(next_code, byte);
                next_code++;
            }
            
            // Reiniciar con el byte actual
            current = root->children[byte];
        }
    }
    
    // Emitir el último código
    if (current != root) {
        bw_write_bits(&bw, current->code, LZW_BITS);
    }

    size_t final_size = bw_finalize(&bw);
    *out = bw.data;
    *outn = final_size;

    free_tree(root);
    return 0;
}

// ============================================================================
// DESCOMPRESIÓN LZW
// ============================================================================

int lzw_decompress(const uint8_t *in, size_t n, uint8_t **out, size_t *outn) {
    if (!in || !out || !outn) return -1;
    if (n == 0) {
        *out = NULL;
        *outn = 0;
        return 0;
    }

    // Inicializar tabla de descompresión
    lzw_entry_t *table = malloc((LZW_MAX_CODE + 1) * sizeof(lzw_entry_t));
    if (!table) return -1;

    for (int i = 0; i < LZW_INIT_CODES; i++) {
        table[i].data = malloc(1);
        table[i].data[0] = (uint8_t)i;
        table[i].len = 1;
    }
    
    int next_code = LZW_INIT_CODES;

    // Buffer de salida dinámico
    size_t out_cap = n * 2;  // estimación
    size_t out_len = 0;
    uint8_t *outbuf = malloc(out_cap);
    if (!outbuf) {
        free(table);
        return -1;
    }

    bitreader_t br;
    br_init(&br, in, n);

    uint32_t prev_code;
    if (br_read_bits(&br, LZW_BITS, &prev_code) != 0 || prev_code >= LZW_INIT_CODES) {
        free(outbuf);
        for (int i = 0; i < next_code; i++) free(table[i].data);
        free(table);
        return -1;
    }

    // Escribir primer código
    if (out_len + table[prev_code].len > out_cap) {
        out_cap *= 2;
        outbuf = realloc(outbuf, out_cap);
    }
    memcpy(outbuf + out_len, table[prev_code].data, table[prev_code].len);
    out_len += table[prev_code].len;

    // Procesar códigos restantes
    while (1) {
        uint32_t code;
        if (br_read_bits(&br, LZW_BITS, &code) != 0) {
            break;  // fin de datos
        }

        uint8_t *entry_data;
        int entry_len;

        if (code < (uint32_t)next_code) {
            // Código existente
            entry_data = table[code].data;
            entry_len = table[code].len;
        } else if (code == (uint32_t)next_code) {
            // Caso especial: código no existe aún
            entry_len = table[prev_code].len + 1;
            entry_data = malloc(entry_len);
            memcpy(entry_data, table[prev_code].data, table[prev_code].len);
            entry_data[entry_len - 1] = table[prev_code].data[0];
        } else {
            // Código inválido
            free(outbuf);
            for (int i = 0; i < next_code; i++) free(table[i].data);
            free(table);
            return -1;
        }

        // Escribir salida
        if (out_len + entry_len > out_cap) {
            while (out_len + entry_len > out_cap) out_cap *= 2;
            outbuf = realloc(outbuf, out_cap);
        }
        memcpy(outbuf + out_len, entry_data, entry_len);
        out_len += entry_len;

        // Agregar nueva entrada a la tabla
        if (next_code <= LZW_MAX_CODE) {
            table[next_code].len = table[prev_code].len + 1;
            table[next_code].data = malloc(table[next_code].len);
            memcpy(table[next_code].data, table[prev_code].data, table[prev_code].len);
            table[next_code].data[table[next_code].len - 1] = entry_data[0];
            next_code++;
        }

        if (code == (uint32_t)(next_code - 1) && code != prev_code) {
            // Liberamos el entry_data temporal del caso especial
            // (ya fue copiado a la tabla)
        }

        prev_code = code;
    }

    *out = outbuf;
    *outn = out_len;

    // Liberar tabla
    for (int i = 0; i < next_code; i++) {
        free(table[i].data);
    }
    free(table);

    return 0;
}
