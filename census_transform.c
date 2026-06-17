#include "census_transform.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>  // Necessario para malloc e free
#include <dirent.h>

/*=============================
  IMPLEMENTACAO DAS FUNCOES
=============================*/

/**
 * @brief Implementacao da leitura PGM (Alocacao Dinamica sem limites).
 */
unsigned char* read_pgm_p2(const char *filename, int *width, int *height) {
    FILE *file = fopen(filename, "r");
    char magic[3];
    int pixel, total;

    if (!file) return NULL;

    if (fscanf(file, "%2s", magic) != 1 || strcmp(magic, "P2") != 0) {
        fclose(file);
        return NULL;
    }

    if (fscanf(file, "%d %d", width, height) != 2) {
        fclose(file);
        return NULL;
    }

    // Consome o 255 do cabecalho
    int max_val;
    fscanf(file, "%d", &max_val);

    total = (*width) * (*height);

    // ALOCACAO DINAMICA: Pede memoria sob medida para o tamanho da foto
    unsigned char *image = (unsigned char *)malloc(total * sizeof(unsigned char));

    if (image == NULL) {
        printf("Erro: Faltou memoria no sistema para alocar a imagem!\n");
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < total; i++) {
        if (fscanf(file, "%d", &pixel) != 1) {
            free(image); // Libera a memoria se der erro na metade
            fclose(file);
            return NULL;
        }
        image[i] = (unsigned char)pixel;
    }

    fclose(file);
    return image; // Retorna o ponteiro com a foto carregada
}

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

void clear_image(unsigned char image[], int width, int height) {
    for (int i = 0; i < width * height; i++) {
        image[i] = 0;
    }
}

void census_transform_3x3(const unsigned char input[], unsigned char output[], int width, int height) {
    clear_image(output, width, height);

    for (int r = 1; r < height - 1; r++) {
        for (int c = 1; c < width - 1; c++) {

            unsigned int census = 0;
            unsigned char center = input[r * width + c];

            for (int i = r - 1; i <= r + 1; i++) {
                for (int j = c - 1; j <= c + 1; j++) {
                    if (i == r && j == c) continue;
                    census <<= 1;
                    if (input[i * width + j] >= center)
                        census |= 1;
                }
            }
            output[r * width + c] = (unsigned char)census;
        }
    }
}

/**
 * @brief Funcao principal.
 */
int main(void) {
    DIR *d;
    struct dirent *dir;
    int imagens_processadas = 0;

    d = opendir(".");

    if (d) {
        while ((dir = readdir(d)) != NULL) {

            char *ext = strrchr(dir->d_name, '.');

            if (ext && strcmp(ext, ".pgm") == 0) {

                if (strncmp(dir->d_name, "census_output", 13) == 0) {
                    continue;
                }

                char input_filename[256];
                char output_filename[256];

                strcpy(input_filename, dir->d_name);
                sprintf(output_filename, "census_output_c_%s", input_filename);

                int w, h;

                // Recebe o ponteiro dinamico da imagem lida
                unsigned char *img_in_dinamica = read_pgm_p2(input_filename, &w, &h);

                if (!img_in_dinamica) {
                    printf("Falha ao ler o arquivo %s. Pulando...\n", input_filename);
                    continue;
                }

                // Cria um ponteiro dinamico com o mesmo tamanho exato para a saida
                unsigned char *img_out_dinamica = (unsigned char *)malloc(w * h * sizeof(unsigned char));

                if (!img_out_dinamica) {
                    printf("Erro ao alocar memoria para a saida da imagem %s\n", input_filename);
                    free(img_in_dinamica); // Libera a entrada antes de pular
                    continue;
                }

                census_transform_3x3(img_in_dinamica, img_out_dinamica, w, h);

                if (!write_pgm_p2(output_filename, img_out_dinamica, w, h)) {
                    printf("Erro fatal ao salvar %s\n", output_filename);
                } else {
                    printf("[Codigo C] Sucesso (%dx%d pixels): %s -> %s\n", w, h, input_filename, output_filename);
                    imagens_processadas++;
                }

                // LIMPEZA OBRIGATORIA: Devolve a memoria pro Mac ao fim de cada foto
                free(img_in_dinamica);
                free(img_out_dinamica);
            }
        }
        closedir(d);
    }

    if (imagens_processadas == 0) {
        printf("Aviso: Nenhuma imagem .pgm valida foi encontrada.\n");
    } else {
        printf("\n=========================================\n");
        printf("Concluido! Total de %d imagens processadas.\n", imagens_processadas);
        printf("=========================================\n");
    }

    return 0;
}