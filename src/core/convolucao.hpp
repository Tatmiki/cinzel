#pragma once
// Convolução genérica: g = f * h + offset

#include "bordas_pixel.hpp"
#include "imagem.hpp"
#include "mascaras.hpp"

// Soma dos produtos entre a máscara e a vizinhança de cada pixel, em ponto flutuante.
//   rotacionar = true  -> CONVOLUÇÃO verdadeira: g(x,y) = Σ h(s,t) · f(x - t, y - s)
//   rotacionar = false -> CORRELAÇÃO:            g(x,y) = Σ h(s,t) · f(x + t, y + s)
// (s, t) = deslocamento da célula em relação ao centro da máscara.
// O resultado NÃO é limitado a [0, 255]: pode ter negativos e valores > 255.
ImagemF convoluir(const ImagemF& f, const Mascara& h, Borda borda, bool rotacionar);

// Versão completa para a interface: g = Faixa( f * h + offset ).
Imagem convolucaoGenerica(const Imagem& f, const Mascara& h, float offset,
                          Borda borda, Faixa faixa, bool rotacionar = true);
