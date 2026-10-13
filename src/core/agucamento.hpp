#pragma once
// Aguçamento: Laplaciano e high boost.

#include "bordas_pixel.hpp"
#include "imagem.hpp"

// g = f + c·∇²f, com c = -1 se o centro da máscara do Laplaciano for negativo (é o caso
// das máscaras de 4 e 8 vizinhos usadas aqui) e c = +1 se for positivo.
// extras: "Laplaciano (normalizado)", "Laplaciano (absoluto)", "Agucada"
Resultado agucarLaplaciano(const Imagem& img, int vizinhos, Borda borda);

// g = f + k·(f - f̄), com f̄ = média n x n.  k = 1: unsharp masking;  k > 1: high boost.
// extras: "Suavizada", "Mascara de nitidez (f - media)", "Resultado"
Resultado highBoost(const Imagem& img, float k, int n, Borda borda);
