#ifndef LZW_H
#define LZW_H

#include <stddef.h>
#include <stdint.h>

/**
 * Comprime datos usando el algoritmo LZW (Lempel-Ziv-Welch)
 * 
 * @param in     Buffer de entrada con datos a comprimir
 * @param n      Tamaño del buffer de entrada
 * @param out    Puntero donde se almacenará el buffer de salida (debe liberarse con free)
 * @param outn   Puntero donde se almacenará el tamaño del buffer de salida
 * @return       0 en éxito, -1 en error
 */
int lzw_compress(const uint8_t *in, size_t n, uint8_t **out, size_t *outn);

/**
 * Descomprime datos comprimidos con LZW
 * 
 * @param in     Buffer de entrada con datos comprimidos
 * @param n      Tamaño del buffer de entrada
 * @param out    Puntero donde se almacenará el buffer descomprimido (debe liberarse con free)
 * @param outn   Puntero donde se almacenará el tamaño del buffer descomprimido
 * @return       0 en éxito, -1 en error
 */
int lzw_decompress(const uint8_t *in, size_t n, uint8_t **out, size_t *outn);

#endif
