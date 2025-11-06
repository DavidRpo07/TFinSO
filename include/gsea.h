#ifndef GSEA_H
#define GSEA_H

typedef enum {
    OP_NONE = 0,
    OP_COMPRESS = 1,
    OP_DECOMPRESS = 2,
    OP_ENCRYPT = 4,
    OP_DECRYPT = 8
} gsea_op_t;

typedef struct {
    char ops_order[4];    // para guardar 'c','d','e','u' en el orden que llegan
    int  ops_count;
    const char *in_path;  // -i
    const char *out_path; // -o
    const char *key;      // -k (opcional)
    const char *comp_alg; // --comp-alg
    const char *enc_alg;  // --enc-alg
} gsea_opts_t;

// helpers actuales
int fs_is_dir(const char *path);              // 1 si dir, 0 si no, -1 error
int fs_copy_file(const char *infile, const char *outfile);
int fs_list_dir(const char *dirpath);

#endif
