#include "convolucao.hpp"

ImagemF convoluir(const ImagemF& f, const Mascara& h, Borda borda, bool rotacionar) {
    ImagemF g(f.largura, f.altura);
    const int cy = h.centroLinha();
    const int cx = h.centroColuna();

    for (int y = 0; y < f.altura; ++y) {
        for (int x = 0; x < f.largura; ++x) {
            float soma = 0.0f;
            for (int i = 0; i < h.linhas; ++i) {
                for (int j = 0; j < h.colunas; ++j) {
                    int s = i - cy;  // deslocamento vertical em relação ao centro
                    int t = j - cx;  // deslocamento horizontal em relação ao centro
                    float valor = rotacionar ? pixelComBorda(f, x - t, y - s, borda)
                                             : pixelComBorda(f, x + t, y + s, borda);
                    soma += h.at(i, j) * valor;
                }
            }
            g.at(x, y) = soma;
        }
    }
    return g;
}

Imagem convolucaoGenerica(const Imagem& f, const Mascara& h, float offset,
                          Borda borda, Faixa faixa, bool rotacionar) {
    ImagemF g = convoluir(paraFloat(f), h, borda, rotacionar);
    for (float& v : g.px) v += offset;   // offset ANTES de tratar a faixa
    return paraUint8(g, faixa);
}
