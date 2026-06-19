import os
import glob
import struct
import time
import subprocess
import numpy as np
import serial

PORT = "/dev/tty.usbmodem21203"
# PORT = "/dev/tty.usbmodem11103"
BAUD = 38400

PASTA_ALVO = "cmake-build-debug"
BINARIO_C = "./SEMB"

TILE_UTIL = 16
BORDER = 1
TILE_TOTAL = TILE_UTIL + 2 * BORDER

PREFIXO_PY = "census_output_python_"
PREFIXO_C = "census_output_c_"
PREFIXO_CPLACA = "census_output_cplaca_"


# ============================================================
# Funções auxiliares para ficheiros PGM P2
# ============================================================

def ler_pgm_p2(caminho):
    with open(caminho, "r") as f:
        tokens = []

        for linha in f:
            linha = linha.strip()

            if not linha or linha.startswith("#"):
                continue

            tokens.extend(linha.split())

    if tokens[0] != "P2":
        raise ValueError(f"{caminho} não está no formato P2.")

    largura = int(tokens[1])
    altura = int(tokens[2])

    pixels = np.array([int(x) for x in tokens[4:]], dtype=np.uint8)
    return pixels.reshape((altura, largura))

def salvar_pgm_p2(caminho, imagem):
    altura, largura = imagem.shape

    with open(caminho, "w") as f:
        f.write(f"P2\n{largura} {altura}\n255\n")

        for linha in imagem:
            f.write(" ".join(str(int(pixel)) for pixel in linha) + "\n")

def buscar_imagens_originais():
    arquivos = glob.glob(os.path.join(PASTA_ALVO, "*.pgm"))

    return sorted([
        caminho
        for caminho in arquivos
        if not os.path.basename(caminho).startswith("census_output")
    ])


# ============================================================
# Processamento C desktop
# ============================================================

def rodar_codigo_c_desktop():
    print("[C desktop] Executando census_transform...")

    resultado = subprocess.run(
        [BINARIO_C],
        cwd=PASTA_ALVO,
        capture_output=True,
        text=True
    )

    if resultado.stdout:
        print(resultado.stdout)

    if resultado.returncode != 0:
        if resultado.stderr:
            print(resultado.stderr)

        raise RuntimeError("Erro ao executar o código C desktop.")


# ============================================================
# Processamento Python (Otimizado com Loop Unrolling)
# ============================================================

def transformada_census_python(imagem):
    altura, largura = imagem.shape
    saida = np.zeros((altura, largura), dtype=np.uint8)

    for row in range(1, altura - 1):
        for col in range(1, largura - 1):
            centro = imagem[row, col]
            census = 0

            # Lógica desenrolada espelhando o firmware da placa e C++
            census |= (imagem[row - 1, col - 1] >= centro) << 7
            census |= (imagem[row - 1, col]     >= centro) << 6
            census |= (imagem[row - 1, col + 1] >= centro) << 5
            census |= (imagem[row, col - 1]     >= centro) << 4
            census |= (imagem[row, col + 1]     >= centro) << 3
            census |= (imagem[row + 1, col - 1] >= centro) << 2
            census |= (imagem[row + 1, col]     >= centro) << 1
            census |= (imagem[row + 1, col + 1] >= centro)

            saida[row, col] = census

    return saida

def gerar_resultados_python(imagens):
    print("\n[Python] Gerando referências...")

    for caminho_original in imagens:
        nome_original = os.path.basename(caminho_original)

        imagem_original = ler_pgm_p2(caminho_original)
        resultado_python = transformada_census_python(imagem_original)

        caminho_python = os.path.join(
            PASTA_ALVO,
            PREFIXO_PY + nome_original
        )

        salvar_pgm_p2(caminho_python, resultado_python)

        print(
            f"[Python] Sucesso: {nome_original} -> "
            f"{os.path.basename(caminho_python)}"
        )


# ============================================================
# Processamento da placa
# ============================================================

def criar_tile_com_borda(imagem, x, y):
    altura, largura = imagem.shape
    tile = np.zeros((TILE_TOTAL, TILE_TOTAL), dtype=np.uint8)

    for r in range(TILE_TOTAL):
        src_y = max(0, min(altura - 1, y - BORDER + r))

        for c in range(TILE_TOTAL):
            src_x = max(0, min(largura - 1, x - BORDER + c))
            tile[r, c] = imagem[src_y, src_x]

    return tile

