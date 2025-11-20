#ifndef DES_H
#define DES_H

#include <stddef.h>
#include <stdint.h>

/**
 * Cifra datos usando DES (Data Encryption Standard)
 * Implementa key schedule completo y 16 rondas Feistel
 * Usa modo ECB con padding PKCS#7
 * 
 * @param in     Buffer de entrada con datos a cifrar
 * @param n      Tamaño del buffer de entrada
 * @param key    Clave de 64 bits (8 bytes), se ignoran bits de paridad
 * @param klen   Longitud de la clave (debe ser >= 8)
 * @param out    Puntero donde se almacenará el buffer cifrado (debe liberarse con free)
 * @param outn   Puntero donde se almacenará el tamaño del buffer cifrado
 * @return       0 en éxito, -1 en error
 */
int des_encrypt(const uint8_t *in, size_t n,
                const uint8_t *key, size_t klen,
                uint8_t **out, size_t *outn);

/**
 * Descifra datos cifrados con DES
 * 
 * @param in     Buffer de entrada con datos cifrados
 * @param n      Tamaño del buffer de entrada (debe ser múltiplo de 8)
 * @param key    Clave de 64 bits (8 bytes)
 * @param klen   Longitud de la clave (debe ser >= 8)
 * @param out    Puntero donde se almacenará el buffer descifrado (debe liberarse con free)
 * @param outn   Puntero donde se almacenará el tamaño del buffer descifrado
 * @return       0 en éxito, -1 en error
 */
int des_decrypt(const uint8_t *in, size_t n,
                const uint8_t *key, size_t klen,
                uint8_t **out, size_t *outn);

#endif
