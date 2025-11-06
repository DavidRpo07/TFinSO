#include "vigenere.h"
#include <stdlib.h>

int vig_encrypt(const uint8_t *in, size_t n,
                const uint8_t *key, size_t klen,
                uint8_t **out, size_t *outn)
{
    if (!in || !key || klen == 0){
        return -1;
    }

    uint8_t *buf = malloc(n);
    if (!buf) return -1;

    for (size_t i = 0; i < n; i++){
        // suma byte a byte, modulando a 256
        buf[i] = (uint8_t)((in[i] + key[i % klen]) & 0xFF);
    }

    *out = buf;
    *outn = n;
    return 0;
}

int vig_decrypt(const uint8_t *in, size_t n,
                const uint8_t *key, size_t klen,
                uint8_t **out, size_t *outn)
{
    if (!in || !key || klen == 0){
        return -1;
    }

    uint8_t *buf = malloc(n);
    if (!buf) return -1;

    for (size_t i = 0; i < n; i++){
        // resta byte a byte, modulando a 256
        buf[i] = (uint8_t)((in[i] - key[i % klen]) & 0xFF);
    }

    *out = buf;
    *outn = n;
    return 0;
}
