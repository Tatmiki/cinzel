# Planejamento do Projeto - Cinzel

> **Cinzel**: sistema de processamento de imagens em escala de cinza (8 bits/pixel)
> desenvolvido para a 1ª Avaliação de Processamento de Imagens — UNIR/FAETI, 2026.

## 1. Objetivo

Construir um sistema que implemente **do zero** os algoritmos da Unidade 01
(operações pontuais, histograma, filtragem espacial, convolução, bordas e aguçamento),
com uma interface simples para aplicar as operações, ajustar parâmetros e visualizar
os resultados intermediários.

O objetivo real da avaliação **não é só o sistema funcionar**: é conseguir **explicar,
modificar e justificar** cada linha durante a arguição e o desafio prático. Isso guia
todas as decisões deste planejamento:

- Código **simples e legível** acima de código "esperto" ou otimizado.
- Os algoritmos ficam isolados da interface (o professor vai pedir para mexer no algoritmo,
  não na GUI).
- Os pontos que o professor já avisou que vai cobrar (tamanho da vizinhança, tratamento de
  bordas, máscaras, respostas intermediárias) devem ser **fáceis de alterar**.

## 2. Composição da nota

| Componente                                | Peso | Implicação para o planejamento                         |
|-------------------------------------------|------|--------------------------------------------------------|
| Sistema, código-fonte, testes e relatório | 40%  | Implementar tudo + gerar imagens de teste + relatório  |
| Arguição                                  | 30%  | Estudar a teoria de cada algoritmo e saber fazer à mão |
| Desafio prático                           | 30%  | Código modular, fácil de modificar ao vivo             |

$$
N = 0{,}40\,S + 0{,}30\,A + 0{,}30\,D
$$

onde $S$ = sistema/relatório, $A$ = arguição e $D$ = desafio prático. A nota de $S$ só é
considerada integralmente se

$$
\frac{A + D}{2} \geq 50\%
$$


## 3. Escopo

### Dentro do escopo
- Leitura/escrita de imagens (PNG, JPG, BMP, TIF, etc.), convertidas para cinza 8 bits.
- Todas as operações listadas em [03_requisitos.md](03_requisitos.md).
- Interface gráfica simples (painel de controles + visualização de imagens + histograma).
- Executável de testes com matrizes pequenas (para conferir resultados de cálculos manuais).
- Geração das imagens para o relatório (testes obrigatórios de convolução).
- Relatório em PDF.
- Repositório no GitHub.

## 4. Fases e cronograma sugerido

O cronograma é relativo (em "blocos" de trabalho). Ajustar conforme a data de entrega.

| Fase | Entrega                                                         | Esforço  |
|------|-----------------------------------------------------------------|----------|
| 0    | Ambiente: CMake, repositório Git, estrutura de pastas, stb      | 0,5 bloco |
| 1    | Núcleo: tipo `Imagem`, leitura/escrita, `ImagemF`, conversões   | 1 bloco  |
| 2    | Operações pontuais: negativo, limiar, contraste, gamma, log     | 1 bloco  |
| 3    | Operações algébricas: dissolve uniforme e não uniforme          | 0,5 bloco |
| 4    | Histograma: cálculo, expansão, equalização                      | 1 bloco  |
| 5    | Vizinhança + bordas: acesso com estratégia de borda             | 0,5 bloco |
| 6    | Convolução genérica (n×m, offset, bordas, faixa [0,255])        | 1 bloco  |
| 7    | Filtros média e mediana                                         | 0,5 bloco |
| 8    | Contraste adaptativo                                            | 0,5 bloco |
| 9    | Bordas: Roberts, Sobel, Kirsch, Laplaciano (+ intermediárias)   | 1,5 bloco |
| 10   | Aguçamento: Laplaciano e high boost                             | 0,5 bloco |
| 11   | Interface gráfica (ImGui)                                       | 1,5 bloco |
| 12   | Testes obrigatórios de convolução + geração de imagens          | 1 bloco  |
| 13   | Relatório (teoria, métodos, resultados, discussão)              | 2 blocos |
| 14   | Estudo para arguição: questões conceituais + cálculos à mão     | contínuo |

### Marcos (checkpoints)
1. **M1:** abrir uma imagem, aplicar o negativo e salvar (fases 0–2).
2. **M2:** convolução genérica funcionando e testada à mão (fases 5–6).
3. **M3:** todos os algoritmos implementados (fases 7–10).
4. **M4:** GUI completa + imagens dos testes obrigatórios geradas (fases 11–12).
5. **M5:** relatório finalizado + revisão das 10 questões conceituais (fases 13–14).

## 5. Entregáveis

- [ ] Código-fonte completo no GitHub (com README de compilação).
- [ ] Relatório PDF: introdução, fundamentação teórica, materiais e métodos, resultados, discussão e conclusão.
- [ ] Pasta com imagens de entrada usadas e resultados produzidos.

## 6. Documentos relacionados

- [02_tecnologias.md](02_tecnologias.md): stack e justificativas.
- [03_requisitos.md](03_requisitos.md): lista de requisitos funcionais e não funcionais.
- [04_arquitetura.md](04_arquitetura.md): estrutura de pastas, módulos e decisões técnicas.
