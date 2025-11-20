#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <stdint.h>
#include "gsea.h"
#include "pipeline.h"

/* -------- utilidades básicas de fs que ya tenías -------- */

int fs_is_dir(const char *path){
    struct stat st;
    if (stat(path, &st) == -1) return -1;
    return S_ISDIR(st.st_mode) ? 1 : 0;
}

int fs_copy_file(const char *infile, const char *outfile){
    int fd_in = open(infile, O_RDONLY);
    if (fd_in == -1){ perror("open in"); return -1; }

    int fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1){ perror("open out"); close(fd_in); return -1; }

    char buf[8192];
    for(;;){
        ssize_t n = read(fd_in, buf, sizeof(buf));
        if (n < 0){
            if (errno == EINTR) continue;
            perror("read");
            close(fd_in); close(fd_out);
            return -1;
        }
        if (n == 0) break;
        ssize_t escritos = 0;
        while (escritos < n){
            ssize_t m = write(fd_out, buf + escritos, n - escritos);
            if (m < 0){
                if (errno == EINTR) continue;
                perror("write");
                close(fd_in); close(fd_out);
                return -1;
            }
            escritos += m;
        }
    }

    if (close(fd_in)  == -1) perror("close in");
    if (close(fd_out) == -1) perror("close out");
    return 0;
}

int fs_list_dir(const char *dirpath){
    DIR *d = opendir(dirpath);
    if (!d){ perror("opendir"); return -1; }

    struct dirent *de;
    while ((de = readdir(d))){
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;
        char full[4096];
        int r = snprintf(full, sizeof(full), "%s/%s", dirpath, de->d_name);
        if (r < 0 || r >= (int)sizeof(full)){
            fprintf(stderr, "ruta muy larga\n");
            continue;
        }
        struct stat st;
        if (stat(full, &st) == -1){
            perror("stat child");
            continue;
        }
        if (S_ISREG(st.st_mode)){
            printf("- %s\n", de->d_name);
        }
    }
    closedir(d);
    return 0;
}

int fs_ensure_dir(const char *dirpath){
    struct stat st;
    if (stat(dirpath, &st) == 0){
        if (S_ISDIR(st.st_mode)) return 0;
        errno = ENOTDIR;
        return -1;
    }
    if (mkdir(dirpath, 0755) == -1){
        perror("mkdir salida");
        return -1;
    }
    return 0;
}

/* -------- estructuras para el pool de hilos -------- */

struct thread_arg {
    gsea_opts_t base;      // copia de opciones
    char in_file[4096];    // ruta completa de entrada
    char out_file[4096];   // ruta completa de salida
};

struct pool_ctx {
    struct thread_arg *args;  // array de trabajos (uno por archivo)
    size_t count;             // número total de archivos
    size_t next;              // índice del próximo archivo a asignar
    pthread_mutex_t lock;     // protege 'next'
};

/* -------- worker de pool: un hilo procesa N archivos -------- */

static void *thread_worker(void *ptr){
    struct pool_ctx *ctx = (struct pool_ctx*)ptr;

    for(;;){
        // Tomar el siguiente índice disponible
        pthread_mutex_lock(&ctx->lock);
        if (ctx->next >= ctx->count){
            pthread_mutex_unlock(&ctx->lock);
            break; // no hay más archivos
        }
        size_t idx = ctx->next++;
        pthread_mutex_unlock(&ctx->lock);

        struct thread_arg *a = &ctx->args[idx];

        pthread_t tid = pthread_self();
        fprintf(stderr,
                "[hilo %lu] inicio %s -> %s (idx=%zu)\n",
                (unsigned long)tid, a->in_file, a->out_file, idx);

        // ajustar rutas en la copia de opciones
        a->base.in_path  = a->in_file;
        a->base.out_path = a->out_file;

        int rc = gsea_process_file(&a->base);
        if (rc != 0){
            fprintf(stderr,
                    "[hilo %lu] fallo %s -> %s rc=%d\n",
                    (unsigned long)tid, a->in_file, a->out_file, rc);
        } else {
            fprintf(stderr,
                    "[hilo %lu] OK %s -> %s\n",
                    (unsigned long)tid, a->in_file, a->out_file);
        }
    }
    return NULL;
}

