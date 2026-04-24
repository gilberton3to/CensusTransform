//
// Created by Gilberto Neto on 22/04/26.
//

#include <stdio.h>
#include <string.h>

#define MAX_WIDTH 89
#define MAX_HEIGHT 89
#define MAX_PIXELS (MAX_WIDTH * MAX_HEIGHT)

#define INPUT_FILE "input.pgm"
#define OUTPUT_FILE "output-census.pgm"

unsigned char img_in[MAX_PIXELS];
unsigned char img_out[MAX_PIXELS];

int read_pgm_p2(const char *filename, unsigned char image[], int *width, int *height) {
    FILE *file = fopen(filename, "r");
    char magic[3];
    int pixel;
    int total_pixels;

    if (file == NULL) {
        printf("Erro: nao foi possivel abrir o arquivo de entrada: %s\n", filename);
        return 0;
    }

    if (fscanf(file, "%2s", magic) != 1 || strcmp(magic, "P2") != 0) {
        printf("Erro: formato invalido. Esperado PGM P2.\n");
        fclose(file);
        return 0;
    }

    if (fscanf(file, "%d %d", width, height) != 2) {
        printf("Erro: nao foi possivel ler as dimensoes da imagem.\n");
        fclose(file);
        return 0;
    }

    if (*width <= 0 || *height <= 0 || *width > MAX_WIDTH || *height > MAX_HEIGHT) {
        printf("Erro: tamanho de imagem invalido: %d x %d\n.", *width, *height);
        fclose(file);
        return 0;
    }

    total_pixels = (*width) * (*height);

    for (int k = 0; k < total_pixels; k++) {
        if (fscanf(file, "%d", &pixel) != 1) {
            printf("Erro: dados de pixel insuficientes na posicao %d.\n", k);
            fclose(file);
            return 0;
        }

        image[k] = (unsigned char)pixel;
    }

    fclose(file);
    return 1;
}

int write_pgm_p2(const char *filename, const unsigned char image[], int width, int height) {
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        printf("Erro: nao foi possivel criar o arquivo de saida: %s\n", filename);
        return 0;
    }

    fprintf(file, "P2\n");
    fprintf(file, "%d %d\n", width, height);
    fprintf(file, "255\n");

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            fprintf(file, "%d ", image[row * width + col]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    return 1;
}

void clear_image(unsigned char image[], int width, int height) {
    for (int k = 0; k < width * height; k++) {
        image[k] = 0;
    }
}

void census_transform_3x3(
    const unsigned char input[],
    unsigned char output[],
    int width,
    int height
) {
    clear_image(output, width, height);

    for (int row = 1; row < height - 1; row++) {
        for (int col = 1; col < width - 1; col++) {
            unsigned int census = 0;
            unsigned char center = input[row * width + col];

            for (int i = row - 1; i <= row + 1; i++) {
                for (int j = col - 1; j <= col + 1; j++) {
                    if (i == row && j == col) continue;

                    census <<= 1;

                    if (input[i * width + j] >= center) {
                        census |= 1;
                    }
                }
            }

            output[row * width + col] = (unsigned char)census;
        }
    }
}

int test_census_transform_3x3(void) {
    const int width = 3;
    const int height = 3;

    unsigned char input[9] = {
        10, 20, 30,
        40, 50, 60,
        70, 80, 90
    };

    unsigned char output[9];

    census_transform_3x3(input, output, width, height);

    if (output[4] != 15) {
        printf("Teste falhou: esperado 15, obtido %d\n", output[4]);
        return 0;
    }

    printf("Teste passou: census_transform_3x3\n");
    return 1;
}

int main(void) {
    int width;
    int height;

    printf("Executando teste interno...\n");

    if (!test_census_transform_3x3()) {
        printf("Erro: falha no teste interno.\n");
        return 1;
    }

    printf("Lendo imagem de entrada...\n");

    if (!read_pgm_p2(INPUT_FILE, img_in, &width, &height)) {
        return 1;
    }

    printf("Imagem carregada: %d x %d\n", width, height);

    census_transform_3x3(img_in, img_out, width, height);

    printf("Salvando imagem de saida...\n");

    if (!write_pgm_p2(OUTPUT_FILE, img_out, width, height)) {
        return 1;
    }

    printf("Sucesso! Arquivo gerado: %s\n", OUTPUT_FILE);

    return 0;
}