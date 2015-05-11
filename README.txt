1. Pré-requisitos:
  * make
  * flex  (ou equivalente que deve ser definido no Makefile como LEX)
  * bison (ou equivalente que deve ser definido no Makefile como YACC)
  * gcc   (ou equivalente que deve ser definido no Makefile como CC)
  * diff  (ou equivalente que deve ser definido no Makefile como DIFF)

2. Comandos do Makefile:
  * test          : Roda todos os testes do projeto.
  * test_scanner  : Roda todos os testes referentes ao analisador léxico.
  * test_parser   : Roda todos os testes referentes ao analisador sintático.
  * monga_scanner : Gera o arquivo de objeto referente ao analisador léxico.
  * clean         : Remove todos os arquivos gerados pelo make.

3. Estrutura do projeto:
  * bin/          : Diretório onde são colocados todos os arquivos binários produzidos pelo make.
  * src/ast/      : Diretório onde ficam todos os fontes referentes a árvore de sintaxe abstrata.
  * src/common/   : Diretório onde ficam fontes com funcionalidades genéricas.
  * src/scanner/  : Diretório onde ficam todos os fontes referentes ao analisador léxico.
  * src/parser/   : Diretório onde ficam todos os fontes referentes ao analisador sintático.
  * test/answers/ : Diretório onde ficam as saídas corretas para os testes.
  * test/inputs/  : Diretório onde ficam as entradas para os testes.
