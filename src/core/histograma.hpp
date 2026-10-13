#pragma once
// Histograma: cálculo, expansão e equalização.

#include <array>

#include "imagem.hpp"

using Histograma = std::array<int, 256>;

// h(k) = número de pixels com valor k
Histograma histograma(const Imagem& img);

// Expansão: s = (r - rmin) · 255 / (rmax - rmin). Linear, só "estica" o histograma.
Imagem expandirHistograma(const Imagem& img);

// Equalização: s_k = round(255 · CDF(k)), CDF(k) = Σ_{j<=k} h(j)/N. Não linear.
Imagem equalizarHistograma(const Imagem& img);

// Desenha o histograma como imagem (barras pretas em fundo branco), para o relatório.
Imagem desenharHistograma(const Imagem& img, int largura = 256, int altura = 160);
