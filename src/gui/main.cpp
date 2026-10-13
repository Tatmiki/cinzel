// Interface gráfica do Cinzel (Dear ImGui + GLFW + OpenGL 3).
// Este arquivo NÃO contém algoritmos: só chama as funções de src/core e mostra os resultados.

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cstdio>
#include <exception>
#include <filesystem>
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

namespace fs = std::filesystem;

#ifndef CINZEL_RAIZ
#define CINZEL_RAIZ "."
#endif

// ===========================================================================
// Exibição de imagens (boilerplate de OpenGL)
// ===========================================================================

// Envia uma Imagem cinza para a GPU como textura RGBA
static GLuint criarTextura(const Imagem& img) {
    std::vector<uint8_t> rgba(img.px.size() * 4);
    for (size_t i = 0; i < img.px.size(); ++i) {
        rgba[4 * i + 0] = rgba[4 * i + 1] = rgba[4 * i + 2] = img.px[i];
        rgba[4 * i + 3] = 255;
    }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // zoom mostra pixels nítidos
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.largura, img.altura, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
    return tex;
}

// Uma imagem + sua textura na GPU
struct Visualizacao {
    std::string nome;
    Imagem img;
    GLuint tex = 0;

    void atualizar(const Imagem& nova) {
        liberar();
        img = nova;
        if (!img.vazia()) tex = criarTextura(img);
    }
    void liberar() {
        if (tex) glDeleteTextures(1, &tex);
        tex = 0;
    }
    bool pronta() const { return tex != 0; }
};

// Mostra a imagem; ao passar o mouse, mostra o valor do pixel e a vizinhança 3x3
static void mostrarImagem(const Visualizacao& v, float escala) {
    if (!v.pronta()) {
        ImGui::TextDisabled("(vazio)");
        return;
    }
    ImGui::Image((ImTextureID)(intptr_t)v.tex, ImVec2(v.img.largura * escala, v.img.altura * escala));
    if (!ImGui::IsItemHovered()) return;

    ImVec2 canto = ImGui::GetItemRectMin();
    ImVec2 mouse = ImGui::GetIO().MousePos;
    int x = std::clamp(static_cast<int>((mouse.x - canto.x) / escala), 0, v.img.largura - 1);
    int y = std::clamp(static_cast<int>((mouse.y - canto.y) / escala), 0, v.img.altura - 1);

    ImGui::BeginTooltip();
    ImGui::Text("%s   (x = %d, y = %d)   valor = %d", v.nome.c_str(), x, y, v.img.at(x, y));
    ImGui::Separator();
    ImGui::Text("Vizinhanca 3x3:");
    for (int dy = -1; dy <= 1; ++dy) {
        std::string linha;
        for (int dx = -1; dx <= 1; ++dx) {
            int xx = x + dx, yy = y + dy;
            char buf[16];
            if (xx < 0 || yy < 0 || xx >= v.img.largura || yy >= v.img.altura) std::snprintf(buf, sizeof buf, "   -");
            else std::snprintf(buf, sizeof buf, "%4d", v.img.at(xx, yy));
            linha += buf;
        }
        ImGui::TextUnformatted(linha.c_str());
    }
    ImGui::EndTooltip();
}

static void mostrarHistograma(const char* id, const Imagem& img, float largura) {
    if (img.vazia()) return;
    Histograma h = histograma(img);
    float valores[256];
    for (int i = 0; i < 256; ++i) valores[i] = static_cast<float>(h[i]);
    ImGui::PlotHistogram(id, valores, 256, 0, nullptr, 0.0f, FLT_MAX, ImVec2(std::max(largura, 256.0f), 80));
}

// ===========================================================================
// Operações e parâmetros
// ===========================================================================

enum Operacao {
    OP_NEGATIVO, OP_LIMIAR, OP_CONTRASTE, OP_GAMMA, OP_LOG,
    OP_DISSOLVE_U, OP_DISSOLVE_NU,
    OP_EXPANSAO, OP_EQUALIZACAO,
    OP_ADAPTATIVO, OP_MEDIA, OP_MEDIANA,
    OP_ROBERTS, OP_SOBEL, OP_KIRSCH, OP_LAPLACIANO,
    OP_AGUC_LAP, OP_HIGHBOOST,
    OP_CONVOLUCAO,
    OP_TOTAL
};

