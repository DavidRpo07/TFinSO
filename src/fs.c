#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>     
#include <fcntl.h>      
#include <sys/stat.h>  
#include <sys/types.h>
#include <dirent.h>     
#include "gsea.h"

int fs_is_dir(const char *path){
    struct stat st;
    if (stat(path, &st) == -1) return -1;
    return S_ISDIR(st.st_mode) ? 1 : 0;
}

// Copia archivo
int fs_copy_file(const char *infile, const char *outfile){
    int fd_in = open(infile, O_RDONLY);
    if (fd_in == -1){ perror("open in"); return -1; }

    // Crear o Truncar salida
    int fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1){ perror("open out"); close(fd_in); return -1; }

    char buf[8192];
    for(;;){
        ssize_t n = read(fd_in, buf, sizeof(buf));
        if (n < 0){ if (errno == EINTR) continue; perror("read"); close(fd_in); close(fd_out); return -1; }
        if (n == 0) break; // fin de archivo
        ssize_t escritos = 0;
        while (escritos < n){
            ssize_t m = write(fd_out, buf + escritos, n - escritos);
            if (m < 0){ if (errno == EINTR) continue; perror("write"); close(fd_in); close(fd_out); return -1; }
            escritos += m;
        }
    }

    if (close(fd_in)  == -1) perror("close in");
    if (close(fd_out) == -1) perror("close out");
    return 0;
}

// Lista archivos regulares dentro de un directorio
int fs_list_dir(const char *dirpath){
    DIR *d = opendir(dirpath);
    if (!d){ perror("opendir"); return -1; }

    struct dirent *de;
    while ((de = readdir(d))){
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;
        char full[4096];
        int r = snprintf(full, sizeof(full), "%s/%s", dirpath, de->d_name);
        if (r < 0 || r >= (int)sizeof(full)) { fprintf(stderr, "ruta muy larga\n"); continue; }

        struct stat st; if (stat(full, &st) == -1){ perror("stat child"); continue; }
        if (S_ISREG(st.st_mode)){
            printf("- %s\n", de->d_name);
        }
    }
    closedir(d);
    return 0;
}