/* -------- función principal de procesamiento concurrente -------- */

int fs_process_dir_concurrent(const gsea_opts_t *opt){
    if (fs_ensure_dir(opt->out_path) != 0){
        return -1;
    }

    DIR *d = opendir(opt->in_path);
    if (!d){ perror("opendir in"); return -1; }

    /* 1) Contar archivos regulares */
    size_t count = 0;
    struct dirent *de;
    while ((de = readdir(d))){
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;
        char full[4096];
        int r = snprintf(full, sizeof(full), "%s/%s", opt->in_path, de->d_name);
        if (r < 0 || r >= (int)sizeof(full)) continue;
        struct stat st;
        if (stat(full, &st) == -1) continue;
        if (S_ISREG(st.st_mode)) count++;
    }

    if (count == 0){
        fprintf(stderr, "No hay archivos regulares en el directorio.\n");
        closedir(d);
        return 0;
    }

    rewinddir(d);

    /* 2) Preparar array de trabajos (uno por archivo) */
    struct thread_arg *args = calloc(count, sizeof(*args));
    if (!args){
        perror("calloc args");
        closedir(d);
        return -1;
    }

    size_t idx = 0;
    while ((de = readdir(d))){
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;
        char full[4096];
        int r = snprintf(full, sizeof(full), "%s/%s", opt->in_path, de->d_name);
        if (r < 0 || r >= (int)sizeof(full)) continue;
        struct stat st;
        if (stat(full, &st) == -1) continue;
        if (!S_ISREG(st.st_mode)) continue;

        struct thread_arg *a = &args[idx];
        a->base = *opt; // copia superficial de opciones
        snprintf(a->in_file,  sizeof(a->in_file),  "%s", full);
        snprintf(a->out_file, sizeof(a->out_file), "%s/%s",
                 opt->out_path, de->d_name);

        fprintf(stderr,
                "[prep] idx=%zu archivo=%s -> %s\n",
                idx, a->in_file, a->out_file);

        idx++;
    }
    closedir(d);

    // puede que count > idx si algunos se saltaron por errores; ajustar
    count = idx;
    if (count == 0){
        free(args);
        fprintf(stderr, "No se pudo preparar ningún archivo.\n");
        return 0;
    }

    /* 3) Calcular número de hilos: min( archivos, nucleos*4 ) */
    long cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (cores < 1) cores = 1;

    size_t max_workers = (size_t)cores * 4;
    if (max_workers > count) max_workers = count;

    fprintf(stderr,
            "[pool] archivos=%zu, cores=%ld, workers=%zu\n",
            count, cores, max_workers);

    pthread_t *tids = calloc(max_workers, sizeof(*tids));
    if (!tids){
        perror("calloc tids");
        free(args);
        return -1;
    }

    struct pool_ctx ctx;
    ctx.args = args;
    ctx.count = count;
    ctx.next  = 0;
    pthread_mutex_init(&ctx.lock, NULL);

    /* 4) Crear los workers */
    for (size_t i = 0; i < max_workers; i++){
        int err = pthread_create(&tids[i], NULL, thread_worker, &ctx);
        if (err != 0){
            fprintf(stderr,
                    "[pool] pthread_create fallo i=%zu: %s\n",
                    i, strerror(err));
            // dejamos max_workers en el número realmente creado
            max_workers = i;
            break;
        } else {
            fprintf(stderr,
                    "[pool] worker i=%zu tid=%lu creado\n",
                    i, (unsigned long)tids[i]);
        }
    }

    /* 5) Esperar a que todos los workers terminen */
    int global_rc = 0;
    for (size_t i = 0; i < max_workers; i++){
        void *ret = NULL;
        int err = pthread_join(tids[i], &ret);
        if (err != 0){
            fprintf(stderr,
                    "[join] worker i=%zu fallo join: %s\n",
                    i, strerror(err));
            global_rc = -1;
        } else {
            // no usamos rc por archivo aquí; cada hilo procesó varios
            fprintf(stderr,
                    "[join] worker i=%zu tid=%lu terminado\n",
                    i, (unsigned long)tids[i]);
        }
    }

    pthread_mutex_destroy(&ctx.lock);
    free(tids);
    free(args);
    return global_rc;
}