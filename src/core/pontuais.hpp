#pragma once
// Transformações de intensidade (pontuais): s = T(r).
// O novo valor depende SÓ do valor do próprio pixel, por isso todas usam uma LUT de 256 posições.

#include <array>
#include <cstdint>

#include "imagem.hpp"

using LUT = std::array<uint8_t, 256>;

Imagem aplicarLUT(const Imagem& img, const LUT& lut);

// s = 255 - r
Imagem negativo(const Imagem& img);

// s = 255 se r >= T, senão 0
Imagem limiarizar(const Imagem& img, int T);

// Linear por partes passando por (0,0), (r1,s1), (r2,s2), (255,255). Exige r1 <= r2.
Imagem alargarContraste(const Imagem& img, int r1, int s1, int r2, int s2);

// s = 255 · c · (r/255)^gamma
Imagem potencia(const Imagem& img, float c, float gamma);

// s = c · ln(1 + r). Se c <= 0, usa c = 255 / ln(256) (assim 255 -> 255).
Imagem logaritmo(const Imagem& img, float c);
