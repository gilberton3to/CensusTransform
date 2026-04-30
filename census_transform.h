/******************************************************************************
*  @file census_transform.c
 * @brief Aplicação do algoritmo Census Transform 3x3 em imagens PGM (P2).
 *
 * @details
 * Este programa realiza:
 * 1. Leitura de uma imagem PGM P2 (ASCII, escala de cinza);
 * 2. Validação das dimensões da imagem;
 * 3. Aplicação do Census Transform 3x3;
 * 4. Escrita da imagem resultante em um novo arquivo PGM.
 *
 * O Census Transform é um método que descreve o padrão local de intensidades
 * ao redor de um pixel, sem depender diretamente do valor absoluto desse pixel.
 *
 * Para cada pixel interno da imagem, considera-se uma vizinhança 3x3, onde o
 * pixel central é comparado com seus 8 vizinhos.
 *
 * Cada vizinho é analisado individualmente:
 * - se o valor do vizinho for maior ou igual ao valor do pixel central,
 *   é gerado o bit 1;
 * - caso contrário, é gerado o bit 0.
 *
 * Essas comparações são feitas em uma ordem fixa (varredura da janela 3x3),
 * formando uma sequência de 8 bits. Essa sequência representa a estrutura
 * local da imagem ao redor do pixel.
 *
 * O resultado final é um número inteiro de 8 bits (0 a 255), que é armazenado
 * na mesma posição correspondente na imagem de saída.
 *
 * ENTRADA:
 * - Arquivo PGM P2 contendo pixels inteiros (0 a 255)
 *
 * SAÍDA:
 * - Arquivo PGM P2 com os valores do Census Transform (0 a 255)
 *
 * COMO USAR:
 * - Coloque a imagem de entrada com nome definido em INPUT_FILE
 * - Compile e execute o programa
 * - O arquivo de saída será gerado com nome definido em OUTPUT_FILE
 *
 * PLATAFORMA:
 * - Linguagem C
 * - Execução em ambiente desktop (Linux, macOS ou Windows)
 *
 * CONTEXTO:
 * - Trabalho da disciplina de Sistemas Embarcados
 *
 * AUTORES:
 * - Gilberto Gomes Soares Neto
 * - Izadora de Oliveira Albuquerque Montenegro
 *
 * DATA:
 * - 30/04/2026
 *
 * LICENÇA:
 * - Uso acadêmico
 ******************************************************************************/

#ifndef CENSUS_TRANSFORM_H
#define CENSUS_TRANSFORM_H

#include <stdio.h>

/*=============================
  DEFINIÇÕES
=============================*/

/**
 * @def MAX_WIDTH
 * @brief Largura máxima suportada para a imagem de entrada.
 */
#define MAX_WIDTH 89

/**
 * @def MAX_HEIGHT
 * @brief Altura máxima suportada para a imagem de entrada.
 */
#define MAX_HEIGHT 89

/**
 * @def MAX_PIXELS
 * @brief Número máximo de pixels armazenáveis nas imagens de entrada e saída.
 */
#define MAX_PIXELS (MAX_WIDTH * MAX_HEIGHT)

/*=============================
  VARIÁVEIS GLOBAIS
=============================*/

/**
 * @var img_in
 * @brief Vetor que armazena os pixels da imagem de entrada.
 *
 * @details
 * Representa a imagem original em formato linear.
 * Cada posição corresponde a um pixel acessado por:
 *      índice = r * width + c
 */
extern unsigned char img_in[MAX_PIXELS];

/**
 * @var img_out
 * @brief Vetor que armazena os pixels da imagem resultante.
 *
 * @details
 * Contém os valores gerados pelo Census Transform.
 * As bordas permanecem com valor 0.
 */
extern unsigned char img_out[MAX_PIXELS];

/*=============================
  PROTÓTIPOS DAS FUNÇÕES
=============================*/

/**
 * @brief Lê uma imagem no formato PGM P2.
 *
 * @details
 * Esta função abre um arquivo de imagem PGM P2, valida seu cabeçalho,
 * lê suas dimensões e armazena os pixels em um vetor linear.
 *
 * Passo a passo:
 *
 * 1. Abre o arquivo informado em modo de leitura.
 * 2. Lê o identificador do formato, que deve ser "P2".
 * 3. Lê a largura e a altura da imagem.
 * 4. Lê o valor máximo de intensidade dos pixels.
 * 5. Verifica se a imagem possui tamanho válido:
 *    - mínimo de 3x3, necessário para a janela do Census Transform;
 *    - máximo limitado por MAX_WIDTH e MAX_HEIGHT.
 * 6. Calcula o total de pixels com width * height.
 * 7. Lê cada pixel do arquivo e armazena no vetor image.
 * 8. Fecha o arquivo.
 *
 * @param filename Nome do arquivo PGM P2 que será lido.
 * @param image Vetor onde os pixels da imagem serão armazenados.
 * @param width Ponteiro onde será salva a largura da imagem.
 * @param height Ponteiro onde será salva a altura da imagem.
 *
 * @return 1 se a leitura for concluída com sucesso.
 * @return 0 se houver erro na abertura, validação ou leitura do arquivo.
 *
 * @note Variáveis globais afetadas:
 * - Nenhuma diretamente.
 * - A função altera apenas o vetor passado em image. Se img_in for passado
 *   como argumento, então img_in será preenchido com os pixels da imagem.
 */