def processar_imagem_na_placa(ser, caminho_original):
    nome_original = os.path.basename(caminho_original)

    imagem = ler_pgm_p2(caminho_original)
    altura, largura = imagem.shape

    reconstruida = np.zeros_like(imagem)

    for y in range(0, altura, TILE_UTIL):
        for x in range(0, largura, TILE_UTIL):
            util_w = min(TILE_UTIL, largura - x)
            util_h = min(TILE_UTIL, altura - y)

            tile = criar_tile_com_borda(imagem, x, y)

            header = struct.pack(
                "<HHHH",
                TILE_TOTAL,
                TILE_TOTAL,
                util_w,
                util_h
            )

            ser.write(b"S")
            ser.write(header)
            ser.write(tile.tobytes())

            rx_sync = ser.read(1)
            rx_header = ser.read(8)
            rx_data = ser.read(TILE_UTIL * TILE_UTIL)

            if rx_sync != b"S":
                raise RuntimeError(
                    f"Erro de sincronismo com a placa: {rx_sync}"
                )

            if len(rx_header) != 8:
                raise RuntimeError(
                    "Header incompleto recebido da placa."
                )

            if len(rx_data) != TILE_UTIL * TILE_UTIL:
                raise RuntimeError(
                    "Dados incompletos recebidos da placa."
                )

            tile_saida = np.frombuffer(
                rx_data,
                dtype=np.uint8
            ).reshape((TILE_UTIL, TILE_UTIL))

            reconstruida[y:y + util_h, x:x + util_w] = (
                tile_saida[:util_h, :util_w]
            )

    # Normaliza as bordas para ficar igual ao Python e ao C desktop.
    reconstruida[0, :] = 0
    reconstruida[-1, :] = 0
    reconstruida[:, 0] = 0
    reconstruida[:, -1] = 0

    caminho_saida = os.path.join(
        PASTA_ALVO,
        PREFIXO_CPLACA + nome_original
    )

    salvar_pgm_p2(caminho_saida, reconstruida)

    return caminho_saida

def gerar_resultados_placa(imagens):
    print("\n[C placa] Iniciando processamento na STM32...")

    with serial.Serial(PORT, BAUD, timeout=5) as ser:
        time.sleep(2)

        for caminho_original in imagens:
            nome_original = os.path.basename(caminho_original)

            try:
                inicio = time.perf_counter()

                caminho_saida = processar_imagem_na_placa(
                    ser,
                    caminho_original
                )

                tempo = time.perf_counter() - inicio

                print(
                    f"[C placa] Sucesso: {nome_original} -> "
                    f"{os.path.basename(caminho_saida)} "
                    f"({tempo:.2f}s)"
                )

            except Exception as erro:
                print(
                    f"[C placa] ERRO em {nome_original}: {erro}"
                )


# ============================================================
# Comparações
# ============================================================

def comparar_imagens(nome_a, imagem_a, nome_b, imagem_b):
    if imagem_a.shape != imagem_b.shape:
        return False, (
            f"{nome_a} e {nome_b}: dimensões diferentes "
            f"{imagem_a.shape} != {imagem_b.shape}"
        )

    diferencas = imagem_a != imagem_b
    qtd_diferencas = np.count_nonzero(diferencas)

    if qtd_diferencas == 0:
        return True, f"{nome_a} e {nome_b}: iguais"

    total = imagem_a.size
    acuracia = 100 * (total - qtd_diferencas) / total

    y, x = np.argwhere(diferencas)[0]

    return False, (
        f"{nome_a} e {nome_b}: {qtd_diferencas} pixels diferentes "
        f"de {total} ({acuracia:.2f}% iguais). "
        f"Primeira diferença em x={x}, y={y}: "
        f"{nome_a}={imagem_a[y, x]}, {nome_b}={imagem_b[y, x]}"
    )

