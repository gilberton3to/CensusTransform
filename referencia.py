import numpy as np

def ler_pgm_p2(caminho):
    with open(caminho, 'r') as f:
        # Lê tudo ignorando comentários
        tokens = [palavra for linha in f if not linha.startswith('#') for palavra in linha.split()]

    largura, altura = int(tokens[1]), int(tokens[2])
    # Pega os pixels e transforma em uma matriz
    pixels = np.array([int(x) for x in tokens[4:]], dtype=np.uint8)
    return pixels.reshape((altura, largura))

def transformada_census(img):
    altura, largura = img.shape
    saida = np.zeros((altura, largura), dtype=np.uint8)

    for row in range(1, altura - 1):
        for col in range(1, largura - 1):
            centro = img[row, col]
            census = 0
            for i in range(row - 1, row + 2):
                for j in range(col - 1, col + 2):
                    if i == row and j == col:
                        continue
                    census <<= 1
                    if img[i, j] >= centro:
                        census |= 1
            saida[row, col] = census
    return saida

# Executando o teste
print("Rodando o Modelo de Ouro em Python...")
img_entrada = ler_pgm_p2("cmake-build-debug/input.pgm")
img_saida = transformada_census(img_entrada)

# Salvando a saída de referência
with open("saida_python.pgm", 'w') as f:
    f.write(f"P2\n{img_entrada.shape[1]} {img_entrada.shape[0]}\n255\n")
    for row in img_saida:
        f.write(" ".join(str(p) for p in row) + "\n")
print("Arquivo saida_python.pgm gerado com sucesso!")