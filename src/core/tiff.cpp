// Leitura de TIFF com a libtiff (o stb_image não lê TIFF).
// É LEITURA de arquivo (permitida pela especificação), não processamento de imagem.

#include <tiffio.h>

#include <vector>

#include "imagem.hpp"

bool carregarTIFF(const std::string& caminho, Imagem& img) {
    TIFFSetWarningHandler(nullptr);  // não imprimir avisos sobre tags desconhecidas
    TIFF* tif = TIFFOpen(caminho.c_str(), "r");
    if (!tif) return false;

    uint32_t largura = 0, altura = 0;
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &largura);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &altura);

    // A libtiff decodifica qualquer variante (compressão, 1/8/16 bits, cinza, RGB, paleta)
    // para RGBA 8 bits por canal, já com a primeira linha no topo.
    std::vector<uint32_t> rgba(static_cast<size_t>(largura) * altura);
    bool ok = TIFFReadRGBAImageOriented(tif, largura, altura, rgba.data(), ORIENTATION_TOPLEFT, 0);
    TIFFClose(tif);
    if (!ok) return false;

    // Cor -> cinza com os mesmos pesos do stb_image: (77R + 150G + 29B) / 256
    img = Imagem(static_cast<int>(largura), static_cast<int>(altura));
    for (size_t i = 0; i < rgba.size(); ++i) {
        uint32_t R = TIFFGetR(rgba[i]), G = TIFFGetG(rgba[i]), B = TIFFGetB(rgba[i]);
        img.px[i] = static_cast<uint8_t>((R * 77 + G * 150 + B * 29) >> 8);
    }
    return true;
}
