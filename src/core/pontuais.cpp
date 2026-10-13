#include "pontuais.hpp"

#include <algorithm>
#include <cmath>

// Arredonda e limita a [0, 255]
static uint8_t paraByte(double v) {
    return static_cast<uint8_t>(std::clamp(std::lround(v), 0L, 255L));
}

Imagem aplicarLUT(const Imagem& img, const LUT& lut) {
    Imagem saida(img.largura, img.altura);
    for (size_t i = 0; i < img.px.size(); ++i)
        saida.px[i] = lut[img.px[i]];
    return saida;
}

Imagem negativo(const Imagem& img) {
    LUT lut;
    for (int r = 0; r < 256; ++r) lut[r] = static_cast<uint8_t>(255 - r);
    return aplicarLUT(img, lut);
}

Imagem limiarizar(const Imagem& img, int T) {
    LUT lut;
    for (int r = 0; r < 256; ++r) lut[r] = (r >= T) ? 255 : 0;
    return aplicarLUT(img, lut);
}

Imagem alargarContraste(const Imagem& img, int r1, int s1, int r2, int s2) {
    if (r1 > r2) { std::swap(r1, r2); std::swap(s1, s2); }
    LUT lut;
    for (int r = 0; r < 256; ++r) {
        double s;
        if (r < r1) {
            s = static_cast<double>(s1) / r1 * r;                               // 1ª reta
        } else if (r <= r2) {
            s = (r2 == r1) ? s1 : s1 + static_cast<double>(s2 - s1) / (r2 - r1) * (r - r1);  // 2ª reta
        } else {
            s = s2 + static_cast<double>(255 - s2) / (255 - r2) * (r - r2);     // 3ª reta
        }
        // (r < r1 nunca acontece com r1 = 0, e r > r2 nunca acontece com r2 = 255:
        //  por isso as divisões acima não dividem por zero)
        lut[r] = paraByte(s);
    }
    return aplicarLUT(img, lut);
}

Imagem potencia(const Imagem& img, float c, float gamma) {
    LUT lut;
    for (int r = 0; r < 256; ++r) {
        double rn = r / 255.0;                       // normaliza para [0, 1]
        lut[r] = paraByte(255.0 * c * std::pow(rn, gamma));
    }
    return aplicarLUT(img, lut);
}

Imagem logaritmo(const Imagem& img, float c) {
    double k = (c > 0) ? c : 255.0 / std::log(256.0);
    LUT lut;
    for (int r = 0; r < 256; ++r)
        lut[r] = paraByte(k * std::log(1.0 + r));
    return aplicarLUT(img, lut);
}
