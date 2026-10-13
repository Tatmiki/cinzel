#include "filtros.hpp"

#include <algorithm>
#include <vector>

#include "convolucao.hpp"

Imagem filtroMedia(const Imagem& img, int n, Borda borda) {
    // A máscara da média é simétrica: convolução e correlação dão o mesmo resultado.
    ImagemF g = convoluir(paraFloat(img), mascaras::media(n), borda, true);
    return paraUint8(g, Faixa::Saturar);  // média de valores em [0,255] fica em [0,255]
}

Imagem filtroMediana(const Imagem& img, int n, Borda borda) {
    ImagemF f = paraFloat(img);
    Imagem g(img.largura, img.altura);
    const int c = (n - 1) / 2;          // mesmo centro usado nas máscaras
    std::vector<float> vizinhos;
    vizinhos.reserve(n * n);

    for (int y = 0; y < img.altura; ++y) {
        for (int x = 0; x < img.largura; ++x) {
            vizinhos.clear();
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    vizinhos.push_back(pixelComBorda(f, x + (j - c), y + (i - c), borda));

            // nth_element coloca na posição do meio o valor que estaria lá se o vetor
            // fosse ordenado (mais rápido que ordenar tudo, mesmo resultado).
            auto meio = vizinhos.begin() + vizinhos.size() / 2;
            std::nth_element(vizinhos.begin(), meio, vizinhos.end());
            g.at(x, y) = static_cast<uint8_t>(*meio);
        }
    }
    return g;
}
