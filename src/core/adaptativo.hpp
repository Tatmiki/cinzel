#pragma once
// Controle de contraste adaptativo (realce local).

#include "bordas_pixel.hpp"
#include "imagem.hpp"

// Para cada pixel, com média m e desvio padrão σ da vizinhança n x n:
//     g(x,y) = m + (c / σ) · (f(x,y) - m)
// σ pequeno (pouco contraste local) -> ganho c/σ grande -> realça detalhes.
// σ = 0 (região uniforme) -> g = f.
// extras: "Media local", "Desvio padrao local", "Resultado"
Resultado contrasteAdaptativo(const Imagem& img, float c, int n, Borda borda);
