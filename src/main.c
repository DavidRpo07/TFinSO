#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include "gsea.h"

static int parse_args(int argc, char **argv, gsea_opts_t *opt){
    memset(opt, 0, sizeof(*opt));
    int c;
    while ((c = getopt(argc, argv, "i:o:")) != -1){
        switch(c){
            case 'i': opt->in_path = optarg; break;
            case 'o': opt->out_path = optarg; break;
            default:
                fprintf(stderr, "Error: uso incorrecto. Ejemplo: %s -i entrada -o salida\n", argv[0]);
                return -1;
        }
    }

    if (!opt->in_path || !opt->out_path){
        fprintf(stderr, "Error: faltan argumentos -i o -o.\n");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv){
    gsea_opts_t opt;
    if (parse_args(argc, argv, &opt) != 0) return 1;

    int isdir = fs_is_dir(opt.in_path);
    if (isdir == -1){ perror("stat in"); return 1; }

    if (!isdir){
        if (fs_copy_file(opt.in_path, opt.out_path) != 0){
            fprintf(stderr, "Fallo al copiar '%s' -> '%s'\n", opt.in_path, opt.out_path);
            return 1;
        }
        printf("Copiado OK: %s -> %s\n", opt.in_path, opt.out_path);
        return 0;
    }


    printf("Listando '%s' \n", opt.in_path);
    if (fs_list_dir(opt.in_path) != 0) return 1;
    return 0;
}