int read_pgm_p2(const char *filename, unsigned char image[], int *width, int *height);

/**
 * @brief Escreve uma imagem no formato PGM P2.
 *
 * @details
 * Esta função cria um arquivo PGM P2 a partir de um vetor linear de pixels.
 * Ela é usada para salvar a imagem resultante após a aplicação do
 * Census Transform 3x3.
 *
 * Passo a passo:
 *
 * 1. Abre ou cria o arquivo informado em modo de escrita.
 * 2. Escreve o cabeçalho do formato PGM P2.
 * 3. Percorre a imagem linha por linha.
 * 4. Acessa cada pixel no vetor linear usando:
 *
 *        image[i * width + j]
 *
 * 5. Escreve cada valor no arquivo.
 * 6. Insere uma quebra de linha ao final de cada linha da imagem.
 * 7. Fecha o arquivo.
 *
 * @param filename Nome do arquivo de saída.
 * @param image Vetor contendo os pixels que serão escritos.
 * @param width Largura da imagem.
 * @param height Altura da imagem.
 *
 * @return 1 se a escrita for concluída com sucesso.
 * @return 0 se houver erro ao criar ou escrever o arquivo.
 *
 * @note Variáveis globais afetadas:
 * - Nenhuma diretamente.
 * - A função apenas lê o vetor passado em image. Se img_out for passado
 *   como argumento, seu conteúdo será gravado no arquivo de saída.
 */
int write_pgm_p2(const char *filename, const unsigned char image[], int width, int height);

/**
 * @brief Inicializa todos os pixels de uma imagem com zero.
 *
 * @details
 * Esta função percorre todas as posições de um vetor de imagem e atribui
 * o valor 0 a cada pixel.
 *
 * No contexto deste programa, ela é usada antes da aplicação do
 * Census Transform 3x3 para garantir que a imagem de saída comece zerada.
 *
 * @param image Vetor da imagem que será zerada.
 * @param width Largura da imagem.
 * @param height Altura da imagem.
 *
 * @return Esta função não retorna valor.
 *
 * @note Variáveis globais afetadas:
 * - Nenhuma diretamente.
 */
void clear_image(unsigned char image[], int width, int height);

/**
 * @brief Aplica o Census Transform 3x3 em uma imagem em escala de cinza.
 *
 * @details
 *
 * Para cada pixel interno da imagem, considera-se uma vizinhança 3x3, onde o
 * pixel central é comparado com seus 8 vizinhos.
 *
 * Cada vizinho é analisado individualmente:
 * - se o valor do vizinho for maior ou igual ao valor do pixel central,
 *   é gerado o bit 1;
 * - caso contrário, é gerado o bit 0.
 *
 * Essas comparações são feitas em uma ordem fixa, percorrendo a janela 3x3
 * linha por linha, da esquerda para a direita, ignorando o próprio pixel
 * central. O resultado é uma sequência de 8 bits.
 *
 * Passo a passo:
 *
 * 1. Zera a imagem de saída usando clear_image.
 * 2. Percorre apenas as linhas internas da imagem.
 * 3. Percorre apenas as colunas internas da imagem.
 * 4. Para cada pixel interno, salva o valor do pixel central.
 * 5. Inicializa a variável census com 0.
 * 6. Percorre a janela 3x3 ao redor do pixel central.
 * 7. Ignora a posição do próprio centro.
 * 8. Para cada vizinho, desloca census uma posição para a esquerda.
 * 9. Compara o vizinho com o centro.
 * 10. Se o vizinho for maior ou igual ao centro, adiciona o bit 1.
 * 11. Após os 8 vizinhos, armazena o valor final na imagem de saída.
 *
 * @param input Vetor contendo os pixels da imagem original.
 * @param output Vetor onde serão armazenados os valores resultantes.
 * @param width Largura da imagem.
 * @param height Altura da imagem.
 *
 * @return Esta função não retorna valor.
 *
 * @note Variáveis globais afetadas:
 * - Nenhuma diretamente.
 * - Se img_out for passado como argumento output, img_out será zerado e
 *   preenchido com os valores do Census Transform.
 */
void census_transform_3x3(
    const unsigned char input[],
    unsigned char output[],
    int width,
    int height
);

#endif