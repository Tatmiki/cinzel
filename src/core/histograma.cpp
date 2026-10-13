#include "histograma.hpp"

#include <algorithm>
#include <cmath>

#include "pontuais.hpp"

Histograma histograma(const Imagem& img) {
    Histograma h{};
    for (uint8_t v : img.px) h[v]++;
    return h;
}

Imagem expandirHistograma(const Imagem& img) {
    if (img.vazia()) return Imagem();
    auto [itMin, itMax] = std::minmax_element(img.px.begin(), img.px.end());
    int rmin = *itMin, rmax = *itMax;
    if (rmax == rmin) return img;  // imagem constante: nada a expandir (evita divisão por zero)

    LUT lut{};
    for (int r = 0; r < 256; ++r) {
        double s = static_cast<double>(r - rmin) * 255.0 / (rmax - rmin);
        lut[r] = static_cast<uint8_t>(std::clamp(std::lround(s), 0L, 255L));
    }
    return aplicarLUT(img, lut);
}

Imagem equalizarHistograma(const Imagem& img) {
    if (img.vazia()) return Imagem();
    Histograma h = histograma(img);
    const double N = static_cast<double>(img.px.size());

    LUT lut{};
    double acumulado = 0.0;  // CDF(k) = Σ p(j), j = 0..k
    for (int k = 0; k < 256; ++k) {
        acumulado += h[k] / N;
        lut[k] = static_cast<uint8_t>(std::clamp(std::lround(255.0 * acumulado), 0L, 255L));
    }
    return aplicarLUT(img, lut);
}

Imagem desenharHistograma(const Imagem& img, int largura, int altura) {
    Histograma h = histograma(img);
    int maior = *std::max_element(h.begin(), h.end());
    Imagem saida(largura, altura, 255);
    if (maior == 0) return saida;

    for (int x = 0; x < largura; ++x) {
        int k = x * 256 / largura;  // nível de cinza representado por esta coluna
        int barra = static_cast<int>(std::lround(static_cast<double>(h[k]) / maior * (altura - 1)));
        for (int y = altura - barra; y < altura; ++y) saida.at(x, y) = 0;
    }
    return saida;
}
