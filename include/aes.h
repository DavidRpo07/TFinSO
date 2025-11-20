#ifndef AES_H
#define AES_H

#include <stddef.h>
#include <stdint.h>

/**
 * Cifra datos usando AES simplificado (S-box, ShiftRows, MixColumns)
 * Implementa una versión simplificada de AES con las operaciones básicas
 * Usa bloques de 16 bytes y modo ECB con padding PKCS#7
 * 
 * @param in     Buffer de entrada con datos a cifrar
 * @param n      Tamaño del buffer de entrada
 * @param key    Clave de cifrado (mínimo 16 bytes)
 * @param klen   Longitud de la clave (debe ser >= 16)
 * @param out    Puntero donde se almacenará el buffer cifrado (debe liberarse con free)
 * @param outn   Puntero donde se almacenará el tamaño del buffer cifrado
 * @return       0 en éxito, -1 en error
 */
int aes_encrypt(const uint8_t *in, size_t n,
                const uint8_t *key, size_t klen,
                uint8_t **out, size_t *outn);

/**
 * Descifra datos cifrados con AES simplificado
 * 
 * @param in     Buffer de entrada con datos cifrados
 * @param n      Tamaño del buffer de entrada (debe ser múltiplo de 16)
 * @param key    Clave de descifrado (mínimo 16 bytes)
 * @param klen   Longitud de la clave (debe ser >= 16)
 * @param out    Puntero donde se almacenará el buffer descifrado (debe liberarse con free)
 * @param outn   Puntero donde se almacenará el tamaño del buffer descifrado
 * @return       0 en éxito, -1 en error
 */
int aes_decrypt(const uint8_t *in, size_t n,
                const uint8_t *key, size_t klen,
                uint8_t **out, size_t *outn);

#endif
