#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include "gsea.h"
#include "pipeline.h"

static int parse_args(int argc, char **argv, gsea_opts_t *opt){
    memset(opt, 0, sizeof(*opt));
    static struct option longopts[] = {
        {"comp-alg", required_argument, 0, 1000},
        {"enc-alg",  required_argument, 0, 1001},
        {0,0,0,0}
    };
    int c;
    while ((c = getopt_long(argc, argv, "cdeui:o:k:", longopts, NULL)) != -1){
        switch(c){
        case 'c': opt->ops_order[opt->ops_count++] = 'c'; break;
        case 'd': opt->ops_order[opt->ops_count++] = 'd'; break;
        case 'e': opt->ops_order[opt->ops_count++] = 'e'; break;
        case 'u': opt->ops_order[opt->ops_count++] = 'u'; break;
        case 'i': opt->in_path = optarg; break;
        case 'o': opt->out_path = optarg; break;
        case 'k': opt->key = optarg; break;
        case 1000: opt->comp_alg = optarg; break;
        case 1001: opt->enc_alg  = optarg; break;
        default:
            fprintf(stderr,
              "Uso: %s -[c|d][e|u] -i in -o out [--comp-alg rle|lzw] [--enc-alg vigenere] [-k clave]\n",
               argv[0]);
            return -1;
        }
    }
    if (!opt->in_path || !opt->out_path){
        fprintf(stderr, "Error: faltan -i o -o\n");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv){
    gsea_opts_t opt;
    if (parse_args(argc, argv, &opt) != 0) return 1;

    int isdir = fs_is_dir(opt.in_path);
    if (isdir == -1){
        perror("stat in");
        return 1;
    }

    if (!isdir){
    if (gsea_process_file(&opt) != 0){
        fprintf(stderr, "error procesando archivo\n");
        return 1;
    }
    return 0;
    } else {
        // CASO 2: directorio
        // solo listar
        printf("Directorio detectado: '%s'\n", opt.in_path);
        if (fs_list_dir(opt.in_path) != 0) return 1;
    }
    return 0;
}
