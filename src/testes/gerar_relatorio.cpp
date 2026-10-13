// Gera TODAS as imagens usadas no relatório, organizadas por seção em imagens/saida/,
// e um RESUMO.md com a lista de arquivos, parâmetros e máscaras usados.
// Rodar:  ./build/cinzel_relatorio

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <string>

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

namespace fs = std::filesystem;

#ifndef CINZEL_RAIZ
#define CINZEL_RAIZ "."
#endif

// ---------------------------------------------------------------------------
// Utilitários de saída (não fazem parte dos algoritmos avaliados)
// ---------------------------------------------------------------------------

static const fs::path ENTRADA = fs::path(CINZEL_RAIZ) / "imagens" / "entrada";
static const fs::path SAIDA = fs::path(CINZEL_RAIZ) / "imagens" / "saida";

static std::ofstream resumo;
static fs::path pastaAtual;
static int arquivosGerados = 0;

static Imagem abrir(const std::string& nome) {
    Imagem img;
    if (!carregar((ENTRADA / nome).string(), img)) {
        std::fprintf(stderr, "ERRO: nao consegui abrir imagens/entrada/%s\n", nome.c_str());
        std::exit(1);
    }
    return img;
}

static void secao(const std::string& pasta, const std::string& titulo, const std::string& texto) {
    pastaAtual = SAIDA / pasta;
    fs::create_directories(pastaAtual);
    std::printf("\n[%s] %s\n", pasta.c_str(), titulo.c_str());
    resumo << "\n## " << titulo << "\n\nPasta: `imagens/saida/" << pasta << "/`\n\n";
    if (!texto.empty()) resumo << texto << "\n\n";
    resumo << "| Arquivo | Descrição |\n|---|---|\n";
}

static void nota(const std::string& texto) { resumo << "\n" << texto << "\n\n| Arquivo | Descrição |\n|---|---|\n"; }

static void gravar(const std::string& arquivo, const Imagem& img, const std::string& descricao) {
    fs::path caminho = pastaAtual / arquivo;
    if (!salvar(caminho.string(), img)) {
        std::fprintf(stderr, "ERRO: nao consegui salvar %s\n", caminho.string().c_str());
        std::exit(1);
    }
    ++arquivosGerados;
    std::printf("  %s\n", arquivo.c_str());
    resumo << "| `" << arquivo << "` | " << descricao << " |\n";
}

// Salva a imagem e o desenho do seu histograma
static void gravarComHistograma(const std::string& base, const Imagem& img, const std::string& descricao) {
    gravar(base + ".png", img, descricao);
    gravar(base + "_hist.png", desenharHistograma(img), "Histograma de `" + base + ".png`");
}

static void gravarResultado(const std::string& prefixo, const Resultado& r, const std::string& descricao) {
    for (const auto& [nome, img] : r.extras) {
        // "K0 (N)" -> "K0_N",  "Laplaciano (normalizado)" -> "Laplaciano_normalizado"
        std::string arq;
        for (char c : nome) {
            if (c == ' ') arq += '_';
            else if (c != '(' && c != ')') arq += c;
        }
        gravar(prefixo + "_" + arq + ".png", img, descricao + ": " + nome);
    }
}

// Desenha a curva s = T(r) de uma operação pontual aplicando-a a uma rampa 0..255
template <typename Op>
static Imagem curva(Op operacao) {
    Imagem rampa(256, 1);
    for (int r = 0; r < 256; ++r) rampa.px[r] = static_cast<uint8_t>(r);
    Imagem s = operacao(rampa);

    Imagem grafico(256, 256, 255);
    for (int i = 0; i < 256; ++i) grafico.at(i, 255 - i) = 200;  // diagonal (identidade) em cinza
    for (int r = 0; r < 256; ++r) {
        int y0 = 255 - s.px[r];
        int y1 = 255 - s.px[r > 0 ? r - 1 : 0];
        for (int y = std::min(y0, y1); y <= std::max(y0, y1); ++y) grafico.at(r, y) = 0;
    }
    return grafico;
}

static std::string textoMascara(const Mascara& h) {
    std::ostringstream o;
    o << "$$\\begin{bmatrix}";
    for (int i = 0; i < h.linhas; ++i) {
        for (int j = 0; j < h.colunas; ++j) o << (j ? " & " : "") << h.at(i, j);
        o << (i + 1 < h.linhas ? " \\\\ " : "");
    }
    o << "\\end{bmatrix} \\quad \\textstyle\\sum h = " << h.soma() << "$$";
    return o.str();
}

