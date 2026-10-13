#pragma once
// Operações algébricas: dissolve cruzado.
// As duas imagens precisam ter o mesmo tamanho (senão lança std::invalid_argument).

#include "imagem.hpp"

// Uniforme: g = (1 - t)·f1 + t·f2, com o mesmo t em todos os pixels.
Imagem dissolveUniforme(const Imagem& f1, const Imagem& f2, float t);

// Não uniforme: g = (1 - t(x,y))·f1 + t(x,y)·f2, com pesos t(x,y) em [0, 1].
Imagem dissolveNaoUniforme(const Imagem& f1, const Imagem& f2, const ImagemF& pesos);

// Geradores de pesos t(x,y) para o dissolve não uniforme
ImagemF pesosRampaHorizontal(int largura, int altura);  // t = x / (L-1): f1 à esquerda, f2 à direita
ImagemF pesosRadial(int largura, int altura);           // t = distância ao centro / distância máxima
ImagemF pesosDeImagem(const Imagem& mascara);           // t = m(x,y) / 255