static const char* NOMES_OP[OP_TOTAL] = {
    "Negativo", "Limiarizacao", "Alargamento de contraste", "Potencia (gamma)", "Logaritmo",
    "Dissolve cruzado uniforme", "Dissolve cruzado nao uniforme",
    "Expansao de histograma", "Equalizacao de histograma",
    "Contraste adaptativo", "Filtro da media", "Filtro da mediana",
    "Bordas: Roberts", "Bordas: Sobel", "Bordas: Kirsch", "Bordas: Laplaciano",
    "Agucamento: Laplaciano", "Agucamento: High boost",
    "Convolucao generica",
};

static const char* NOMES_BORDA[] = {"Zero", "Replicar", "Refletir", "Circular"};       // ordem do enum Borda
static const char* NOMES_FAIXA[] = {"Saturar", "Normalizar", "Absoluto"};             // ordem do enum Faixa
static const int MAX_MASCARA = 9;

struct Parametros {
    int T = 128;
    int r1 = 70, s1 = 20, r2 = 180, s2 = 235;
    float cGamma = 1.0f, gamma = 0.5f;
    float cLog = 0.0f;                    // 0 = automático
    float t = 0.5f;
    int pesos = 0;                        // 0 = rampa, 1 = radial
    int n = 3;                            // média / mediana
    float cAdapt = 20.0f;
    int nAdapt = 7;
    int magnitude = 0;                    // 0 = euclidiana, 1 = |Gx|+|Gy|
    int faixaBordas = 0;                  // Saturar
    int faixaLap = 2;                     // Absoluto
    int vizinhosLap = 4;
    float k = 2.0f;
    int nHB = 5;
    int borda = 1;                        // Replicar
    // convolução genérica
    int linhas = 3, colunas = 3;
    float mascara[MAX_MASCARA * MAX_MASCARA] = {0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 1, 0};  // identidade (h(1,1) = 1)
    float offset = 0.0f;
    bool rotacionar = true;
    int faixaConv = 0;
    int cTeste = 1, dTeste = 1;
};

static Mascara mascaraDosParametros(const Parametros& p) {
    Mascara h(p.linhas, p.colunas, std::vector<float>(p.linhas * p.colunas));
    for (int i = 0; i < p.linhas; ++i)
        for (int j = 0; j < p.colunas; ++j) h.at(i, j) = p.mascara[i * MAX_MASCARA + j];
    return h;
}

static void carregarMascara(Parametros& p, const Mascara& h) {
    std::fill(std::begin(p.mascara), std::end(p.mascara), 0.0f);
    p.linhas = h.linhas;
    p.colunas = h.colunas;
    for (int i = 0; i < h.linhas; ++i)
        for (int j = 0; j < h.colunas; ++j) p.mascara[i * MAX_MASCARA + j] = h.at(i, j);
}

static Resultado executar(int op, const Imagem& f, const Imagem& f2, const Parametros& p) {
    const Borda b = static_cast<Borda>(p.borda);
    auto so = [](const Imagem& img) { Resultado r; r.principal = img; return r; };

    switch (op) {
        case OP_NEGATIVO:    return so(negativo(f));
        case OP_LIMIAR:      return so(limiarizar(f, p.T));
        case OP_CONTRASTE:   return so(alargarContraste(f, p.r1, p.s1, p.r2, p.s2));
        case OP_GAMMA:       return so(potencia(f, p.cGamma, p.gamma));
        case OP_LOG:         return so(logaritmo(f, p.cLog));
        case OP_DISSOLVE_U:  return so(dissolveUniforme(f, f2, p.t));
        case OP_DISSOLVE_NU: {
            ImagemF pesos = (p.pesos == 0) ? pesosRampaHorizontal(f.largura, f.altura)
                                           : pesosRadial(f.largura, f.altura);
            Resultado r = so(dissolveNaoUniforme(f, f2, pesos));
            r.extras = {{"Pesos t(x,y)", paraUint8(pesos, Faixa::Normalizar)}, {"Resultado", r.principal}};
            return r;
        }
        case OP_EXPANSAO:    return so(expandirHistograma(f));
        case OP_EQUALIZACAO: return so(equalizarHistograma(f));
        case OP_ADAPTATIVO:  return contrasteAdaptativo(f, p.cAdapt, p.nAdapt, b);
        case OP_MEDIA:       return so(filtroMedia(f, p.n, b));
        case OP_MEDIANA:     return so(filtroMediana(f, p.n, b));
        case OP_ROBERTS:     return roberts(f, b, static_cast<Magnitude>(p.magnitude), static_cast<Faixa>(p.faixaBordas));
        case OP_SOBEL:       return sobel(f, b, static_cast<Magnitude>(p.magnitude), static_cast<Faixa>(p.faixaBordas));
        case OP_KIRSCH:      return kirsch(f, b, static_cast<Faixa>(p.faixaBordas));
        case OP_LAPLACIANO:  return laplaciano(f, p.vizinhosLap, b, static_cast<Faixa>(p.faixaLap));
        case OP_AGUC_LAP:    return agucarLaplaciano(f, p.vizinhosLap, b);
        case OP_HIGHBOOST:   return highBoost(f, p.k, p.nHB, b);
        case OP_CONVOLUCAO:
            return so(convolucaoGenerica(f, mascaraDosParametros(p), p.offset, b,
                                         static_cast<Faixa>(p.faixaConv), p.rotacionar));
    }
    return so(f);
}