static std::string num(double v) {
    std::ostringstream o;
    o << v;
    return o.str();
}

// Número para nome de arquivo, sem ponto (0.25 -> "025"), para não confundir com a extensão
static std::string numArq(double v) {
    std::string s = num(v);
    s.erase(std::remove(s.begin(), s.end(), '.'), s.end());
    return s;
}

// ---------------------------------------------------------------------------
// Experimentos
// ---------------------------------------------------------------------------

static void algebricas() {
    secao("01_algebricas", "1. Operações algébricas: dissolve cruzado",
          "Entradas: `lena_gray_512.tif` (f1) e `macaco.tiff` (f2), ambas 512x512.\n"
          "Uniforme: $g = (1-t) f_1 + t f_2$. Não uniforme: $g = (1-t(x,y)) f_1 + t(x,y) f_2$.");
    Imagem f1 = abrir("lena_gray_512.tif");
    Imagem f2 = abrir("macaco.tiff");
    gravar("original_f1.png", f1, "Entrada f1 (Lena)");
    gravar("original_f2.png", f2, "Entrada f2 (Mandril)");
    for (float t : {0.25f, 0.5f, 0.75f})
        gravar("dissolve_uniforme_t" + numArq(t) + ".png", dissolveUniforme(f1, f2, t),
               "Dissolve uniforme, t = " + num(t));

    ImagemF rampa = pesosRampaHorizontal(f1.largura, f1.altura);
    gravar("pesos_rampa.png", paraUint8(rampa, Faixa::Normalizar), "Pesos t(x,y) = x/(L-1) (preto = 0, branco = 1)");
    gravar("dissolve_nao_uniforme_rampa.png", dissolveNaoUniforme(f1, f2, rampa),
           "Dissolve não uniforme com rampa horizontal: f1 à esquerda, f2 à direita");

    ImagemF radial = pesosRadial(f1.largura, f1.altura);
    gravar("pesos_radial.png", paraUint8(radial, Faixa::Normalizar), "Pesos t(x,y) = distância ao centro / distância máxima");
    gravar("dissolve_nao_uniforme_radial.png", dissolveNaoUniforme(f1, f2, radial),
           "Dissolve não uniforme radial: f1 no centro, f2 nos cantos");
}

static void intensidade() {
    secao("02_intensidade", "2. Transformações de intensidade",
          "Cada operação tem a imagem original, o resultado e a curva $s = T(r)$ "
          "(curva preta; a diagonal cinza é a identidade).");

    Imagem mamo = abrir("mamografia.png");
    gravar("negativo_original.png", mamo, "Original: mamografia.png");
    gravar("negativo_resultado.png", negativo(mamo), "Negativo: s = 255 - r");
    gravar("negativo_curva.png", curva([](const Imagem& i) { return negativo(i); }), "Curva do negativo");

    Imagem poly = abrir("polymersomes.tif");
    gravar("contraste_original.png", poly, "Original: polymersomes.tif (baixo contraste, valores entre 90 e 241)");
    gravar("contraste_resultado.png", alargarContraste(poly, 140, 30, 200, 230),
           "Alargamento de contraste: (r1,s1) = (140,30), (r2,s2) = (200,230)");
    gravar("contraste_curva.png", curva([](const Imagem& i) { return alargarContraste(i, 140, 30, 200, 230); }),
           "Curva do alargamento de contraste");

    Imagem lim = abrir("limiar.png");
    gravar("limiar_original.png", lim, "Original: limiar.png");
    for (int T : {60, 128, 180})
        gravar("limiar_T" + std::to_string(T) + ".png", limiarizar(lim, T), "Limiarização, T = " + std::to_string(T));

    Imagem frat = abrir("fratura.png");
    gravar("gamma_escura_original.png", frat, "Original: fratura.png (predominantemente escura)");
    for (float g : {0.6f, 0.5f, 0.4f, 0.3f}) {
        gravar("gamma_escura_g" + numArq(g) + ".png", potencia(frat, 1.0f, g), "Potência, c = 1, γ = " + num(g));
        gravar("gamma_curva_g" + numArq(g) + ".png", curva([g](const Imagem& i) { return potencia(i, 1.0f, g); }),
               "Curva da potência, γ = " + num(g));
    }
    Imagem lavada = abrir("washed.jpg");
    gravar("gamma_clara_original.png", lavada, "Original: washed.jpg (predominantemente clara, 'lavada')");
    for (float g : {3.0f, 4.0f, 5.0f}) {
        gravar("gamma_clara_g" + numArq(g) + ".png", potencia(lavada, 1.0f, g), "Potência, c = 1, γ = " + num(g));
        gravar("gamma_curva_g" + numArq(g) + ".png", curva([g](const Imagem& i) { return potencia(i, 1.0f, g); }),
               "Curva da potência, γ = " + num(g));
    }

    Imagem hub = abrir("hubble-original.tif");
    gravar("log_original.png", hub, "Original: hubble-original.tif (escura)");
    gravar("log_resultado.png", logaritmo(hub, 0.0f), "Logaritmo, c = 255/ln(256) ≈ 45,99");
    gravar("log_curva.png", curva([](const Imagem& i) { return logaritmo(i, 0.0f); }), "Curva do logaritmo");
}

