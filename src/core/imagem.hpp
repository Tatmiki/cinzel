#pragma once
// Tipos básicos do sistema: imagem 8 bits, imagem em ponto flutuante e conversões.

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

// Imagem final em escala de cinza, 8 bits/pixel: o que se lê, exibe e salva.
// O pixel (x, y) fica em px[y * largura + x] (linha por linha, "row-major").
struct Imagem {
    int largura = 0, altura = 0;
    std::vector<uint8_t> px;

    Imagem() = default;
    Imagem(int l, int a, uint8_t valor = 0) : largura(l), altura(a), px(l * a, valor) {}
    Imagem(int l, int a, std::vector<uint8_t> v) : largura(l), altura(a), px(std::move(v)) {}

    uint8_t& at(int x, int y)       { return px[y * largura + x]; }
    uint8_t  at(int x, int y) const { return px[y * largura + x]; }
    bool vazia() const { return px.empty(); }
};

// Imagem intermediária em ponto flutuante.
// Guarda valores negativos e maiores que 255 (Gx, Gy, Laplaciano, convoluções).
struct ImagemF {
    int largura = 0, altura = 0;
    std::vector<float> px;

    ImagemF() = default;
    ImagemF(int l, int a, float valor = 0.0f) : largura(l), altura(a), px(l * a, valor) {}

    float& at(int x, int y)       { return px[y * largura + x]; }
    float  at(int x, int y) const { return px[y * largura + x]; }
};

// Como converter valores fora de [0, 255] para 8 bits.
enum class Faixa {
    Saturar,     // v' = clamp(v, 0, 255)
    Normalizar,  // v' = (v - min) * 255 / (max - min)
    Absoluto     // v' = clamp(|v|, 0, 255)
};

// Resultado de uma operação: imagem principal + imagens intermediárias com nome
// (ex.: {"Gx", ...}, {"Gy", ...}). A interface mostra tudo sem conhecer o algoritmo.
struct Resultado {
    Imagem principal;
    std::vector<std::pair<std::string, Imagem>> extras;
};

ImagemF paraFloat(const Imagem& img);
Imagem  paraUint8(const ImagemF& img, Faixa modo);

bool mesmoTamanho(const Imagem& a, const Imagem& b);

// Leitura (PNG, JPG, BMP, PGM, TIFF...) sempre convertendo para cinza 8 bits.
bool carregar(const std::string& caminho, Imagem& img);
// Escrita em PNG.
bool salvar(const std::string& caminho, const Imagem& img);

// Leitor de TIFF próprio (o stb_image não lê TIFF). Implementado em tiff.cpp.
bool carregarTIFF(const std::string& caminho, Imagem& img);