static bool usaVizinhanca(int op) { return op >= OP_ADAPTATIVO; }

// Slider que só aceita valores ímpares (vizinhanças n x n centradas)
static bool sliderImpar(const char* rotulo, int* v, int minimo, int maximo) {
    bool mudou = ImGui::SliderInt(rotulo, v, minimo, maximo);
    if (*v % 2 == 0) *v += 1;
    return mudou;
}

// Desenha os controles da operação atual. Retorna true se algum parâmetro mudou.
static bool painelParametros(int op, Parametros& p) {
    bool m = false;
    switch (op) {
        case OP_LIMIAR:
            m |= ImGui::SliderInt("T", &p.T, 0, 255);
            ImGui::TextWrapped("s = 255 se r >= T, senao 0");
            break;
        case OP_CONTRASTE:
            m |= ImGui::SliderInt("r1", &p.r1, 0, 255);
            m |= ImGui::SliderInt("s1", &p.s1, 0, 255);
            m |= ImGui::SliderInt("r2", &p.r2, 0, 255);
            m |= ImGui::SliderInt("s2", &p.s2, 0, 255);
            ImGui::TextWrapped("Retas por (0,0), (r1,s1), (r2,s2), (255,255)");
            break;
        case OP_GAMMA:
            m |= ImGui::SliderFloat("c", &p.cGamma, 0.1f, 3.0f);
            m |= ImGui::SliderFloat("gamma", &p.gamma, 0.05f, 6.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
            ImGui::TextWrapped("s = 255 c (r/255)^gamma");
            break;
        case OP_LOG:
            m |= ImGui::SliderFloat("c", &p.cLog, 0.0f, 100.0f);
            ImGui::TextWrapped("s = c ln(1 + r).  c = 0 usa c = 255/ln(256) = 45,99");
            break;
        case OP_DISSOLVE_U:
            m |= ImGui::SliderFloat("t", &p.t, 0.0f, 1.0f);
            ImGui::TextWrapped("g = (1 - t) f1 + t f2.  f1 = imagem de entrada, f2 = segunda imagem");
            break;
        case OP_DISSOLVE_NU:
            m |= ImGui::RadioButton("Rampa horizontal", &p.pesos, 0);
            ImGui::SameLine();
            m |= ImGui::RadioButton("Radial", &p.pesos, 1);
            ImGui::TextWrapped("g = (1 - t(x,y)) f1 + t(x,y) f2");
            break;
        case OP_ADAPTATIVO:
            m |= ImGui::SliderFloat("c", &p.cAdapt, 1.0f, 100.0f);
            m |= sliderImpar("n", &p.nAdapt, 3, 51);
            ImGui::TextWrapped("g = m + (c / sigma)(f - m), com media m e desvio sigma da vizinhanca n x n");
            break;
        case OP_MEDIA:
        case OP_MEDIANA:
            m |= sliderImpar("n", &p.n, 1, 31);
            break;
        case OP_ROBERTS:
        case OP_SOBEL:
        case OP_KIRSCH:
            if (op != OP_KIRSCH) {
                m |= ImGui::RadioButton("sqrt(Gx^2 + Gy^2)", &p.magnitude, 0);
                ImGui::SameLine();
                m |= ImGui::RadioButton("|Gx| + |Gy|", &p.magnitude, 1);
            }
            m |= ImGui::Combo("Faixa da resposta", &p.faixaBordas, NOMES_FAIXA, 3);
            if (op != OP_KIRSCH) ImGui::TextWrapped("Gx e Gy sao exibidos normalizados (cinza medio = 0).");
            break;
        case OP_LAPLACIANO:
        case OP_AGUC_LAP:
            m |= ImGui::RadioButton("4 vizinhos", &p.vizinhosLap, 4);
            ImGui::SameLine();
            m |= ImGui::RadioButton("8 vizinhos", &p.vizinhosLap, 8);
            if (op == OP_LAPLACIANO) m |= ImGui::Combo("Faixa", &p.faixaLap, NOMES_FAIXA, 3);
            else ImGui::TextWrapped("g = f - Laplaciano(f)  (centro da mascara negativo)");
            break;
        case OP_HIGHBOOST:
            m |= ImGui::SliderFloat("k", &p.k, 0.0f, 10.0f);
            m |= sliderImpar("n (media)", &p.nHB, 3, 31);
            ImGui::TextWrapped("g = f + k (f - media).  k = 1: unsharp masking; k > 1: high boost");
            break;
        case OP_CONVOLUCAO: {
            ImGui::TextWrapped("g = f * h + offset");
            m |= ImGui::SliderInt("linhas (n)", &p.linhas, 1, MAX_MASCARA);
            m |= ImGui::SliderInt("colunas (m)", &p.colunas, 1, MAX_MASCARA);
            float largura = std::min(60.0f, (ImGui::GetContentRegionAvail().x - 8 * p.colunas) / p.colunas);
            for (int i = 0; i < p.linhas; ++i) {
                for (int j = 0; j < p.colunas; ++j) {
                    if (j) ImGui::SameLine();
                    ImGui::PushID(i * MAX_MASCARA + j);
                    ImGui::SetNextItemWidth(largura);
                    m |= ImGui::InputFloat("##h", &p.mascara[i * MAX_MASCARA + j], 0, 0, "%g");
                    ImGui::PopID();
                }
            }
            ImGui::Text("Soma dos coeficientes: %g", mascaraDosParametros(p).soma());
            m |= ImGui::SliderFloat("offset", &p.offset, -255.0f, 255.0f, "%.0f");
            m |= ImGui::Combo("Faixa", &p.faixaConv, NOMES_FAIXA, 3);
            m |= ImGui::Checkbox("Rotacionar 180 graus (convolucao verdadeira)", &p.rotacionar);

            ImGui::SeparatorText("Mascaras prontas");
            ImGui::SetNextItemWidth(110);
            ImGui::SliderInt("c##teste", &p.cTeste, 1, 5);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(110);
            ImGui::SliderInt("d##teste", &p.dTeste, 1, 10);
            ImGui::SameLine();
            if (ImGui::Button("2.1")) { carregarMascara(p, mascaras::agucamentoCD(p.cTeste, p.dTeste)); m = true; }
            if (ImGui::Button("h1")) { carregarMascara(p, mascaras::relevoH1()); p.offset = 128; m = true; }
            ImGui::SameLine();
            if (ImGui::Button("h2")) { carregarMascara(p, mascaras::relevoH2()); p.offset = 128; m = true; }
            ImGui::SameLine();
            if (ImGui::Button("h3")) { carregarMascara(p, mascaras::bordaH3()); p.offset = 0; m = true; }
            ImGui::SameLine();
            if (ImGui::Button("h4")) { carregarMascara(p, mascaras::bordaH4()); p.offset = 0; m = true; }
            if (ImGui::Button("Identidade")) { carregarMascara(p, Mascara(1, 1, {1})); m = true; }
            ImGui::SameLine();
            if (ImGui::Button("Media 3x3")) { carregarMascara(p, mascaras::media(3)); m = true; }
            ImGui::SameLine();
            if (ImGui::Button("Sobel X")) { carregarMascara(p, mascaras::sobelX()); m = true; }
            ImGui::SameLine();
            if (ImGui::Button("Laplaciano")) { carregarMascara(p, mascaras::laplaciano4()); m = true; }
            break;
        }
        default:
            ImGui::TextDisabled("(sem parametros)");
    }
    if (usaVizinhanca(op)) m |= ImGui::Combo("Borda", &p.borda, NOMES_BORDA, 4);
    return m;
}

// ===========================================================================
// Programa principal
// ===========================================================================

static std::vector<std::string> listarImagens(const fs::path& pasta) {
    std::vector<std::string> nomes;
    std::error_code erro;
    for (const auto& e : fs::directory_iterator(pasta, erro))
        if (e.is_regular_file()) nomes.push_back(e.path().filename().string());
    std::sort(nomes.begin(), nomes.end());
    return nomes;
}

// Combo com os arquivos de imagens/entrada; retorna true quando o usuário escolhe um
static bool comboArquivo(const char* rotulo, const std::vector<std::string>& nomes, int& escolhido) {
    bool mudou = false;
    const char* atual = (escolhido >= 0 && escolhido < static_cast<int>(nomes.size())) ? nomes[escolhido].c_str() : "(escolha)";
    if (ImGui::BeginCombo(rotulo, atual)) {
        for (int i = 0; i < static_cast<int>(nomes.size()); ++i) {
            if (ImGui::Selectable(nomes[i].c_str(), i == escolhido)) { escolhido = i; mudou = true; }
        }
        ImGui::EndCombo();
    }
    return mudou;
}

static std::string nomeArquivoSeguro(std::string s) {
    for (char& c : s) if (!std::isalnum(static_cast<unsigned char>(c))) c = '_';
    return s;
}

int main() {
    if (!glfwInit()) {
        std::fprintf(stderr, "Falha ao iniciar o GLFW\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* janela = glfwCreateWindow(1500, 920, "Cinzel - Processamento de Imagens", nullptr, nullptr);
    if (!janela) {
        std::fprintf(stderr, "Falha ao criar a janela\n");
        return 1;
    }
    glfwMakeContextCurrent(janela);
    glfwSwapInterval(1);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;  // não criar imgui.ini
    const char* fonte = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    if (fs::exists(fonte)) io.Fonts->AddFontFromFileTTF(fonte, 16.0f);
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(janela, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    const fs::path pastaEntrada = fs::path(CINZEL_RAIZ) / "imagens" / "entrada";
    const fs::path pastaSaida = fs::path(CINZEL_RAIZ) / "imagens" / "saida" / "gui";
    std::vector<std::string> arquivos = listarImagens(pastaEntrada);

    Visualizacao original{"Original"}, segunda{"Segunda imagem"}, resultado{"Resultado"};
    std::vector<Visualizacao> extras;
    int arqEntrada = -1, arqSegunda = -1;
    int op = OP_NEGATIVO;
    Parametros p;
    bool automatico = true;
    float zoom = 1.0f, zoomExtras = 0.5f;
    std::string mensagem;

    // Começa com a cameraman, se existir
    for (int i = 0; i < static_cast<int>(arquivos.size()); ++i)
        if (arquivos[i] == "cameraman.tif") arqEntrada = i;

    auto abrir = [&](int indice, Visualizacao& destino) {
        Imagem img;
        if (indice >= 0 && carregar((pastaEntrada / arquivos[indice]).string(), img)) {
            destino.atualizar(img);
            mensagem.clear();
        } else {
            mensagem = "Nao consegui abrir a imagem.";
        }
    };
    auto aplicar = [&]() {
        if (!original.pronta()) return;
        try {
            Resultado r = executar(op, original.img, segunda.img, p);
            resultado.atualizar(r.principal);
            for (auto& e : extras) e.liberar();
            extras.clear();
            for (auto& [nome, img] : r.extras) {
                extras.push_back({nome});
                extras.back().atualizar(img);
            }
            mensagem.clear();
        } catch (const std::exception& e) {
            mensagem = std::string("Erro: ") + e.what();
        }
    };

    if (arqEntrada >= 0) { abrir(arqEntrada, original); aplicar(); }

    while (!glfwWindowShouldClose(janela)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::Begin("Cinzel", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoBringToFrontOnFocus);

        // ------------------------- Painel de controles -------------------------
        ImGui::BeginChild("controles", ImVec2(400, 0), ImGuiChildFlags_Borders);
        bool mudou = false;

        ImGui::SeparatorText("Imagem de entrada");
        if (comboArquivo("Arquivo", arquivos, arqEntrada)) { abrir(arqEntrada, original); mudou = true; }
        if (original.pronta()) ImGui::TextDisabled("%d x %d pixels", original.img.largura, original.img.altura);

        if (op == OP_DISSOLVE_U || op == OP_DISSOLVE_NU) {
            if (comboArquivo("Segunda", arquivos, arqSegunda)) { abrir(arqSegunda, segunda); mudou = true; }
            if (segunda.pronta()) ImGui::TextDisabled("%d x %d pixels (precisa ser igual)", segunda.img.largura, segunda.img.altura);
        }

        ImGui::SeparatorText("Operacao");
        ImGui::SetNextItemWidth(-1);
        if (ImGui::Combo("##op", &op, NOMES_OP, OP_TOTAL, 20)) mudou = true;

        ImGui::SeparatorText("Parametros");
        mudou |= painelParametros(op, p);

        ImGui::SeparatorText("Executar");
        ImGui::Checkbox("Aplicar automaticamente", &automatico);
        if (ImGui::Button("Aplicar", ImVec2(-1, 0)) || (automatico && mudou)) aplicar();
        if (ImGui::Button("Usar resultado como entrada", ImVec2(-1, 0)) && resultado.pronta()) {
            original.atualizar(resultado.img);
            aplicar();
        }
        if (ImGui::Button("Salvar resultado e respostas", ImVec2(-1, 0)) && resultado.pronta()) {
            fs::create_directories(pastaSaida);
            std::string base = nomeArquivoSeguro(NOMES_OP[op]);
            salvar((pastaSaida / (base + ".png")).string(), resultado.img);
            for (auto& e : extras)
                salvar((pastaSaida / (base + "_" + nomeArquivoSeguro(e.nome) + ".png")).string(), e.img);
            mensagem = "Salvo em imagens/saida/gui/" + base + "*.png";
        }
        if (!mensagem.empty()) ImGui::TextColored(ImVec4(1, 0.8f, 0.3f, 1), "%s", mensagem.c_str());

        ImGui::SeparatorText("Visualizacao");
        ImGui::SliderFloat("Zoom", &zoom, 0.25f, 4.0f, "%.2fx (1 = ajustar)");
        ImGui::SliderFloat("Zoom extras", &zoomExtras, 0.1f, 2.0f, "%.2fx");
        ImGui::TextWrapped("Passe o mouse sobre uma imagem para ver o valor do pixel e a vizinhanca 3x3.");
        ImGui::EndChild();

        ImGui::SameLine();

        // ------------------------- Área das imagens -------------------------
        ImGui::BeginChild("imagens", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
        float coluna = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2.0f;
        auto escalaPara = [&](const Visualizacao& v) {
            return v.pronta() ? zoom * coluna / v.img.largura : 1.0f;  // zoom 1 = ajustar à coluna
        };

        ImGui::BeginGroup();
        ImGui::Text("Original");
        mostrarImagem(original, escalaPara(original));
        mostrarHistograma("##hist_orig", original.img, original.img.largura * escalaPara(original));
        ImGui::EndGroup();
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("Resultado: %s", NOMES_OP[op]);
        mostrarImagem(resultado, escalaPara(resultado));
        mostrarHistograma("##hist_res", resultado.img, resultado.img.largura * escalaPara(resultado));
        ImGui::EndGroup();

        if (!extras.empty()) {
            ImGui::SeparatorText("Respostas intermediarias");
            float disponivel = ImGui::GetContentRegionAvail().x;
            float usado = 0.0f;
            for (size_t i = 0; i < extras.size(); ++i) {
                float largura = extras[i].img.largura * zoomExtras;
                if (i > 0 && usado + largura < disponivel) ImGui::SameLine();
                else usado = 0.0f;
                usado += largura + ImGui::GetStyle().ItemSpacing.x;
                ImGui::BeginGroup();
                ImGui::TextUnformatted(extras[i].nome.c_str());
                mostrarImagem(extras[i], zoomExtras);
                ImGui::EndGroup();
            }
        }
        if (op == OP_DISSOLVE_U || op == OP_DISSOLVE_NU) {
            ImGui::SeparatorText("Segunda imagem (f2)");
            mostrarImagem(segunda, zoomExtras);
        }
        ImGui::EndChild();
        ImGui::End();

        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(janela, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(janela);
    }

    original.liberar();
    segunda.liberar();
    resultado.liberar();
    for (auto& e : extras) e.liberar();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(janela);
    glfwTerminate();
    return 0;
}