static void histogramas() {
    secao("03_histograma", "3. Histograma: expansão e equalização",
          "Cada imagem vem acompanhada do desenho do seu histograma (`*_hist.png`).");

    Imagem poly = abrir("polymersomes.tif");
    gravarComHistograma("polymersomes_original", poly, "Original: polymersomes.tif");
    gravarComHistograma("polymersomes_expansao", expandirHistograma(poly), "Expansão de histograma");
    gravarComHistograma("polymersomes_equalizacao", equalizarHistograma(poly), "Equalização de histograma");

    Imagem reg = abrir("regiao.tif");
    gravarComHistograma("regiao_original", reg, "Original: regiao.tif (valores entre 127 e 204)");
    gravarComHistograma("regiao_expansao", expandirHistograma(reg), "Expansão de histograma");
    gravarComHistograma("regiao_equalizacao", equalizarHistograma(reg), "Equalização de histograma");

    Imagem sol = abrir("sol2.jpg");
    gravarComHistograma("sol2_original", sol, "Original: sol2.jpg (escura)");
    gravarComHistograma("sol2_expansao", expandirHistograma(sol), "Expansão (quase não muda: já usa quase toda a faixa)");
    gravarComHistograma("sol2_equalizacao", equalizarHistograma(sol), "Equalização de histograma");

    Imagem lavada = abrir("washed.jpg");
    gravarComHistograma("washed_original", lavada, "Original: washed.jpg (clara)");
    gravarComHistograma("washed_equalizacao", equalizarHistograma(lavada), "Equalização de histograma");
}

static void adaptativo() {
    secao("04_contraste_adaptativo", "4. Contraste adaptativo",
          "Entrada: `sol2.jpg`. Fórmula: $g = m + \\frac{c}{\\sigma}(f - m)$, com média $m$ e desvio "
          "padrão $\\sigma$ da vizinhança $n \\times n$. Borda: replicar.");
    Imagem sol = abrir("sol2.jpg");
    gravar("original.png", sol, "Original: sol2.jpg");
    struct P { float c; int n; };
    for (P p : {P{20, 7}, P{20, 15}, P{40, 15}, P{40, 31}}) {
        std::string nome = "c" + num(p.c) + "_n" + std::to_string(p.n);
        Resultado r = contrasteAdaptativo(sol, p.c, p.n, Borda::Replicar);
        gravar(nome + ".png", r.principal, "Contraste adaptativo, c = " + num(p.c) + ", n = " + std::to_string(p.n));
        if (p.c == 20 && p.n == 15) {
            gravar(nome + "_media_local.png", r.extras[0].second, "Média local m(x,y), n = 15");
            gravar(nome + "_desvio_local.png", r.extras[1].second, "Desvio padrão local σ(x,y), n = 15 (normalizado)");
        }
    }
}

