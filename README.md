# Cinzel

Sistema de processamento de imagens em escala de cinza (8 bits/pixel) em C++17, desenvolvido
para a 1ª Avaliação de Processamento de Imagens (UNIR/FAETI, 2026).

Implementa, sem bibliotecas de processamento: dissolve cruzado, transformações de intensidade,
expansão e equalização de histograma, contraste adaptativo, filtros da média e da mediana,
detecção de bordas (Roberts, Sobel, Kirsch, Laplaciano), aguçamento (Laplaciano e high boost)
e convolução genérica.

## Vídeo de explicação

Apresentação e explicação do projeto: [LINK PARA O YOUTUBE](https://youtu.be/eIo5EQS-ExA)

## Dependências (Ubuntu)

```bash
sudo apt install build-essential cmake libglfw3-dev libgl1-mesa-dev libtiff-dev
```

O Dear ImGui é baixado automaticamente pelo CMake. O stb_image (PNG/JPG/BMP) fica em `external/stb/`;
a libtiff lê os arquivos TIFF.

## Compilar e executar

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

./build/cinzel_testes      # testes unitários
./build/cinzel_relatorio   # gera as imagens do relatório em imagens/saida/
./build/cinzel             # interface gráfica
```

## Estrutura

| Pasta | Conteúdo |
|---|---|
| `src/core/` | Todos os algoritmos (sem dependência de interface) |
| `src/gui/` | Interface Dear ImGui |
| `src/testes/` | Testes unitários e gerador das imagens do relatório |
| `imagens/entrada/` | Imagens de teste |
| `imagens/saida/` | Resultados gerados (`RESUMO.md` descreve cada arquivo) |
| `docs/planejamento/` | Planejamento, requisitos e arquitetura |
| `relatorio/` | Relatório em LaTeX |

Detalhes da implementação e de como rodar os testes: [GUIA_DO_SISTEMA.md](GUIA_DO_SISTEMA.md).
