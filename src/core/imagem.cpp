// stb: biblioteca externa de leitura/escrita. Os avisos dela não são do nosso código.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Warray-bounds"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#pragma GCC diagnostic pop

#include "imagem.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>

ImagemF paraFloat(const Imagem& img) {
    ImagemF f(img.largura, img.altura);
    for (size_t i = 0; i < img.px.size(); ++i)
        f.px[i] = img.px[i];
    return f;
}

Imagem paraUint8(const ImagemF& img, Faixa modo) {
    Imagem saida(img.largura, img.altura);
    if (img.px.empty()) return saida;

    float vmin = 0.0f, vmax = 0.0f;
    if (modo == Faixa::Normalizar) {
        auto [itMin, itMax] = std::minmax_element(img.px.begin(), img.px.end());
        vmin = *itMin;
        vmax = *itMax;
    }

    for (size_t i = 0; i < img.px.size(); ++i) {
        float v = img.px[i];
        switch (modo) {
            case Faixa::Saturar:
                break;
            case Faixa::Absoluto:
                v = std::fabs(v);
                break;
            case Faixa::Normalizar:
                // Imagem constante: max == min daria divisão por zero.
                v = (vmax > vmin) ? (v - vmin) * 255.0f / (vmax - vmin) : 0.0f;
                break;
        }
        saida.px[i] = static_cast<uint8_t>(std::clamp(std::lround(v), 0L, 255L));
    }
    return saida;
}

bool mesmoTamanho(const Imagem& a, const Imagem& b) {
    return a.largura == b.largura && a.altura == b.altura;
}

static std::string extensaoMinuscula(const std::string& caminho) {
    auto ponto = caminho.find_last_of('.');
    if (ponto == std::string::npos) return "";
    std::string ext = caminho.substr(ponto + 1);
    for (char& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return ext;
}

bool carregar(const std::string& caminho, Imagem& img) {
    std::string ext = extensaoMinuscula(caminho);
    if (ext == "tif" || ext == "tiff")
        return carregarTIFF(caminho, img);

    int w, h, canais;
    // O último argumento (1) pede ao stb um único canal: cinza.
    uint8_t* dados = stbi_load(caminho.c_str(), &w, &h, &canais, 1);
    if (!dados) return false;
    img = Imagem(w, h, std::vector<uint8_t>(dados, dados + w * h));
    stbi_image_free(dados);
    return true;
}

bool salvar(const std::string& caminho, const Imagem& img) {
    if (img.vazia()) return false;
    return stbi_write_png(caminho.c_str(), img.largura, img.altura, 1,
                          img.px.data(), img.largura) != 0;
}