def comparar_todos_resultados(imagens):
    print("\n[Comparações] Iniciando validação...")

    aprovadas = 0

    for caminho_original in imagens:
        nome_original = os.path.basename(caminho_original)

        print(f"\nImagem: {nome_original}")

        caminho_python = os.path.join(
            PASTA_ALVO,
            PREFIXO_PY + nome_original
        )

        caminho_c = os.path.join(
            PASTA_ALVO,
            PREFIXO_C + nome_original
        )

        caminho_cplaca = os.path.join(
            PASTA_ALVO,
            PREFIXO_CPLACA + nome_original
        )

        if not os.path.exists(caminho_python):
            print(
                f"[ERRO] Python não encontrado: "
                f"{os.path.basename(caminho_python)}"
            )
            continue

        if not os.path.exists(caminho_c):
            print(
                f"[ERRO] C desktop não encontrado: "
                f"{os.path.basename(caminho_c)}"
            )
            continue

        if not os.path.exists(caminho_cplaca):
            print(
                f"[ERRO] C placa não encontrado: "
                f"{os.path.basename(caminho_cplaca)}"
            )
            continue

        resultado_python = ler_pgm_p2(caminho_python)
        resultado_c = ler_pgm_p2(caminho_c)
        resultado_cplaca = ler_pgm_p2(caminho_cplaca)

        ok_py_c, msg_py_c = comparar_imagens(
            "Python",
            resultado_python,
            "C desktop",
            resultado_c
        )

        ok_py_cplaca, msg_py_cplaca = comparar_imagens(
            "Python",
            resultado_python,
            "C placa",
            resultado_cplaca
        )

        ok_c_cplaca, msg_c_cplaca = comparar_imagens(
            "C desktop",
            resultado_c,
            "C placa",
            resultado_cplaca
        )

        print(
            "[Python x C desktop]",
            "OK" if ok_py_c else "ERRO",
            "-",
            msg_py_c
        )

        print(
            "[Python x C placa]",
            "OK" if ok_py_cplaca else "ERRO",
            "-",
            msg_py_cplaca
        )

        print(
            "[C desktop x C placa]",
            "OK" if ok_c_cplaca else "ERRO",
            "-",
            msg_c_cplaca
        )

        if ok_py_c and ok_py_cplaca and ok_c_cplaca:
            aprovadas += 1

    total = len(imagens)

    print("\nResumo final")
    print(f"Imagens testadas: {total}")
    print(f"Aprovadas: {aprovadas}")
    print(f"Reprovadas: {total - aprovadas}")

    if aprovadas == total:
        print(
            "Resultado final: Python, C desktop e C placa "
            "produziram resultados iguais."
        )
    else:
        print(
            "Resultado final: existem diferenças, ficheiros em falta "
            "ou erro na comunicação com a placa."
        )

def injetar_erro_controlado(imagens):
    """
    Altera propositalmente um pixel do primeiro resultado da placa
    para demonstrar que o sistema de comparação detecta erros.
    """

    if not imagens:
        return

    nome_original = os.path.basename(imagens[0])

    caminho_cplaca = os.path.join(
        PASTA_ALVO,
        PREFIXO_CPLACA + nome_original
    )

    if not os.path.exists(caminho_cplaca):
        print("\n[Erro controlado] Ficheiro da placa não encontrado.")
        return

    imagem = ler_pgm_p2(caminho_cplaca)

    y = 10
    x = 10

    valor_original = int(imagem[y, x])

    imagem[y, x] = (valor_original + 1) % 256

    salvar_pgm_p2(caminho_cplaca, imagem)

    print(
        f"[Erro controlado] Pixel ({x}, {y}) alterado em "
        f"{os.path.basename(caminho_cplaca)}: "
        f"{valor_original} -> {int(imagem[y, x])}"
    )


# ============================================================
# Execução principal
# ============================================================

def main():
    rodar_codigo_c_desktop()

    imagens = buscar_imagens_originais()

    if not imagens:
        print(
            f"Nenhuma imagem original encontrada em "
            f"'{PASTA_ALVO}'."
        )
        return

    print(f"\nImagens encontradas: {len(imagens)}")
    print(f"Pasta alvo: {PASTA_ALVO}")
    print(f"Porta serial: {PORT}")
    print(f"Baud rate: {BAUD}")

    gerar_resultados_placa(imagens)
    gerar_resultados_python(imagens)

    injetar_erro_controlado(imagens)

    comparar_todos_resultados(imagens)


if __name__ == "__main__":
    main()