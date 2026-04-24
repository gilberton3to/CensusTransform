//
// Created by Gilberto Neto on 22/04/26.
//

#include <stdio.h>

#define MAX_WIDTH 89
#define MAX_HEIGHT 89

unsigned char imgIn[MAX_WIDTH * MAX_HEIGHT];
unsigned char imgTemp[MAX_WIDTH * MAX_HEIGHT];

int main() {
    FILE *f_in, *f_out;
    char magic[3];
    int width, height, max_val;
    int pixel;
    int x, y, i, j;
    int m = 3, n = 3;

    f_in = fopen("input.pgm", "r");
    if (!f_in) {
        printf("Erro: Nao abriu input.pgm\n");
        return 1;
    }

    fscanf(f_in, "%2s", magic);
    fscanf(f_in, "%d %d", &width, &height);
    fscanf(f_in, "%d", &max_val);

    for (x = 0; x < width * height; x++) {
        fscanf(f_in, "%d", &pixel);
        imgIn[x] = (unsigned char)pixel;
    }
    fclose(f_in);

    for (x = 0; x < width * height; x++) {
        imgTemp[x] = 0;
    }

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

    f_out = fopen("output.pgm", "w");
    if (!f_out) {
        printf("Erro ao criar output.pgm\n");
        return 1;
    }

    fprintf(f_out, "P2\n");
    fprintf(f_out, "%d %d\n", width, height);
    fprintf(f_out, "255\n");

    for (x = 0; x < height; x++) {
        for (y = 0; y < width; y++) {
            fprintf(f_out, "%d ", imgTemp[x * width + y]);
        }
        fprintf(f_out, "\n");
    }
    fclose(f_out);

    printf("Sucesso! O algoritmo rodou e gerou o output.pgm\n");

    return 0;
}