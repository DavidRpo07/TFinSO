#ifndef VIGENERE_H
#define VIGENERE_H

#include <stddef.h>
#include <stdint.h>

int vig_encrypt(const uint8_t *in, size_t n,
                const uint8_t *key, size_t klen,
                uint8_t **out, size_t *outn);

int vig_decrypt(const uint8_t *in, size_t n,
                const uint8_t *key, size_t klen,
                uint8_t **out, size_t *outn);

#endif
