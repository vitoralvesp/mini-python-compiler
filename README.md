# MiniPython Compiler Project
Projeto desenvolvido por Anna Luíza Stella Santos (10417401) e Vitor Alves Pereira (10410862).

## Sobre
**MiniPython Compiler** é um projeto desenvolvido para compilar programas desenvolvidos em MiniPython.

Para compilar:

`gcc -Wall -Wno-unused-result -g -Og src/miniPythonCompiler.c -o temp/app`

Para executar
```
EXEMPLOS:
temp/app input/example-1.txt
temp/app input/example-2.txt
temp/app input/example-3.txt

ERROS LÉXICOS:
temp/app input/error/lexical/lexical-errors-1.txt
temp/app input/error/lexical/lexical-errors-2.txt
temp/app input/error/lexical/lexical-errors-3.txt
temp/app input/error/lexical/lexical-errors-4.txt
temp/app input/error/lexical/lexical-errors-5.txt
temp/app input/error/lexical/lexical-errors-6.txt
temp/app input/error/lexical/lexical-errors-7.txt
temp/app input/error/lexical/lexical-errors-8.txt

ERROS SINTÁTICOS
temp/app input/error/sintatic/sintatic-errors-1.txt
temp/app input/error/sintatic/sintatic-errors-2.txt

```
