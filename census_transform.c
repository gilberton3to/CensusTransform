/******************************************************************************
 * @file census_transform.c
 * @brief Implementação do Census Transform 3x3 em imagens PGM.
 *
 * @author
 * Gilberto Gomes Soares Neto
 * Izadora de Oliveira Albuquerque Montenegro
 *
 * @date
 * 30/04/2026
 ******************************************************************************/

#include "census_transform.h"
#include <string.h>

/*=============================
  CONFIGURAÇÃO
=============================*/

#define INPUT_FILE "test-1.pgm"
#define OUTPUT_FILE "output-census.pgm"

/*=============================
  VARIÁVEIS GLOBAIS
=============================*/

unsigned char img_in[MAX_PIXELS];
unsigned char img_out[MAX_PIXELS];

/**
 * @brief Implementação da leitura PGM.
 */
int read_pgm_p2(const char *filename, unsigned char image[], int *width, int *height) {
    FILE *file = fopen(filename, "r");
    char magic[3];
    int pixel, total;

    // falha ao abrir arquivo
    if (!file) return 0;

    // verifica se é formato P2
    if (fscanf(file, "%2s", magic) != 1 || strcmp(magic, "P2") != 0) {
        fclose(file);
        return 0;
    }

    // lê dimensões
    if (fscanf(file, "%d %d", width, height) != 2) {
        fclose(file);
        return 0;
    }

    int max_val;
    fscanf(file, "%d", &max_val);

    // valida tamanho mínimo e máximo
    if (*width < 3 || *height < 3 || *width > MAX_WIDTH || *height > MAX_HEIGHT) {
        fclose(file);
        return 0;
    }

    total = (*width) * (*height);

    // lê todos os pixels
    for (int i = 0; i < total; i++) {
        if (fscanf(file, "%d", &pixel) != 1) {
            fclose(file);
            return 0;
        }
        image[i] = (unsigned char)pixel;
    }

    fclose(file);
    return 1;
}

/**
 * @brief Implementação da escrita PGM.
 */
int write_pgm_p2(const char *filename, const unsigned char image[], int width, int height) {
    FILE *file = fopen(filename, "w");
    if (!file) return 0;

    fprintf(file, "P2\n%d %d\n255\n", width, height);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fprintf(file, "%d ", image[i * width + j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    return 1;
}

/**
 * @brief Zera imagem.
 */
void clear_image(unsigned char image[], int width, int height) {
    for (int i = 0; i < width * height; i++) {
        image[i] = 0;
    }
}
/**
 * @brief Implementação do Census Transform.
 */
void census_transform_3x3(
    const unsigned char input[],
    unsigned char output[],
    int width,
    int height
) {
    // garante saída zerada (bordas ficam 0)
    clear_image(output, width, height);

    // percorre apenas pixels internos
    for (int r = 1; r < height - 1; r++) {
        for (int c = 1; c < width - 1; c++) {

            unsigned int census = 0;
            unsigned char center = input[r * width + c];

            // percorre vizinhança 3x3
            for (int i = r - 1; i <= r + 1; i++) {
                for (int j = c - 1; j <= c + 1; j++) {

                    // ignora o próprio centro
                    if (i == r && j == c) continue;

                    // desloca bits para a esquerda
                    census <<= 1;

                    // adiciona 1 se vizinho >= centro
                    if (input[i * width + j] >= center)
                        census |= 1;
                }
            }

            // salva resultado (8 bits)
            output[r * width + c] = (unsigned char)census;
        }
    }
}

/**
 * @brief Função principal.
 */
int main(void) {
    int w, h;

    if (!read_pgm_p2(INPUT_FILE, img_in, &w, &h)) return 1;

    census_transform_3x3(img_in, img_out, w, h);

    if (!write_pgm_p2(OUTPUT_FILE, img_out, w, h)) return 1;

    printf("Execução concluida\n");
    return 0;
}