static void filtragem() {
    secao("05_filtragem", "5. Filtragem espacial: média e mediana",
          "Entrada: `lena_gray_256.tif` com 5% de ruído sal-e-pimenta adicionado pelo gerador "
          "(semente fixa 42, então o ruído é sempre o mesmo). Borda: replicar.");
    Imagem lena = abrir("lena_gray_256.tif");
    Imagem ruido = lena;
    std::mt19937 gerador(42);
    std::uniform_real_distribution<float> u(0.0f, 1.0f);
    for (uint8_t& p : ruido.px) {
        float sorteio = u(gerador);
        if (sorteio < 0.025f) p = 0;          // pimenta
        else if (sorteio < 0.05f) p = 255;    // sal
    }
    gravar("original.png", lena, "Original sem ruído: lena_gray_256.tif");
    gravar("ruido_sal_pimenta.png", ruido, "Entrada dos filtros: original + 5% de sal-e-pimenta");
    for (int n : {3, 5, 7}) {
        gravar("media_n" + std::to_string(n) + ".png", filtroMedia(ruido, n, Borda::Replicar),
               "Média " + std::to_string(n) + "x" + std::to_string(n));
        gravar("mediana_n" + std::to_string(n) + ".png", filtroMediana(ruido, n, Borda::Replicar),
               "Mediana " + std::to_string(n) + "x" + std::to_string(n));
    }

    nota("**Tratamento de bordas:** média 25x25 em `cameraman.tif` com cada estratégia. "
         "A diferença aparece numa faixa de 12 pixels junto às bordas da imagem.");
    Imagem cam = abrir("cameraman.tif");
    gravar("bordas_original.png", cam, "Original: cameraman.tif");
    for (Borda b : {Borda::Zero, Borda::Replicar, Borda::Refletir, Borda::Circular})
        gravar(std::string("bordas_media25_") + nomeBorda(b) + ".png", filtroMedia(cam, 25, b),
               std::string("Média 25x25, borda = ") + nomeBorda(b));
}

static void bordas() {
    secao("06_deteccao_bordas", "6. Detecção de bordas",
          "Entrada: `cameraman.tif`. Borda: replicar. Máscaras aplicadas por correlação. "
          "$G_x$ e $G_y$ são normalizados (cinza médio = 0, escuro = negativo, claro = positivo); "
          "magnitudes e respostas finais são saturadas em [0, 255].");
    Imagem cam = abrir("cameraman.tif");
    gravar("original.png", cam, "Original: cameraman.tif");

    gravarResultado("roberts", roberts(cam, Borda::Replicar), "Roberts");
    gravarResultado("sobel", sobel(cam, Borda::Replicar), "Sobel");
    gravar("sobel_Magnitude_soma_abs.png", sobel(cam, Borda::Replicar, Magnitude::SomaAbsolutos).principal,
           "Sobel: magnitude aproximada |Gx| + |Gy|");

    Resultado k = kirsch(cam, Borda::Replicar);
    gravarResultado("kirsch", k, "Kirsch");
    gravar("kirsch_Maximo_normalizado.png", kirsch(cam, Borda::Replicar, Faixa::Normalizar).principal,
           "Kirsch: resposta final (máximo) normalizada em vez de saturada");

    gravarResultado("laplaciano4", laplaciano(cam, 4, Borda::Replicar), "Laplaciano 4 vizinhos");
    gravarResultado("laplaciano8", laplaciano(cam, 8, Borda::Replicar), "Laplaciano 8 vizinhos");
}

static void agucamento() {
    secao("07_agucamento", "7. Aguçamento",
          "Laplaciano: $g = f - \\nabla^2 f$ (centro da máscara negativo, então c = -1). "
          "High boost: $g = f + k(f - \\bar f)$, com $\\bar f$ = média 5x5. Borda: replicar.");
    Imagem lua = abrir("lua.jpg");
    gravar("lua_original.png", lua, "Original: lua.jpg");
    gravarResultado("lua_lap4", agucarLaplaciano(lua, 4, Borda::Replicar), "Aguçamento, Laplaciano 4 vizinhos");
    gravarResultado("lua_lap8", agucarLaplaciano(lua, 8, Borda::Replicar), "Aguçamento, Laplaciano 8 vizinhos");

    Imagem cam = abrir("cameraman.tif");
    gravar("highboost_original.png", cam, "Original: cameraman.tif");
    for (float kk : {1.0f, 2.0f, 4.0f}) {
        Resultado r = highBoost(cam, kk, 5, Borda::Replicar);
        std::string nome = "highboost_k" + num(kk);
        gravar(nome + ".png", r.principal,
               kk == 1.0f ? "k = 1 (unsharp masking)" : "High boost, k = " + num(kk));
        if (kk == 1.0f) {
            gravar("highboost_suavizada.png", r.extras[0].second, "Imagem suavizada f̄ (média 5x5)");
            gravar("highboost_mascara_nitidez.png", r.extras[1].second, "Máscara de nitidez f - f̄ (normalizada)");
        }
    }
}

