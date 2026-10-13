#include "adaptativo.hpp"

#include <algorithm>
#include <cmath>

Resultado contrasteAdaptativo(const Imagem& img, float c, int n, Borda borda) {
    ImagemF f = paraFloat(img);
    ImagemF media(img.largura, img.altura), desvio(img.largura, img.altura), g(img.largura, img.altura);
    const int centro = (n - 1) / 2;
    const float N = static_cast<float>(n * n);

    for (int y = 0; y < img.altura; ++y) {
        for (int x = 0; x < img.largura; ++x) {
            // Média e desvio padrão da vizinhança: σ² = E[f²] - m²
            float soma = 0.0f, somaQuadrados = 0.0f;
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j) {
                    float v = pixelComBorda(f, x + (j - centro), y + (i - centro), borda);
                    soma += v;
                    somaQuadrados += v * v;
                }
            float m = soma / N;
            float variancia = somaQuadrados / N - m * m;
            float sigma = std::sqrt(std::max(variancia, 0.0f));  // max: evita raiz de -0.0001 por arredondamento

            float valor = f.at(x, y);
            g.at(x, y) = (sigma > 1e-6f) ? m + (c / sigma) * (valor - m) : valor;
            media.at(x, y) = m;
            desvio.at(x, y) = sigma;
        }
    }

    Resultado r;
    r.principal = paraUint8(g, Faixa::Saturar);
    r.extras = {{"Media local", paraUint8(media, Faixa::Saturar)},
                {"Desvio padrao local", paraUint8(desvio, Faixa::Normalizar)},
                {"Resultado", r.principal}};
    return r;
}
