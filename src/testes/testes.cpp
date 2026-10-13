// Testes unitários com matrizes pequenas: cada teste confere um cálculo feito à mão.
// Rodar:  ./build/cinzel_testes

#include <cstdio>
#include <string>
#include <vector>

#include "adaptativo.hpp"
#include "agucamento.hpp"
#include "algebricas.hpp"
#include "convolucao.hpp"
#include "deteccao.hpp"
#include "filtros.hpp"
#include "histograma.hpp"
#include "imagem.hpp"
#include "mascaras.hpp"
#include "pontuais.hpp"

#ifndef CINZEL_RAIZ
#define CINZEL_RAIZ "."
#endif

static int falhas = 0, total = 0;

#define VERIFICA(cond)                                                       \
    do {                                                                     \
        ++total;                                                             \
        if (!(cond)) {                                                       \
            ++falhas;                                                        \
            std::printf("  FALHOU  %s:%d  %s\n", __FILE__, __LINE__, #cond); \
        }                                                                    \
    } while (0)

#define VERIFICA_IGUAL(obtido, esperado)                                                  \
    do {                                                                                  \
        ++total;                                                                          \
        double _o = (obtido), _e = (esperado);                                            \
        if (_o < _e - 1e-3 || _o > _e + 1e-3) {                                           \
            ++falhas;                                                                     \
            std::printf("  FALHOU  %s:%d  %s = %g (esperado %g)\n", __FILE__, __LINE__,   \
                        #obtido, _o, _e);                                                 \
        }                                                                                 \
    } while (0)

static void secao(const char* nome) { std::printf("- %s\n", nome); }

// Monta imagens pequenas a partir de listas de valores
static Imagem img(int l, int a, std::vector<uint8_t> v) { return Imagem(l, a, std::move(v)); }
static ImagemF imgF(int l, int a, std::vector<float> v) {
    ImagemF f(l, a);
    f.px = std::move(v);
    return f;
}

// ---------------------------------------------------------------------------

static void testeConversaoFaixa() {
    secao("Conversao float -> 8 bits (Saturar, Absoluto, Normalizar)");
    ImagemF v = imgF(4, 1, {-160, 0, 50, 300});
    VERIFICA((paraUint8(v, Faixa::Saturar).px == std::vector<uint8_t>{0, 0, 50, 255}));
    VERIFICA((paraUint8(v, Faixa::Absoluto).px == std::vector<uint8_t>{160, 0, 50, 255}));
    VERIFICA((paraUint8(v, Faixa::Normalizar).px == std::vector<uint8_t>{0, 89, 116, 255}));
    // Imagem constante não pode dividir por zero
    VERIFICA((paraUint8(imgF(2, 1, {7, 7}), Faixa::Normalizar).px == std::vector<uint8_t>{0, 0}));
}

static void testePontuais() {
    secao("Transformacoes pontuais");
    Imagem a = img(4, 1, {0, 50, 128, 255});
    VERIFICA((negativo(a).px == std::vector<uint8_t>{255, 205, 127, 0}));
    VERIFICA(negativo(negativo(a)).px == a.px);

    VERIFICA((limiarizar(img(4, 1, {0, 127, 128, 255}), 128).px == std::vector<uint8_t>{0, 0, 255, 255}));

    // Gamma (c = 1): 64 -> 128 (γ=0,5), 64 -> 16 (γ=2), 200 -> 226 (γ=0,5)
    Imagem g = img(2, 1, {64, 200});
    VERIFICA_IGUAL(potencia(g, 1.0f, 0.5f).px[0], 128);
    VERIFICA_IGUAL(potencia(g, 1.0f, 2.0f).px[0], 16);
    VERIFICA_IGUAL(potencia(g, 1.0f, 0.5f).px[1], 226);
    VERIFICA(potencia(a, 1.0f, 1.0f).px == a.px);  // γ = 1 é a identidade

    // Log com c automático: 10 -> 110, 255 -> 255
    Imagem l = logaritmo(img(3, 1, {0, 10, 255}), 0.0f);
    VERIFICA((l.px == std::vector<uint8_t>{0, 110, 255}));

    // Alargamento de contraste
    Imagem rampa(256, 1);
    for (int r = 0; r < 256; ++r) rampa.px[r] = static_cast<uint8_t>(r);
    VERIFICA(alargarContraste(rampa, 50, 50, 200, 200).px == rampa.px);  // pontos na diagonal = identidade
    Imagem c = alargarContraste(rampa, 100, 50, 150, 200);
    VERIFICA_IGUAL(c.px[50], 25);
    VERIFICA_IGUAL(c.px[100], 50);
    VERIFICA_IGUAL(c.px[125], 125);
    VERIFICA_IGUAL(c.px[150], 200);
    VERIFICA_IGUAL(c.px[255], 255);
    // Pontos extremos (r1 = 0, r2 = 255) sem divisão por zero
    VERIFICA_IGUAL(alargarContraste(rampa, 0, 0, 255, 255).px[77], 77);
}

static void testeAlgebricas() {
    secao("Dissolve cruzado");
    Imagem f1 = img(3, 1, {200, 200, 200}), f2 = img(3, 1, {40, 0, 0});
    VERIFICA_IGUAL(dissolveUniforme(f1, f2, 0.25f).px[0], 160);  // 0,75·200 + 0,25·40
    VERIFICA(dissolveUniforme(f1, f2, 0.0f).px == f1.px);
    VERIFICA(dissolveUniforme(f1, f2, 1.0f).px == f2.px);

    Imagem z = img(3, 1, {0, 0, 0});
    Imagem nu = dissolveNaoUniforme(f1, z, pesosRampaHorizontal(3, 1));  // t = 0; 0,5; 1
    VERIFICA((nu.px == std::vector<uint8_t>{200, 100, 0}));

    bool lancou = false;
    try { dissolveUniforme(f1, img(1, 1, {0}), 0.5f); } catch (const std::exception&) { lancou = true; }
    VERIFICA(lancou);
}

static void testeHistograma() {
    secao("Histograma, expansao e equalizacao");
    VERIFICA((expandirHistograma(img(3, 1, {50, 100, 150})).px == std::vector<uint8_t>{0, 128, 255}));

    // 16 pixels: seis com 2, seis com 3, quatro com 5
    std::vector<uint8_t> v;
    v.insert(v.end(), 6, 2);
    v.insert(v.end(), 6, 3);
    v.insert(v.end(), 4, 5);
    Imagem a = img(4, 4, v);
    Histograma h = histograma(a);
    VERIFICA(h[2] == 6 && h[3] == 6 && h[5] == 4 && h[0] == 0);
    Imagem e = equalizarHistograma(a);
    VERIFICA_IGUAL(e.px[0], 96);    // round(255 · 6/16)
    VERIFICA_IGUAL(e.px[6], 191);   // round(255 · 12/16)
    VERIFICA_IGUAL(e.px[12], 255);  // round(255 · 16/16)

    Imagem constante = img(2, 1, {9, 9});
    VERIFICA(expandirHistograma(constante).px == constante.px);
}

static void testeBordas() {
    secao("Pixel fora da imagem (bordas)");
    ImagemF linha = imgF(3, 1, {10, 20, 30});
    VERIFICA_IGUAL(pixelComBorda(linha, -1, 0, Borda::Zero), 0);
    VERIFICA_IGUAL(pixelComBorda(linha, -1, 0, Borda::Replicar), 10);
    VERIFICA_IGUAL(pixelComBorda(linha, -1, 0, Borda::Refletir), 20);
    VERIFICA_IGUAL(pixelComBorda(linha, -1, 0, Borda::Circular), 30);
    VERIFICA_IGUAL(pixelComBorda(linha, -2, 0, Borda::Refletir), 30);
    VERIFICA_IGUAL(pixelComBorda(linha, 3, 0, Borda::Replicar), 30);
    VERIFICA_IGUAL(pixelComBorda(linha, 3, 0, Borda::Refletir), 20);
    VERIFICA_IGUAL(pixelComBorda(linha, 3, 0, Borda::Circular), 10);
    VERIFICA_IGUAL(pixelComBorda(linha, 1, -1, Borda::Replicar), 20);  // acima da imagem
}

static void testeConvolucao() {
    secao("Convolucao generica");
    // Laplaciano no ponto claro: 4·10 - 4·50 = -160
    ImagemF f = imgF(3, 3, {10, 10, 10,
                            10, 50, 10,
                            10, 10, 10});
    VERIFICA_IGUAL(convoluir(f, mascaras::laplaciano4(), Borda::Replicar, true).at(1, 1), -160);

    // Máscara 1x1 = [1] devolve a própria imagem
    Imagem a = img(3, 1, {1, 2, 3});
    VERIFICA(convolucaoGenerica(a, Mascara(1, 1, {1}), 0, Borda::Zero, Faixa::Saturar).px == a.px);

    // Máscara de deslocamento: diferencia convolução (rotaciona) de correlação
    ImagemF n = imgF(3, 3, {1, 2, 3,
                            4, 5, 6,
                            7, 8, 9});
    Mascara desloca(3, 3, {0, 0, 0,
                           0, 0, 1,
                           0, 0, 0});
    VERIFICA_IGUAL(convoluir(n, desloca, Borda::Zero, false).at(1, 1), 6);  // correlação: f(x+1)
    VERIFICA_IGUAL(convoluir(n, desloca, Borda::Zero, true).at(1, 1), 4);   // convolução: f(x-1)

    // Offset é somado antes da faixa
    VERIFICA_IGUAL(convolucaoGenerica(a, Mascara(1, 1, {0}), 128, Borda::Zero, Faixa::Saturar).px[0], 128);

    // Soma dos coeficientes da máscara do teste 2.1 = d
    VERIFICA_IGUAL(mascaras::agucamentoCD(1, 1).soma(), 1);
    VERIFICA_IGUAL(mascaras::agucamentoCD(2, 3).soma(), 3);
    VERIFICA_IGUAL(mascaras::relevoH1().soma(), 0);
    VERIFICA_IGUAL(mascaras::relevoH2().soma(), 0);
    VERIFICA_IGUAL(mascaras::bordaH3().soma(), 2);
    VERIFICA_IGUAL(mascaras::bordaH4().soma(), -8);
}

static void testeFiltros() {
    secao("Media e mediana");
    Imagem sal = img(3, 3, {10, 10, 10,
                            10, 255, 10,
                            10, 10, 10});
    VERIFICA_IGUAL(filtroMedia(sal, 3, Borda::Replicar).at(1, 1), 37);   // 335/9
    VERIFICA_IGUAL(filtroMediana(sal, 3, Borda::Replicar).at(1, 1), 10);
}

static void testeAdaptativo() {
    secao("Contraste adaptativo");
    Imagem w = img(3, 3, {100, 100, 100,
                          100, 110, 100,
                          100, 100, 100});
    // m ≈ 101,1; σ ≈ 3,14; g ≈ 101,1 + (10/3,14)·8,9 ≈ 129
    VERIFICA_IGUAL(contrasteAdaptativo(w, 10.0f, 3, Borda::Replicar).principal.at(1, 1), 129);
    // Região uniforme (σ = 0): não muda
    Imagem u(5, 5, 80);
    VERIFICA(contrasteAdaptativo(u, 10.0f, 3, Borda::Replicar).principal.px == u.px);
}

static void testeDeteccao() {
    secao("Deteccao de bordas");
    ImagemF borda = imgF(3, 3, {10, 10, 100,
                                10, 10, 100,
                                10, 10, 100});
    // Sobel: Gx = 90·1 + 90·2 + 90·1 = 360, Gy = 0
    VERIFICA_IGUAL(convoluir(borda, mascaras::sobelX(), Borda::Replicar, false).at(1, 1), 360);
    VERIFICA_IGUAL(convoluir(borda, mascaras::sobelY(), Borda::Replicar, false).at(1, 1), 0);
    Imagem b8 = paraUint8(borda, Faixa::Saturar);
    VERIFICA_IGUAL(sobel(b8, Borda::Replicar).principal.at(1, 1), 255);  // 360 saturado
    VERIFICA(sobel(b8, Borda::Replicar).extras.size() == 3);

    // Kirsch: as 8 máscaras somam 0; K1 (NE) tem o formato esperado; borda vertical -> K2 (L) vence
    for (int k = 0; k < 8; ++k) VERIFICA_IGUAL(mascaras::kirsch(k).soma(), 0);
    VERIFICA((mascaras::kirsch(1).v == std::vector<float>{-3, 5, 5, -3, 0, 5, -3, -3, -3}));
    int vencedora = -1;
    float melhor = -1e9f;
    for (int k = 0; k < 8; ++k) {
        float resp = convoluir(borda, mascaras::kirsch(k), Borda::Replicar, false).at(1, 1);
        if (resp > melhor) { melhor = resp; vencedora = k; }
    }
    VERIFICA(vencedora == 2);
    VERIFICA_IGUAL(melhor, 1350);  // 5·100·3 - 3·10·3 - 3·10·2
    VERIFICA(kirsch(b8, Borda::Replicar).extras.size() == 9);

    // Roberts: Gx = f(x,y) - f(x+1,y+1)
    ImagemF r = imgF(2, 2, {50, 0,
                            0, 20});
    VERIFICA_IGUAL(convoluir(r, mascaras::robertsX(), Borda::Replicar, false).at(0, 0), 30);

    // Magnitude: sqrt(3² + 4²) = 5 e |3| + |4| = 7
    ImagemF gx = imgF(1, 1, {3}), gy = imgF(1, 1, {-4});
    VERIFICA_IGUAL(magnitudeGradiente(gx, gy, Magnitude::Euclidiana).px[0], 5);
    VERIFICA_IGUAL(magnitudeGradiente(gx, gy, Magnitude::SomaAbsolutos).px[0], 7);
}

static void testeAgucamento() {
    secao("Agucamento");
    Imagem ponto = img(3, 3, {10, 10, 10,
                              10, 50, 10,
                              10, 10, 10});
    // g = f - ∇²f = 50 - (-160) = 210
    VERIFICA_IGUAL(agucarLaplaciano(ponto, 4, Borda::Replicar).principal.at(1, 1), 210);
    // Imagem uniforme não muda
    Imagem u(4, 4, 90);
    VERIFICA(agucarLaplaciano(u, 8, Borda::Replicar).principal.px == u.px);
    VERIFICA(highBoost(u, 3.0f, 3, Borda::Replicar).principal.px == u.px);
}

static void testeLeituraArquivos() {
    secao("Leitura de arquivos (TIFF proprio e stb)");
    std::string dir = std::string(CINZEL_RAIZ) + "/imagens/entrada/";
    struct Caso { const char* nome; int l, a; };
    const Caso casos[] = {
        {"cameraman.tif", 256, 256},     // TIFF PackBits, big endian
        {"lena_gray_512.tif", 512, 512}, // TIFF sem compressão
        {"macaco.tiff", 512, 512},       // TIFF RGB
        {"f1.tif", 220, 210},            // TIFF LZW
        {"lena512.bmp", 512, 512},       // stb
        {"lua.jpg", 466, 540},           // stb
    };
    for (const Caso& c : casos) {
        Imagem im;
        bool ok = carregar(dir + c.nome, im);
        if (!ok) std::printf("  (aviso: nao consegui abrir %s)\n", c.nome);
        VERIFICA(ok && im.largura == c.l && im.altura == c.a);
    }
}

int main() {
    std::printf("Testes unitarios do Cinzel\n\n");
    testeConversaoFaixa();
    testePontuais();
    testeAlgebricas();
    testeHistograma();
    testeBordas();
    testeConvolucao();
    testeFiltros();
    testeAdaptativo();
    testeDeteccao();
    testeAgucamento();
    testeLeituraArquivos();

    std::printf("\n%d de %d verificacoes passaram.\n", total - falhas, total);
    if (falhas == 0) std::printf("TODOS OS TESTES PASSARAM.\n");
    return falhas == 0 ? 0 : 1;
}
