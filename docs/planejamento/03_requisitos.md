# Requisitos

Lista dos requisitos tirados da especificação (`relatorio/especificacoes_projeto.pdf`).
Use os checkboxes para acompanhar o progresso.

**Notação:** 
- $f$ = imagem de entrada;
- $g$ = imagem de saída;
- $r$ = valor do pixel de entrada;
- $s$ = valor do pixel de saída;
- $L = 256$ níveis de cinza (valores de $0$ a $L-1 = 255$).

---

## 1. Requisitos gerais

- [ ] Processar imagens em **escala de cinza, 8 bits/pixel**: $f(x,y) \in \{0, \dots, 255\}$.
- [ ] Todos os algoritmos avaliados implementados.
- [ ] Bibliotecas só para: leitura, escrita, exibição e interface.
- [ ] Abrir imagem de arquivo (se colorida, converter para cinza na leitura).
- [ ] Salvar a imagem resultante.
- [ ] Mostrar imagem original e resultado lado a lado.
- [ ] Permitir usar o resultado como nova entrada (encadear operações).

## 2. Operações algébricas

- [ ] **Dissolve cruzado uniforme**: $t \in [0,1]$ igual para todos os pixels.

$$
g(x,y) = (1 - t)\, f_1(x,y) + t\, f_2(x,y)
$$

- [ ] **Dissolve cruzado não uniforme**: o peso varia por pixel, $t(x,y)$ (ex.: rampa horizontal,
      radial ou uma terceira imagem usada como máscara de pesos).

$$
g(x,y) = \big(1 - t(x,y)\big)\, f_1(x,y) + t(x,y)\, f_2(x,y)
$$

- [ ] Carregar uma **segunda imagem** e tratar tamanhos diferentes (exigir mesmo tamanho ou
      avisar o usuário).

## 3. Transformações de intensidade (pontuais)

- [ ] **Negativo**: $s = 255 - r$.
- [ ] **Alargamento de contraste**: função linear por partes com pontos $(r_1,s_1)$ e $(r_2,s_2)$ parametrizáveis.
- [ ] **Limiarização** com $T$ parametrizável:

$$
s = \begin{cases} 255, & r \ge T \\ 0, & r < T \end{cases}
$$

- [ ] **Potência (gamma)**, com $c$ e $\gamma$ parametrizáveis e $r$ normalizado em $[0,1]$:

$$
s = 255 \cdot c \left(\frac{r}{255}\right)^{\gamma}
$$

- [ ] **Logaritmo**, com $c$ parametrizável (ou automático, de modo que $255 \mapsto 255$):

$$
s = c \cdot \log(1 + r), \qquad c_{\text{auto}} = \frac{255}{\log(256)}
$$

## 4. Histograma

- [ ] Calcular e **exibir** o histograma da imagem original e do resultado:
      $h(k) = \#\{(x,y) : f(x,y) = k\}$.
- [ ] **Expansão** de histograma:

$$
s = \frac{r - r_{\min}}{r_{\max} - r_{\min}} \cdot 255
$$

- [ ] **Equalização** de histograma ($N$ = total de pixels):

$$
s_k = \operatorname{round}\!\left( 255 \sum_{j=0}^{k} \frac{h(j)}{N} \right)
$$

- [ ] Tratar imagem constante ($r_{\max} = r_{\min}$) sem divisão por zero.

## 5. Contraste adaptativo

- [ ] Constante **$c$** parametrizável.
- [ ] Vizinhança **$n \times n$** parametrizável.
- [ ] Usar média $m(x,y)$ e desvio padrão $\sigma(x,y)$ **locais** da vizinhança.
- [ ] Tratar $\sigma = 0$ (região uniforme).

## 6. Filtragem espacial

- [ ] **Média** com vizinhança $n \times n$ parametrizável.
- [ ] **Mediana** com vizinhança $n \times n$ parametrizável.

## 7. Detecção de bordas

- [ ] **Roberts**: mostrar $G_x$, $G_y$ e magnitude.
- [ ] **Sobel**: mostrar $G_x$, $G_y$ e magnitude.
- [ ] **Kirsch**: mostrar as **8 respostas** direcionais e a resposta final (máximo).
- [ ] **Laplaciano**: mostrar a resposta (máscara de 4 ou 8 vizinhos).
- [ ] Para operadores direcionais: exibir as respostas de **cada máscara** + a resposta final.
- [ ] Magnitude (com opção da aproximação):

