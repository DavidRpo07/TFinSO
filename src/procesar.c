#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "gsea.h"
#include "pipeline.h"              
#include "rle.h"
#include "lzw.h"
#include "huffman.h"
#include "vigenere.h"
#include "des.h"
#include "aes.h"

int gsea_process_file(const gsea_opts_t *opt){
    // 1. leer archivo completo de entradaf
    int fd = open(opt->in_path, O_RDONLY);
    if (fd < 0){
        perror("open in");
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1){
        perror("fstat");
        close(fd);
        return -1;
    }

    size_t inlen = st.st_size;
    uint8_t *inbuf = malloc(inlen ? inlen : 1);
    if (!inbuf){
        perror("malloc");
        close(fd);
        return -1;
    }

    if (inlen > 0){
        ssize_t r = read(fd, inbuf, inlen);
        if (r != (ssize_t)inlen){
            perror("read");
            free(inbuf);
            close(fd);
            return -1;
        }
    }
    close(fd);

    // buffer actual sobre el que aplicamos las operaciones
    uint8_t *cur = inbuf;
    size_t curlen = inlen;

    // 2. aplicar operaciones en el orden que indicó el usuario
    for (int i = 0; i < opt->ops_count; i++){
        char op = opt->ops_order[i];
        uint8_t *tmp = NULL;
        size_t tmplen = 0;

        if (op == 'c'){             // comprimir
            const char *alg = opt->comp_alg ? opt->comp_alg : "rle";
            if (strcmp(alg, "rle") == 0){
                if (rle_compress(cur, curlen, &tmp, &tmplen) != 0){
                    fprintf(stderr, "error: fallo RLE compress\n");
                    if (cur != inbuf) free(cur);
                    free(inbuf);
                    return -1;
                }
            } else if (strcmp(alg, "lzw") == 0){
                if (lzw_compress(cur, curlen, &tmp, &tmplen) != 0){
                    fprintf(stderr, "error: fallo LZW compress\n");
                    if (cur != inbuf) free(cur);
                    free(inbuf);
                    return -1;
                }
            } else if (strcmp(alg, "huffman") == 0){
                if (huffman_compress(cur, curlen, &tmp, &tmplen) != 0){
                    fprintf(stderr, "error: fallo Huffman compress\n");
                    if (cur != inbuf) free(cur);
                    free(inbuf);
                    return -1;
                }
            } else {
                fprintf(stderr, "error: algoritmo de compresión '%s' no soportado\n", alg);
                if (cur != inbuf) free(cur);
                free(inbuf);
                return -1;
            }
        } else if (op == 'd'){      // descomprimir
            const char *alg = opt->comp_alg ? opt->comp_alg : "rle";
            if (strcmp(alg, "rle") == 0){
                if (rle_decompress(cur, curlen, &tmp, &tmplen) != 0){
                    fprintf(stderr, "error: fallo RLE decompress\n");
                    if (cur != inbuf) free(cur);
                    free(inbuf);
                    return -1;
                }
            } else if (strcmp(alg, "lzw") == 0){
                if (lzw_decompress(cur, curlen, &tmp, &tmplen) != 0){
                    fprintf(stderr, "error: fallo LZW decompress\n");
                    if (cur != inbuf) free(cur);
                    free(inbuf);
                    return -1;
                }
            } else if (strcmp(alg, "huffman") == 0){
                if (huffman_decompress(cur, curlen, &tmp, &tmplen) != 0){
                    fprintf(stderr, "error: fallo Huffman decompress\n");
                    if (cur != inbuf) free(cur);
                    free(inbuf);
                    return -1;
                }
            } else {
                fprintf(stderr, "error: algoritmo de compresión '%s' no soportado para -d\n", alg);
                if (cur != inbuf) free(cur);
                free(inbuf);
                return -1;
            }
        } else if (op == 'e'){      // encriptar
            if (!opt->key){
                fprintf(stderr, "error: se pidió -e pero no se pasó -k clave\n");
                if (cur != inbuf) free(cur);
                free(inbuf);
                return -1;
            }
            const char *alg = opt->enc_alg ? opt->enc_alg : "vigenere";
            if (strcmp(alg, "vigenere") == 0){
                if (vig_encrypt(cur, curlen,
                                (const uint8_t*)opt->key, strlen(opt->key),
                                &tmp, &tmplen) != 0){
                    fprintf(stderr, "error: fallo vigenere encrypt\n");
                    if (cur != inbuf) free(cur);
                    free(inbuf);
                    return -1;
                }
            } else if (strcmp(alg, "des") == 0){
                if (des_encrypt(cur, curlen,
                                (const uint8_t*)opt->key, strlen(opt->key),
                                &tmp, &tmplen) != 0){
                    fprintf(stderr, "error: fallo DES encrypt\n");
                    if (cur != inbuf) free(cur);
                    free(inbuf);
                    return -1;
                }
            } else if (strcmp(alg, "aes") == 0){
                if (aes_encrypt(cur, curlen,
                                (const uint8_t*)opt->key, strlen(opt->key),
                                &tmp, &tmplen) != 0){
                    fprintf(stderr, "error: fallo AES encrypt\n");
                    if (cur != inbuf) free(cur);
                    free(inbuf);
                    return -1;
                }
            } else {
                fprintf(stderr, "error: algoritmo de encriptación '%s' no soportado\n", alg);
                if (cur != inbuf) free(cur);
                free(inbuf);
                return -1;
            }
        } else if (op == 'u'){      // desencriptar
            if (!opt->key){
                fprintf(stderr, "error: se pidió -u pero no se pasó -k clave\n");
                if (cur != inbuf) free(cur);
                free(inbuf);
                return -1;
            }
            const char *alg = opt->enc_alg ? opt->enc_alg : "vigenere";
            if (strcmp(alg, "vigenere") == 0){
                if (vig_decrypt(cur, curlen,
                                (const uint8_t*)opt->key, strlen(opt->key),
                                &tmp, &tmplen) != 0){
                    fprintf(stderr, "error: fallo vigenere decrypt\n");
                    if (cur != inbuf) free(cur);
                    free(inbuf);
                    return -1;
                }
            } else if (strcmp(alg, "des") == 0){
                if (des_decrypt(cur, curlen,
                                (const uint8_t*)opt->key, strlen(opt->key),
                                &tmp, &tmplen) != 0){
                    fprintf(stderr, "error: fallo DES decrypt\n");
                    if (cur != inbuf) free(cur);
                    free(inbuf);
                    return -1;
                }
            } else if (strcmp(alg, "aes") == 0){
                if (aes_decrypt(cur, curlen,
                                (const uint8_t*)opt->key, strlen(opt->key),
                                &tmp, &tmplen) != 0){
                    fprintf(stderr, "error: fallo AES decrypt\n");
                    if (cur != inbuf) free(cur);
                    free(inbuf);
                    return -1;
                }
            } else {
                fprintf(stderr, "error: algoritmo de encriptación '%s' no soportado para -u\n", alg);
                if (cur != inbuf) free(cur);
                free(inbuf);
                return -1;
            }
        } else {
            fprintf(stderr, "error: operación desconocida '%c'\n", op);
            if (cur != inbuf) free(cur);
            free(inbuf);
            return -1;
        }

        if (tmp){
            if (cur != inbuf) free(cur);
            cur = tmp;
            curlen = tmplen;
        }
    }

    // 3. escribir archivo de salida
    int fd_out = open(opt->out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0){
        perror("open out");
        if (cur != inbuf) free(cur);
        free(inbuf);
        return -1;
    }
    if (curlen > 0){
        ssize_t w = write(fd_out, cur, curlen);
        if (w != (ssize_t)curlen){
            perror("write");
            close(fd_out);
            if (cur != inbuf) free(cur);
            free(inbuf);
            return -1;
        }
    }
    close(fd_out);

    if (cur != inbuf) free(cur);
    free(inbuf);
    return 0;
}