static void testesConvolucao() {
    secao("08_testes_convolucao", "8. Testes obrigatórios de convolução (seção 2 da especificação)",
          "Entrada: `cameraman.tif`. Todos com a convolução genérica $g = f * h + \\text{offset}$ "
          "(máscara rotacionada 180°, convolução verdadeira), borda = replicar.");
    Imagem cam = abrir("cameraman.tif");
    gravar("original.png", cam, "Original: cameraman.tif");

    // 2.1 Aguçamento
    nota("### 2.1 Filtro de aguçamento\n\n" + textoMascara(mascaras::agucamentoCD(1, 1)) +
         "\n\nMáscara geral: centro $8c + d$, vizinhos $-c$, soma $= d$. Faixa: saturar, offset 0.");
    struct CD { int c, d; };
    for (CD p : {CD{1, 1}, CD{1, 2}, CD{1, 4}, CD{2, 1}, CD{3, 1}}) {
        Mascara h = mascaras::agucamentoCD(p.c, p.d);
        std::string nome = "t21_c" + std::to_string(p.c) + "_d" + std::to_string(p.d) + ".png";
        gravar(nome, convolucaoGenerica(cam, h, 0, Borda::Replicar, Faixa::Saturar),
               "c = " + std::to_string(p.c) + ", d = " + std::to_string(p.d) +
               " (centro " + num(h.at(1, 1)) + ", soma " + num(h.soma()) + ")");
    }

    // 2.2 Relevo
    nota("### 2.2 Filtros de relevo\n\n$h_1$: " + textoMascara(mascaras::relevoH1()) +
         "\n\n$h_2$: " + textoMascara(mascaras::relevoH2()) +
         "\n\nCom offset 128 o cinza médio representa \"sem variação\". Também foi gerada a versão "
         "por correlação (sem rotacionar) para mostrar que o sentido do relevo inverte.");
    Mascara h1 = mascaras::relevoH1(), h2 = mascaras::relevoH2();
    gravar("t22_h1_offset128.png", convolucaoGenerica(cam, h1, 128, Borda::Replicar, Faixa::Saturar), "h1, offset 128, saturar");
    gravar("t22_h2_offset128.png", convolucaoGenerica(cam, h2, 128, Borda::Replicar, Faixa::Saturar), "h2, offset 128, saturar");
    gravar("t22_h1_offset0.png", convolucaoGenerica(cam, h1, 0, Borda::Replicar, Faixa::Saturar), "h1, offset 0, saturar (negativos viram 0)");
    gravar("t22_h2_offset0.png", convolucaoGenerica(cam, h2, 0, Borda::Replicar, Faixa::Saturar), "h2, offset 0, saturar (negativos viram 0)");
    gravar("t22_h1_offset128_correlacao.png", convolucaoGenerica(cam, h1, 128, Borda::Replicar, Faixa::Saturar, false),
           "h1, offset 128, por CORRELAÇÃO (compare com a convolução)");
    gravar("t22_h2_offset128_correlacao.png", convolucaoGenerica(cam, h2, 128, Borda::Replicar, Faixa::Saturar, false),
           "h2, offset 128, por CORRELAÇÃO (compare com a convolução)");

    // 2.3 Bordas
    nota("### 2.3 Detecção de bordas\n\n$h_3$: " + textoMascara(mascaras::bordaH3()) +
         "\n\n$h_4$: " + textoMascara(mascaras::bordaH4()) +
         "\n\nCada máscara com as três estratégias de faixa (offset 0), para discutir os valores "
         "fora de [0, 255].");
    struct F { Faixa f; const char* nome; };
    for (F fx : {F{Faixa::Saturar, "saturar"}, F{Faixa::Absoluto, "absoluto"}, F{Faixa::Normalizar, "normalizar"}}) {
        gravar(std::string("t23_h3_") + fx.nome + ".png",
               convolucaoGenerica(cam, mascaras::bordaH3(), 0, Borda::Replicar, fx.f), std::string("h3, faixa = ") + fx.nome);
        gravar(std::string("t23_h4_") + fx.nome + ".png",
               convolucaoGenerica(cam, mascaras::bordaH4(), 0, Borda::Replicar, fx.f), std::string("h4, faixa = ") + fx.nome);
    }
}

int main() {
    fs::create_directories(SAIDA);
    resumo.open(SAIDA / "RESUMO.md");
    resumo << "# Resumo das imagens geradas\n\n"
              "Gerado automaticamente por `./build/cinzel_relatorio`. "
              "Rode de novo para regenerar tudo (os arquivos são sobrescritos).\n";

    algebricas();
    intensidade();
    histogramas();
    adaptativo();
    filtragem();
    bordas();
    agucamento();
    testesConvolucao();

    std::printf("\n%d imagens geradas em %s\n", arquivosGerados, SAIDA.string().c_str());
    std::printf("Resumo: %s\n", (SAIDA / "RESUMO.md").string().c_str());
    return 0;
}
