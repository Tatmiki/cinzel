#include "bordas_pixel.hpp"

#include <algorithm>

// Converte uma coordenada qualquer em uma coordenada válida em [0, n-1].
static int coordenadaValida(int c, int n, Borda borda) {
    switch (borda) {
        case Borda::Replicar:
            return std::clamp(c, 0, n - 1);
        case Borda::Refletir:
            if (n == 1) return 0;
            // Repete enquanto estiver fora (máscaras maiores que a imagem)
            while (c < 0 || c >= n) {
                if (c < 0) c = -c;
                if (c >= n) c = 2 * (n - 1) - c;
            }
            return c;
        case Borda::Circular:
            return ((c % n) + n) % n;
        case Borda::Zero:
            break;
    }
    return c;
}

float pixelComBorda(const ImagemF& img, int x, int y, Borda borda) {
    bool dentro = x >= 0 && x < img.largura && y >= 0 && y < img.altura;
    if (dentro) return img.at(x, y);
    if (borda == Borda::Zero) return 0.0f;

    x = coordenadaValida(x, img.largura, borda);
    y = coordenadaValida(y, img.altura, borda);
    return img.at(x, y);
}

const char* nomeBorda(Borda borda) {
    switch (borda) {
        case Borda::Zero: return "zero";
        case Borda::Replicar: return "replicar";
        case Borda::Refletir: return "refletir";
        case Borda::Circular: return "circular";
    }
    return "?";
}
