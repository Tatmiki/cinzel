#pragma once
// Máscaras (kernels) usadas no sistema: operadores de borda e testes obrigatórios.

#include <string>
#include <vector>

// Máscara com n linhas e m colunas. h(i, j) = v[i * colunas + j].
struct Mascara {
    int linhas = 0, colunas = 0;
    std::vector<float> v;

    Mascara() = default;
    Mascara(int n, int m, std::vector<float> valores) : linhas(n), colunas(m), v(std::move(valores)) {}

    float  at(int i, int j) const { return v[i * colunas + j]; }
    float& at(int i, int j)       { return v[i * colunas + j]; }

    float soma() const {
        float s = 0.0f;
        for (float x : v) s += x;
        return s;
    }
    // Centro (âncora): ((n-1)/2, (m-1)/2). Para tamanhos ímpares é o centro exato;
    // para pares (ex.: Roberts 2x2) fica no canto superior esquerdo do bloco central.
    int centroLinha() const { return (linhas - 1) / 2; }
    int centroColuna() const { return (colunas - 1) / 2; }
};

namespace mascaras {

// ---- Suavização ----
inline Mascara media(int n) {
    return Mascara(n, n, std::vector<float>(n * n, 1.0f / (n * n)));
}

// ---- Roberts (2x2, diagonais) ----
inline Mascara robertsX() { return Mascara(2, 2, {1, 0,
                                                  0, -1}); }
inline Mascara robertsY() { return Mascara(2, 2, {0, 1,
                                                  -1, 0}); }

// ---- Sobel (3x3) ----
inline Mascara sobelX() { return Mascara(3, 3, {-1, 0, 1,
                                                -2, 0, 2,
                                                -1, 0, 1}); }
inline Mascara sobelY() { return Mascara(3, 3, {-1, -2, -1,
                                                 0,  0,  0,
                                                 1,  2,  1}); }

// ---- Laplaciano ----
inline Mascara laplaciano4() { return Mascara(3, 3, {0,  1, 0,
                                                     1, -4, 1,
                                                     0,  1, 0}); }
inline Mascara laplaciano8() { return Mascara(3, 3, {1,  1, 1,
                                                     1, -8, 1,
                                                     1,  1, 1}); }

// ---- Kirsch: 8 máscaras obtidas girando o anel externo 45° por vez ----
// Posições do anel no sentido horário:  0 1 2
//                                       7 . 3
//                                       6 5 4
inline Mascara kirsch(int k) {
    const float base[8] = {5, 5, 5, -3, -3, -3, -3, -3};   // K0: os 5 em cima (Norte)
    const int linha[8]  = {0, 0, 0, 1, 2, 2, 2, 1};
    const int coluna[8] = {0, 1, 2, 2, 2, 1, 0, 0};
    Mascara h(3, 3, std::vector<float>(9, 0.0f));
    for (int p = 0; p < 8; ++p) {
        // Girar k posições no sentido horário: a posição p recebe o valor que estava em p-k
        h.at(linha[p], coluna[p]) = base[((p - k) % 8 + 8) % 8];
    }
    return h;
}
inline const char* nomeKirsch(int k) {
    static const char* nomes[8] = {"N", "NE", "L", "SE", "S", "SO", "O", "NO"};
    return nomes[k];
}

// ---- Testes obrigatórios da especificação (seção 2) ----
// 2.1: aguçamento com parâmetros c e d (soma dos coeficientes = d)
inline Mascara agucamentoCD(int c, int d) {
    float a = -static_cast<float>(c), centro = 8.0f * c + d;
    return Mascara(3, 3, {a, a, a,
                          a, centro, a,
                          a, a, a});
}
// 2.2: relevo
inline Mascara relevoH1() { return Mascara(3, 3, {0, 0, 0,
                                                  0, 1, 0,
                                                  0, 0, -1}); }
inline Mascara relevoH2() { return Mascara(3, 3, {0, 0, 2,
                                                  0, -1, 0,
                                                 -1, 0, 0}); }
// 2.3: detecção de bordas
inline Mascara bordaH3() { return Mascara(3, 3, {-1, -1, 0,
                                                  1,  0, 1,
                                                  0,  1, 1}); }
inline Mascara bordaH4() { return Mascara(3, 3, { 0, -1,  0,
                                                 -1, -4, -1,
                                                  0, -1,  0}); }

}  // namespace mascaras
