# TFinSO — Guía de uso

Breve guía para compilar y ejecutar el programa de compresión y cifrado incluido en este repositorio.

## Compilar

1. Asegúrate de tener un compilador C (gcc) y make instalados.
2. En la raíz del proyecto ejecuta:

```sh
make clean && make
```

El binario generado se llama `gsea`.

## Uso básico

Sintaxis general:

```sh
./gsea -[c|d][e|u] -i <entrada> -o <salida> [--comp-alg rle|lzw|huffman] [--enc-alg vigenere|des|aes] [-k <clave>]
```

- `-c` : comprimir
- `-d` : descomprimir
- `-e` : encriptar
- `-u` : desencriptar
- `-i` : archivo o directorio de entrada
- `-o` : archivo o directorio de salida
- `--comp-alg` : algoritmo de compresión (por defecto `rle`)
- `--enc-alg`  : algoritmo de cifrado (por defecto `vigenere`)
- `-k` : clave para cifrado/descifrado (obligatoria para `-e`/`-u`)

Nota: el orden de las operaciones sigue el orden en que se pasan las opciones. Por ejemplo `-ce` significa primero comprimir y luego encriptar; `-ec` haría lo contrario.

## Ejemplos

- Comprimir un solo archivo con Huffman:

```sh
./gsea -c -i audio.wav -o audio.huff --comp-alg huffman
```

- Encriptar un archivo con DES (clave de al menos 8 bytes):

```sh
./gsea -e -i cod.txt -o cod.des -k "miClave8" --enc-alg des
```

- Comprimir con LZW y luego encriptar con AES (clave >= 16 bytes):

```sh
./gsea -ce -i cod.txt -o dcod.lzw.aes --comp-alg lzw --enc-alg aes -k "0123456789abcdef"
```

- Procesar un directorio completo:

```sh
./gsea -c -i directorio_pruebas -o output_dir --comp-alg lzw
```

El programa procesará los archivos regulares en el directorio de entrada concurrentemente usando un pool de trabajadores (número de workers = min(n_files, cores*4)).

## Requisitos de clave

- Vigenere: acepta cualquier longitud de clave > 0.
- DES: la implementación requiere al menos 8 bytes de clave (56 bits efectivos).
- AES: la implementación del proyecto requiere al menos 16 bytes de clave (128 bits).

## Notas y recomendaciones

- Recomendación práctica: siempre comprime antes de cifrar (`-c` antes de `-e`) para obtener mejor tasa de compresión.
- El procesamiento de directorios es concurrente; los mensajes de progreso/errores se escriben por stderr.

