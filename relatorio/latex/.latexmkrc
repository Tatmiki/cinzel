# Configuração do latexmk para este relatório.
# Uso (dentro de relatorio/latex/):  latexmk
#   - o PDF fica nesta pasta;
#   - os arquivos auxiliares (.aux, .log, .toc...) vão para build/.
$pdf_mode = 1;              # gerar PDF com pdflatex
$aux_dir = 'build';
$out_dir = '.';
$emulate_aux = 1;
@default_files = ('relatorio_projeto_cinzel.tex');
