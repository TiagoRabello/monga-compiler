1. Pré-requisitos:
  * make
  * flex  (ou equivalente que deve ser definido no Makefile como LEX)
  * bison (ou equivalente que deve ser definido no Makefile como YACC)
  * gcc   (ou equivalente que deve ser definido no Makefile como CC)
  * diff  (ou equivalente que deve ser definido no Makefile como DIFF)

2. Comandos do Makefile:
  * compile src=<path> : Gera um executável apartir de um código fonte Monga.
  * compiler           : Gera o programa utilizado para gerar assembly apartir de códigos Monga.
  * test               : Roda todos os testes do projeto.
  * test_backend       : Roda todos os testes referentes ao gerador de código.
  * test_scanner       : Roda todos os testes referentes ao analisador léxico.
  * test_parser        : Roda todos os testes referentes ao analisador sintático.
  * test_semantic      : Roda todos os testes referentes ao analisador semântico.
  * backend_test       : Gera o programa de testes do gerador de código.
  * scanner_test       : Gera o programa de testes do analisador léxico.
  * parser_test        : Gera o programa de testes do analisador sintático.
  * semantic_test      : Gera o programa de testes do analisador semântico.
  * clean              : Remove todos os arquivos gerados pelo make.

3. Estrutura do projeto:
  * bin/          : Diretório onde são colocados todos os arquivos binários produzidos pelo make.
  * src/ast/      : Diretório onde ficam todos os fontes referentes a árvore de sintaxe abstrata.
  * src/backend/  : Diretório onde ficam todos os fontes referentes a geração de código.
  * src/common/   : Diretório onde ficam fontes com funcionalidades genéricas.
  * src/scanner/  : Diretório onde ficam todos os fontes referentes ao analisador léxico.
  * src/parser/   : Diretório onde ficam todos os fontes referentes ao analisador sintático.
  * src/runtime/  : Diretório onde ficam fontes com funções C utilizadas por programas monga.
  * src/semantic/ : Diretório onde ficam todos os fontes referentes ao analisador semântico.
  * test/answers/ : Diretório onde ficam as saídas corretas para os testes.
  * test/inputs/  : Diretório onde ficam as entradas para os testes.

4. Testando Entradas Manualmente
  Para testar manualmente algum dos analisadores basta gerar seu respectivo programa de teste e passar o arquivo monga como entrada.
  Ex:
    > make scanner_test
    > bin/scanner_test < test/input/fibonacci.monga
    > make parser_test
    > bin/parser_test < test/input/fibonacci.monga
    > make semantic_test
    > bin/semantic_test < test/input/fibonacci.monga
    > make backend_test
    > bin/backend_test < test/input/fibonacci.monga

5. Execução de um Programa Monga
  A execução de um programa Monga começa com a chamada da função monga_main, esta função deve possuir a seguinte assinatura: int monga_main().

  Atualmente a única forma de comunicação de um programa Monga com o mundo exterior é através do retorno desta função.

6. Compilando um Programa Monga
  Para compilar um programa Monga basta utilizar o target compile do Makefile presente no projeto.
  Ex:
    > make compile src=test/input/fibonacci.monga && ./out.exe

  Atualmente só é possível compilar programas que possuam a função monga_main (vide seção 5) declarada.