#include "deteccao.hpp"

#include <algorithm>
#include <cmath>
#include <string>

#include "convolucao.hpp"

static const bool CORRELACAO = false;  // valor do parâmetro "rotacionar" de convoluir()

ImagemF magnitudeGradiente(const ImagemF& gx, const ImagemF& gy, Magnitude tipo) {
    ImagemF mag(gx.largura, gx.altura);
    for (size_t i = 0; i < mag.px.size(); ++i) {
        float a = gx.px[i], b = gy.px[i];
        mag.px[i] = (tipo == Magnitude::Euclidiana) ? std::sqrt(a * a + b * b)
                                                    : std::fabs(a) + std::fabs(b);
    }
    return mag;
}

// Roberts e Sobel só diferem nas máscaras
static Resultado gradiente(const Imagem& img, const Mascara& hx, const Mascara& hy,
                           Borda borda, Magnitude tipo, Faixa faixa) {
    ImagemF f = paraFloat(img);
    ImagemF gx = convoluir(f, hx, borda, CORRELACAO);
    ImagemF gy = convoluir(f, hy, borda, CORRELACAO);
    ImagemF mag = magnitudeGradiente(gx, gy, tipo);

    Resultado r;
    r.principal = paraUint8(mag, faixa);
    r.extras = {{"Gx", paraUint8(gx, Faixa::Normalizar)},
                {"Gy", paraUint8(gy, Faixa::Normalizar)},
                {"Magnitude", r.principal}};
    return r;
}

Resultado roberts(const Imagem& img, Borda borda, Magnitude tipo, Faixa faixa) {
    return gradiente(img, mascaras::robertsX(), mascaras::robertsY(), borda, tipo, faixa);
}

Resultado sobel(const Imagem& img, Borda borda, Magnitude tipo, Faixa faixa) {
    return gradiente(img, mascaras::sobelX(), mascaras::sobelY(), borda, tipo, faixa);
}

Resultado kirsch(const Imagem& img, Borda borda, Faixa faixa) {
    ImagemF f = paraFloat(img);
    ImagemF maximo(img.largura, img.altura, -1e30f);
    Resultado r;

    for (int k = 0; k < 8; ++k) {
        ImagemF resposta = convoluir(f, mascaras::kirsch(k), borda, CORRELACAO);
        for (size_t i = 0; i < maximo.px.size(); ++i)
            maximo.px[i] = std::max(maximo.px[i], resposta.px[i]);
        std::string nome = "K" + std::to_string(k) + " (" + mascaras::nomeKirsch(k) + ")";
        r.extras.push_back({nome, paraUint8(resposta, faixa)});
    }
    r.principal = paraUint8(maximo, faixa);
    r.extras.push_back({"Maximo", r.principal});
    return r;
}

Resultado laplaciano(const Imagem& img, int vizinhos, Borda borda, Faixa faixa) {
    Mascara h = (vizinhos == 8) ? mascaras::laplaciano8() : mascaras::laplaciano4();
    ImagemF lap = convoluir(paraFloat(img), h, borda, CORRELACAO);

    Resultado r;
    r.principal = paraUint8(lap, faixa);
    r.extras = {{"Laplaciano (normalizado)", paraUint8(lap, Faixa::Normalizar)},
                {"Laplaciano", r.principal}};
    return r;
}
