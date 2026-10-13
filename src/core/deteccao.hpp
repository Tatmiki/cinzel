#pragma once
// Detecção de bordas: Roberts, Sobel, Kirsch e Laplaciano.
//
// As máscaras são aplicadas por CORRELAÇÃO (sem rotacionar), como estão escritas em
// mascaras.hpp. Assim Gx > 0 significa "fica mais claro para a direita" e Gy > 0
// "fica mais claro para baixo", e a máscara Kirsch "N" responde a regiões claras acima.
// Para a magnitude o sentido não importa (Gx² e Gy²).

#include "bordas_pixel.hpp"
#include "imagem.hpp"
#include "mascaras.hpp"

enum class Magnitude {
    Euclidiana,     // sqrt(Gx² + Gy²)
    SomaAbsolutos   // |Gx| + |Gy|
};

// Combina Gx e Gy em |∇f|
ImagemF magnitudeGradiente(const ImagemF& gx, const ImagemF& gy, Magnitude tipo);

// extras: "Gx", "Gy" (normalizados: cinza médio = 0) e "Magnitude" (com a faixa escolhida)
Resultado roberts(const Imagem& img, Borda borda, Magnitude tipo = Magnitude::Euclidiana,
                  Faixa faixa = Faixa::Saturar);
Resultado sobel(const Imagem& img, Borda borda, Magnitude tipo = Magnitude::Euclidiana,
                Faixa faixa = Faixa::Saturar);

// extras: "K0 (N)" ... "K7 (NO)" e "Maximo". Resposta final = max das 8 respostas.
Resultado kirsch(const Imagem& img, Borda borda, Faixa faixa = Faixa::Saturar);

// vizinhos = 4 ou 8. extras: "Laplaciano (normalizado)" e "Laplaciano" (com a faixa escolhida)
Resultado laplaciano(const Imagem& img, int vizinhos, Borda borda, Faixa faixa = Faixa::Absoluto);
