#include "huffman.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

// Nodo del árbol de Huffman
typedef struct HuffNode {
    uint8_t byte;
    uint32_t freq;
    struct HuffNode *left;
    struct HuffNode *right;
} HuffNode;

// Cola de prioridad simple para construir el árbol
typedef struct {
    HuffNode **nodes;
    int size;
    int capacity;
} PriorityQueue;

// Funciones auxiliares para la cola de prioridad
static PriorityQueue* pq_create(int capacity) {
    PriorityQueue *pq = malloc(sizeof(PriorityQueue));
    if (!pq) return NULL;
    pq->nodes = malloc(sizeof(HuffNode*) * capacity);
    if (!pq->nodes) {
        free(pq);
        return NULL;
    }
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

static void pq_free(PriorityQueue *pq) {
    if (pq) {
        free(pq->nodes);
        free(pq);
    }
}

static void pq_insert(PriorityQueue *pq, HuffNode *node) {
    if (pq->size >= pq->capacity) return;
    
    int i = pq->size++;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (pq->nodes[parent]->freq <= node->freq) break;
        pq->nodes[i] = pq->nodes[parent];
        i = parent;
    }
    pq->nodes[i] = node;
}

static HuffNode* pq_extract_min(PriorityQueue *pq) {
    if (pq->size == 0) return NULL;
    
    HuffNode *min = pq->nodes[0];
    HuffNode *last = pq->nodes[--pq->size];
    
    int i = 0;
    while (i * 2 + 1 < pq->size) {
        int left = i * 2 + 1;
        int right = i * 2 + 2;
        int smallest = left;
        
        if (right < pq->size && pq->nodes[right]->freq < pq->nodes[left]->freq)
            smallest = right;
        
        if (last->freq <= pq->nodes[smallest]->freq) break;
        
        pq->nodes[i] = pq->nodes[smallest];
        i = smallest;
    }
    pq->nodes[i] = last;
    
    return min;
}

// Crear nodo del árbol
static HuffNode* create_node(uint8_t byte, uint32_t freq, HuffNode *left, HuffNode *right) {
    HuffNode *node = malloc(sizeof(HuffNode));
    if (!node) return NULL;
    node->byte = byte;
    node->freq = freq;
    node->left = left;
    node->right = right;
    return node;
}

// Liberar árbol
static void free_tree(HuffNode *node) {
    if (!node) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

// Construir tabla de códigos
static void build_codes_recursive(HuffNode *node, uint32_t code, int bits, uint32_t *codes, uint8_t *code_lens) {
    if (!node) return;
    
    if (!node->left && !node->right) {
        codes[node->byte] = code;
        code_lens[node->byte] = bits;
        return;
    }
    
    if (node->left)
        build_codes_recursive(node->left, code << 1, bits + 1, codes, code_lens);
    if (node->right)
        build_codes_recursive(node->right, (code << 1) | 1, bits + 1, codes, code_lens);
}

// Escribir bits en el buffer
static void write_bits(uint8_t **out, size_t *out_pos, uint8_t *bit_pos, uint32_t code, uint8_t len) {
    for (int i = len - 1; i >= 0; i--) {
        if ((code >> i) & 1) {
            (*out)[*out_pos] |= (1 << (7 - *bit_pos));
        }
        
        (*bit_pos)++;
        if (*bit_pos == 8) {
            *bit_pos = 0;
            (*out_pos)++;
        }
    }
}

int huffman_compress(const uint8_t *in, size_t n, uint8_t **out, size_t *outn) {
    if (!in || !out || !outn) return -1;
    if (n == 0) {
        *out = malloc(1);
        *outn = 0;
        return 0;
    }
    
    // 1. Calcular frecuencias
    uint32_t freq[256] = {0};
    for (size_t i = 0; i < n; i++) {
        freq[in[i]]++;
    }
    
    // 2. Construir árbol de Huffman
    PriorityQueue *pq = pq_create(256);
    if (!pq) return -1;
    
    int unique_bytes = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            HuffNode *node = create_node(i, freq[i], NULL, NULL);
            if (!node) {
                pq_free(pq);
                return -1;
            }
            pq_insert(pq, node);
            unique_bytes++;
        }
    }
    
    // Caso especial: solo un byte único
    if (unique_bytes == 1) {
        HuffNode *single = pq_extract_min(pq);
        HuffNode *root = create_node(0, single->freq, single, NULL);
        pq_insert(pq, root);
    }
    
    // Construir árbol combinando nodos
    while (pq->size > 1) {
        HuffNode *left = pq_extract_min(pq);
        HuffNode *right = pq_extract_min(pq);
        HuffNode *parent = create_node(0, left->freq + right->freq, left, right);
        if (!parent) {
            pq_free(pq);
            return -1;
        }
        pq_insert(pq, parent);
    }
    
    HuffNode *root = pq_extract_min(pq);
    pq_free(pq);
    
    if (!root) return -1;
    
    // 3. Generar códigos
    uint32_t codes[256] = {0};
    uint8_t code_lens[256] = {0};
    build_codes_recursive(root, 0, 0, codes, code_lens);
    
    // 4. Calcular tamaño de salida
    // Header: 4 bytes (tamaño original) + 256 bytes (longitudes) + 256*4 bytes (frecuencias)
    size_t header_size = 4 + 256 + 256 * 4;
    size_t max_data_size = n * 32 / 8 + 100; // Peor caso
    *out = calloc(1, header_size + max_data_size);
    if (!*out) {
        free_tree(root);
        return -1;
    }
    
    // Escribir header
    size_t pos = 0;
    
    // Tamaño original (4 bytes)
    (*out)[pos++] = (n >> 24) & 0xFF;
    (*out)[pos++] = (n >> 16) & 0xFF;
    (*out)[pos++] = (n >> 8) & 0xFF;
    (*out)[pos++] = n & 0xFF;
    
    // Longitudes de código (256 bytes)
    memcpy(*out + pos, code_lens, 256);
    pos += 256;
    
    // Frecuencias (256 * 4 bytes)
    for (int i = 0; i < 256; i++) {
        (*out)[pos++] = (freq[i] >> 24) & 0xFF;
        (*out)[pos++] = (freq[i] >> 16) & 0xFF;
        (*out)[pos++] = (freq[i] >> 8) & 0xFF;
        (*out)[pos++] = freq[i] & 0xFF;
    }
    
    // 5. Codificar datos
    uint8_t bit_pos = 0;
    for (size_t i = 0; i < n; i++) {
        write_bits(out, &pos, &bit_pos, codes[in[i]], code_lens[in[i]]);
    }
    
    if (bit_pos != 0) pos++; // Ajustar por último byte parcial
    
    *outn = pos;
    free_tree(root);
    
    return 0;
}

