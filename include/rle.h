#include <stddef.h>

int rle_compress(const unsigned char *in, size_t n,
                 unsigned char **out, size_t *outn);
int rle_decompress(const unsigned char *in, size_t n,
                   unsigned char **out, size_t *outn);
