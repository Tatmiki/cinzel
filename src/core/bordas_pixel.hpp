#pragma once
// Acesso a pixels FORA da imagem (tratamento de bordas).
// Toda operação de vizinhança lê pixels por aqui: trocar a estratégia = mexer só neste arquivo.

#include "imagem.hpp"

enum class Borda {
    Zero,      // fora da imagem vale 0
    Replicar,  // repete o pixel da borda mais próximo:  x = clamp(x, 0, L-1)
    Refletir,  // espelho sem repetir a borda:           x = -1 -> 1,  x = L -> L-2
    Circular   // a imagem se repete (periódica):         x = -1 -> L-1
};

// Retorna f~(x, y): f(x, y) se (x, y) estiver dentro, senão aplica a estratégia.
float pixelComBorda(const ImagemF& img, int x, int y, Borda borda);

const char* nomeBorda(Borda borda);