int huffman_decompress(const uint8_t *in, size_t n, uint8_t **out, size_t *outn) {
    if (!in || !out || !outn) return -1;
    if (n < 4 + 256 + 256 * 4) return -1;
    
    // 1. Leer header
    size_t pos = 0;
    
    // Tamaño original
    size_t orig_size = ((size_t)in[pos] << 24) | ((size_t)in[pos+1] << 16) |
                       ((size_t)in[pos+2] << 8) | in[pos+3];
    pos += 4;
    
    if (orig_size == 0) {
        *out = malloc(1);
        *outn = 0;
        return 0;
    }
    
    // Longitudes de código
    uint8_t code_lens[256];
    memcpy(code_lens, in + pos, 256);
    pos += 256;
    
    // Frecuencias
    uint32_t freq[256];
    for (int i = 0; i < 256; i++) {
        freq[i] = ((uint32_t)in[pos] << 24) | ((uint32_t)in[pos+1] << 16) |
                  ((uint32_t)in[pos+2] << 8) | in[pos+3];
        pos += 4;
    }
    
    // 2. Reconstruir árbol de Huffman
    PriorityQueue *pq = pq_create(256);
    if (!pq) return -1;
    
    int unique_bytes = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            HuffNode *node = create_node(i, freq[i], NULL, NULL);
            if (!node) {
                pq_free(pq);
                return -1;
            }
            pq_insert(pq, node);
            unique_bytes++;
        }
    }
    
    // Caso especial: solo un byte único
    if (unique_bytes == 1) {
        HuffNode *single = pq_extract_min(pq);
        HuffNode *root = create_node(0, single->freq, single, NULL);
        pq_insert(pq, root);
    }
    
    while (pq->size > 1) {
        HuffNode *left = pq_extract_min(pq);
        HuffNode *right = pq_extract_min(pq);
        HuffNode *parent = create_node(0, left->freq + right->freq, left, right);
        if (!parent) {
            pq_free(pq);
            return -1;
        }
        pq_insert(pq, parent);
    }
    
    HuffNode *root = pq_extract_min(pq);
    pq_free(pq);
    
    if (!root) return -1;
    
    // 3. Decodificar
    *out = malloc(orig_size);
    if (!*out) {
        free_tree(root);
        return -1;
    }
    
    size_t out_pos = 0;
    HuffNode *current = root;
    
    for (size_t i = pos; i < n && out_pos < orig_size; i++) {
        for (int bit = 7; bit >= 0 && out_pos < orig_size; bit--) {
            if ((in[i] >> bit) & 1) {
                current = current->right;
            } else {
                current = current->left;
            }
            
            if (!current) {
                free(*out);
                free_tree(root);
                return -1;
            }
            
            if (!current->left && !current->right) {
                (*out)[out_pos++] = current->byte;
                current = root;
            }
        }
    }
    
    *outn = orig_size;
    free_tree(root);
    
    return 0;
}
