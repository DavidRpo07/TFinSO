#ifndef GSEA_H
#define GSEA_H



typedef struct {
    const char *in_path; // -i
    const char *out_path; // -o
} gsea_opts_t;


// helpers 
int fs_is_dir(const char *path); // 1 si dir, 0 si no, -1 error
int fs_copy_file(const char *infile, const char *outfile); // copia
int fs_list_dir(const char *dirpath); // lista archivos de directorios


#endif 