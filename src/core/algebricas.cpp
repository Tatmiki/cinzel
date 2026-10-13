#include "algebricas.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

static void exigirMesmoTamanho(const Imagem& a, const Imagem& b) {
    if (!mesmoTamanho(a, b))
        throw std::invalid_argument("As imagens precisam ter o mesmo tamanho");
}

Imagem dissolveUniforme(const Imagem& f1, const Imagem& f2, float t) {
    exigirMesmoTamanho(f1, f2);
    Imagem g(f1.largura, f1.altura);
    for (size_t i = 0; i < g.px.size(); ++i) {
        float v = (1.0f - t) * f1.px[i] + t * f2.px[i];
        g.px[i] = static_cast<uint8_t>(std::clamp(std::lround(v), 0L, 255L));
    }
    return g;
}

Imagem dissolveNaoUniforme(const Imagem& f1, const Imagem& f2, const ImagemF& pesos) {
    exigirMesmoTamanho(f1, f2);
    if (pesos.largura != f1.largura || pesos.altura != f1.altura)
        throw std::invalid_argument("Os pesos precisam ter o tamanho das imagens");
    Imagem g(f1.largura, f1.altura);
    for (size_t i = 0; i < g.px.size(); ++i) {
        float t = pesos.px[i];
        float v = (1.0f - t) * f1.px[i] + t * f2.px[i];
        g.px[i] = static_cast<uint8_t>(std::clamp(std::lround(v), 0L, 255L));
    }
    return g;
}

ImagemF pesosRampaHorizontal(int largura, int altura) {
    ImagemF t(largura, altura);
    for (int y = 0; y < altura; ++y)
        for (int x = 0; x < largura; ++x)
            t.at(x, y) = (largura > 1) ? static_cast<float>(x) / (largura - 1) : 0.0f;
    return t;
}

ImagemF pesosRadial(int largura, int altura) {
    ImagemF t(largura, altura);
    float cx = (largura - 1) / 2.0f, cy = (altura - 1) / 2.0f;
    float dmax = std::sqrt(cx * cx + cy * cy);
    for (int y = 0; y < altura; ++y)
        for (int x = 0; x < largura; ++x) {
            float d = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
            t.at(x, y) = (dmax > 0) ? d / dmax : 0.0f;
        }
    return t;
}

ImagemF pesosDeImagem(const Imagem& mascara) {
    ImagemF t(mascara.largura, mascara.altura);
    for (size_t i = 0; i < t.px.size(); ++i) t.px[i] = mascara.px[i] / 255.0f;
    return t;
}
