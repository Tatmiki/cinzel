#pragma once
// Filtragem espacial: média e mediana com vizinhança n x n.

#include "bordas_pixel.hpp"
#include "imagem.hpp"

// Média: g(x,y) = (1/n²) Σ f~(i,j) na vizinhança. Linear: é uma convolução com máscara 1/n².
Imagem filtroMedia(const Imagem& img, int n, Borda borda);

// Mediana: valor central da vizinhança ordenada. NÃO linear (não é convolução).
Imagem filtroMediana(const Imagem& img, int n, Borda borda);
