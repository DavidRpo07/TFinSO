#!/bin/bash
# Script de pruebas para LZW y DES

echo "=== Script de Pruebas - LZW y DES ==="
echo ""

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Compilar
echo -e "${YELLOW}[1/8] Compilando proyecto...${NC}"
make clean > /dev/null 2>&1
if make; then
    echo -e "${GREEN}✓ Compilación exitosa${NC}"
else
    echo -e "${RED}✗ Error en compilación${NC}"
    exit 1
fi
echo ""

# Crear directorio de pruebas
mkdir -p pruebas_temp
cd pruebas_temp

# Test 1: LZW Compresión/Descompresión
echo -e "${YELLOW}[2/8] Test LZW con datos repetitivos...${NC}"
echo "AAAAAABBBBBCCCCCDDDDDEEEEEFFFFFGGGGGHHHHHIIIIIJJJJJ" > test_lzw.txt
../gsea -c -i test_lzw.txt -o test_lzw.compressed --comp-alg lzw
../gsea -d -i test_lzw.compressed -o test_lzw_restored.txt --comp-alg lzw
if diff test_lzw.txt test_lzw_restored.txt > /dev/null; then
    SIZE_ORIG=$(wc -c < test_lzw.txt)
    SIZE_COMP=$(wc -c < test_lzw.compressed)
    echo -e "${GREEN}✓ LZW correcto${NC} (Original: ${SIZE_ORIG}B, Comprimido: ${SIZE_COMP}B)"
else
    echo -e "${RED}✗ LZW falló - archivos difieren${NC}"
fi
echo ""

# Test 2: LZW con texto real
echo -e "${YELLOW}[3/8] Test LZW con texto real...${NC}"
cat > test_text.txt << 'EOF'
Este es un texto de prueba para el algoritmo LZW.
LZW es un algoritmo de compresión sin pérdida.
Los algoritmos de compresión son muy útiles.
Este texto tiene palabras repetidas para probar LZW.
EOF
../gsea -c -i test_text.txt -o test_text.lzw --comp-alg lzw
../gsea -d -i test_text.lzw -o test_text_restored.txt --comp-alg lzw
if diff test_text.txt test_text_restored.txt > /dev/null; then
    echo -e "${GREEN}✓ LZW con texto real correcto${NC}"
else
    echo -e "${RED}✗ LZW con texto real falló${NC}"
fi
echo ""

# Test 3: DES Cifrado/Descifrado
echo -e "${YELLOW}[4/8] Test DES cifrado/descifrado...${NC}"
echo "Mensaje secreto para cifrar con DES" > test_des.txt
../gsea -e -i test_des.txt -o test_des.encrypted -k "clave123" --enc-alg des
../gsea -u -i test_des.encrypted -o test_des_decrypted.txt -k "clave123" --enc-alg des
if diff test_des.txt test_des_decrypted.txt > /dev/null; then
    echo -e "${GREEN}✓ DES correcto${NC}"
else
    echo -e "${RED}✗ DES falló - archivos difieren${NC}"
fi
echo ""

# Test 4: DES con clave incorrecta
echo -e "${YELLOW}[5/8] Test DES con clave incorrecta...${NC}"
echo "Test de seguridad DES" > test_des_sec.txt
../gsea -e -i test_des_sec.txt -o test_des_sec.enc -k "correcta" --enc-alg des > /dev/null 2>&1
../gsea -u -i test_des_sec.enc -o test_des_sec.dec -k "incorrec" --enc-alg des > /dev/null 2>&1
if ! diff test_des_sec.txt test_des_sec.dec > /dev/null 2>&1; then
    echo -e "${GREEN}✓ DES rechaza clave incorrecta correctamente${NC}"
else
    echo -e "${RED}✗ DES aceptó clave incorrecta (problema de seguridad)${NC}"
fi
echo ""

# Test 5: Combinación LZW + DES
echo -e "${YELLOW}[6/8] Test combinado LZW + DES...${NC}"
cat > test_combined.txt << 'EOF'
Este es un archivo que será comprimido con LZW y luego cifrado con DES.
La combinación de compresión y cifrado es muy común en aplicaciones reales.
LZW reducirá el tamaño y DES protegerá la confidencialidad.
EOF
../gsea -ce -i test_combined.txt -o test_combined.lzw.des --comp-alg lzw --enc-alg des -k "test1234"
../gsea -ud -i test_combined.lzw.des -o test_combined_restored.txt --comp-alg lzw --enc-alg des -k "test1234"
if diff test_combined.txt test_combined_restored.txt > /dev/null; then
    echo -e "${GREEN}✓ Combinación LZW + DES correcta${NC}"
else
    echo -e "${RED}✗ Combinación LZW + DES falló${NC}"
fi
echo ""

# Test 6: Combinación RLE + DES
echo -e "${YELLOW}[7/8] Test combinado RLE + DES...${NC}"
echo "AAAAAABBBBBCCCCCDDDDD" > test_rle_des.txt
../gsea -ce -i test_rle_des.txt -o test_rle_des.out --comp-alg rle --enc-alg des -k "rlekey12"
../gsea -ud -i test_rle_des.out -o test_rle_des_restored.txt --comp-alg rle --enc-alg des -k "rlekey12"
if diff test_rle_des.txt test_rle_des_restored.txt > /dev/null; then
    echo -e "${GREEN}✓ Combinación RLE + DES correcta${NC}"
else
    echo -e "${RED}✗ Combinación RLE + DES falló${NC}"
fi
echo ""

# Test 7: Archivo binario con DES
echo -e "${YELLOW}[8/8] Test DES con datos binarios...${NC}"
dd if=/dev/urandom of=test_binary.dat bs=1024 count=1 > /dev/null 2>&1
../gsea -e -i test_binary.dat -o test_binary.des -k "binary12" --enc-alg des
../gsea -u -i test_binary.des -o test_binary_restored.dat -k "binary12" --enc-alg des
if diff test_binary.dat test_binary_restored.dat > /dev/null; then
    echo -e "${GREEN}✓ DES con datos binarios correcto${NC}"
else
    echo -e "${RED}✗ DES con datos binarios falló${NC}"
fi
echo ""

# Limpiar
cd ..
rm -rf pruebas_temp

echo "=== Resumen de Pruebas ==="
echo -e "${GREEN}Todas las pruebas completadas${NC}"
echo "Revisa los resultados arriba para verificar que todo pasó correctamente."
