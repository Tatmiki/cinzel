#include "agucamento.hpp"

#include "convolucao.hpp"
#include "mascaras.hpp"

Resultado agucarLaplaciano(const Imagem& img, int vizinhos, Borda borda) {
    Mascara h = (vizinhos == 8) ? mascaras::laplaciano8() : mascaras::laplaciano4();
    ImagemF f = paraFloat(img);
    ImagemF lap = convoluir(f, h, borda, false);

    // Centro negativo: o Laplaciano é negativo onde o pixel é mais claro que os vizinhos,
    // então SUBTRAIR (c = -1) deixa o ponto claro ainda mais claro.
    float c = (h.at(h.centroLinha(), h.centroColuna()) < 0) ? -1.0f : 1.0f;

    ImagemF g(f.largura, f.altura);
    for (size_t i = 0; i < g.px.size(); ++i)
        g.px[i] = f.px[i] + c * lap.px[i];

    Resultado r;
    r.principal = paraUint8(g, Faixa::Saturar);
    r.extras = {{"Laplaciano (normalizado)", paraUint8(lap, Faixa::Normalizar)},
                {"Laplaciano (absoluto)", paraUint8(lap, Faixa::Absoluto)},
                {"Agucada", r.principal}};
    return r;
}

Resultado highBoost(const Imagem& img, float k, int n, Borda borda) {
    ImagemF f = paraFloat(img);
    ImagemF suave = convoluir(f, mascaras::media(n), borda, true);

    ImagemF detalhe(f.largura, f.altura), g(f.largura, f.altura);
    for (size_t i = 0; i < g.px.size(); ++i) {
        detalhe.px[i] = f.px[i] - suave.px[i];      // máscara de nitidez: só os detalhes
        g.px[i] = f.px[i] + k * detalhe.px[i];
    }

    Resultado r;
    r.principal = paraUint8(g, Faixa::Saturar);
    r.extras = {{"Suavizada", paraUint8(suave, Faixa::Saturar)},
                {"Mascara de nitidez (f - media)", paraUint8(detalhe, Faixa::Normalizar)},
                {"Resultado", r.principal}};
    return r;
}
