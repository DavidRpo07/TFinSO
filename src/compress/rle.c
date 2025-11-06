#include "rle.h"
#include <stdlib.h>
int rle_compress(const unsigned char *in, size_t n, unsigned char **out, size_t *outn){
  if (!n){ *out=NULL; *outn=0; return 0; }
  unsigned char *buf = malloc(n*2); if(!buf) return -1;
  size_t j=0;
  for (size_t i=0;i<n;){
    unsigned char v = in[i]; size_t run=1;
    while (i+run<n && in[i+run]==v && run<255) run++;
    buf[j++] = (unsigned char)run;
    buf[j++] = v;
    i += run;
  }
  *out = buf; *outn = j; return 0;
}
int rle_decompress(const unsigned char *in, size_t n, unsigned char **out, size_t *outn){
  size_t est=0;
  for (size_t k=0;k+1<n;k+=2) est += in[k];
  unsigned char *buf = malloc(est); if(!buf) return -1;
  size_t j=0;
  for (size_t i=0;i+1<n;i+=2){
    unsigned char cnt = in[i], val = in[i+1];
    for (int c=0;c<cnt;c++) buf[j++]=val;
  }
  *out=buf; *outn=j; return 0;
}
