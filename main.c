//
// Created by Gilberto Neto on 22/04/26.
//

#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
    const char* imgName = "imL_re.png";
    int width, height, channels;

    unsigned char *imgIn = stbi_load(imgName, &width, &height, &channels, 1);
    if (!imgIn) {
        fprintf(stderr, "Erro: nao foi possivel abrir a imagem %s\n", imgName);
        return 1;
    }

    unsigned char *imgTemp = (unsigned char *)calloc(width * height, sizeof(unsigned char));
    if (!imgTemp) {
        fprintf(stderr, "Erro: falha ao alocar memoria para a saida\n");
        stbi_image_free(imgIn);
        return 1;
    }

    int m = 3;
    int n = 3;
    int x, y, i, j;

    for (x = m / 2; x < height - m / 2; x++) {
        for (y = n / 2; y < width - n / 2; y++) {
            unsigned int census = 0;
            int shiftCount = 0;

            unsigned char center_pixel = imgIn[x * width + y];

            for (i = x - m / 2; i <= x + m / 2; i++) {
                for (j = y - n / 2; j <= y + n / 2; j++) {
                    if (shiftCount != m * n / 2) {
                        census <<= 1;
                        unsigned char current_pixel = imgIn[i * width + j];

                        if (current_pixel < center_pixel) {
                            census += 1;
                        }
                    }
                    shiftCount++;
                }
            }

            imgTemp[x * width + y] = (unsigned char)census;
        }
    }

    stbi_write_png("output.png", width, height, 1, imgTemp, width);
    printf("Sucesso! Imagem salva como 'output.png'.\n");

    stbi_image_free(imgIn);
    free(imgTemp);

    return 0;
}