$$
\lvert \nabla f \rvert = \sqrt{G_x^2 + G_y^2} \;\approx\; \lvert G_x \rvert + \lvert G_y \rvert
$$

## 8. Aguçamento

- [ ] **Aguçamento pelo Laplaciano**, com $c = -1$ se o centro da máscara for negativo:

$$
g(x,y) = f(x,y) + c\, \nabla^2 f(x,y)
$$

- [ ] **High boost**, com $k$ parametrizável ($k = 1$ → unsharp masking) e $\bar f$ a imagem suavizada:

$$
g(x,y) = f(x,y) + k\,\big(f(x,y) - \bar f(x,y)\big)
$$

- [ ] Visualizar **separadamente** a resposta do Laplaciano e a imagem final combinada.

## 9. Convolução genérica

- [ ] Convolução com offset:

$$
g = f * h + \text{offset}
$$

- [ ] Máscara **$n \times m$** arbitrária, digitada pelo usuário.
- [ ] Offset parametrizável.
- [ ] **Tratamento de bordas explícito** e selecionável (zeros, replicar, refletir…).
- [ ] **Tratamento de valores fora de $[0,255]$ explícito** e selecionável (saturação,
      normalização, valor absoluto).
- [ ] Decidir e documentar: convolução verdadeira (máscara rotacionada 180°) × correlação.

## 10. Testes obrigatórios de convolução (para o relatório)

Para cada caso: imagem original, imagem resultante e interpretação.

- [ ] **2.1 Aguçamento**: vários $c, d$ inteiros positivos, **obrigatoriamente $c = d = 1$**.
      Explicar $c$, $d$ e a soma dos coeficientes.

$$
h = \begin{bmatrix} -c & -c & -c \\ -c & 8c + d & -c \\ -c & -c & -c \end{bmatrix}
$$

- [ ] **2.2 Relevo**: comparar e explicar o efeito da orientação.

$$
h_1 = \begin{bmatrix} 0 & 0 & 0 \\ 0 & 1 & 0 \\ 0 & 0 & -1 \end{bmatrix}
\qquad
h_2 = \begin{bmatrix} 0 & 0 & 2 \\ 0 & -1 & 0 \\ -1 & 0 & 0 \end{bmatrix}
$$

- [ ] **2.3 Bordas**: analisar sinais e disposição espacial.

$$
h_3 = \begin{bmatrix} -1 & -1 & 0 \\ 1 & 0 & 1 \\ 0 & 1 & 1 \end{bmatrix}
\qquad
h_4 = \begin{bmatrix} 0 & -1 & 0 \\ -1 & -4 & -1 \\ 0 & -1 & 0 \end{bmatrix}
$$

- [ ] Ter atalhos (presets) dessas máscaras na interface.

## 11. Requisitos não funcionais

- [ ] **Legibilidade**: funções curtas, nomes claros, um algoritmo por função.
- [ ] **Modificabilidade**: estratégia de borda, tamanho de vizinhança e máscaras fáceis de trocar (o desafio prático cobra exatamente isso).
- [ ] **Separação núcleo vs. interface**: algoritmos não conhecem a GUI.
- [ ] **Resultados intermediários preservados** em ponto flutuante (sem perder negativos).
- [ ] README de build/instalação.
- [ ] Executável de testes com matrizes pequenas, para comparar com cálculo manual.

## 12. Preparação para arguição e desafio (requisitos de estudo)

- [ ] Saber calcular à mão, numa matriz $3\times3$ ou $5\times5$: média, mediana, convolução, Sobel, Laplaciano, Kirsch, equalização.
- [ ] Responder as 10 questões conceituais da especificação (seção 3).
- [ ] Conseguir trocar ao vivo: tamanho da vizinhança, estratégia de borda, máscara, forma da vizinhança (ex.: cruz em vez de quadrado).
- [ ] Saber prever o efeito de uma máscara desconhecida olhando a **soma dos coeficientes** $\sum_{i,j} h(i,j)$ e a **disposição dos sinais**.
