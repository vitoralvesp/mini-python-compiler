# MiniPython Compiler Project
* Anna Luíza Stella Santos, 10417401
* Vitor Alves Pereira, 10410862

## Sobre
Este é um projeto desenvolvido para atuar como um compilador de programas desenvolvidos em MiniPython. Inicialmente, implementamos apenas as análises léxica e sintática, e esse processo de análise é iniciado após o usuário compilar o projeto com o comando a seguir:

Para compilar:

`gcc -Wall -Wno-unused-result -g -Og src/miniPythonCompiler.c -o temp/app`

Para executar
`
temp/app input/error/lexical/lexical-errors-1.txt
temp/app input/error/lexical/lexical-errors-2.txt
temp/app input/error/lexical/lexical-errors-3.txt
temp/app input/error/lexical/lexical-errors-4.txt
temp/app input/error/lexical/lexical-errors-5.txt
temp/app input/error/lexical/lexical-errors-6.txt
temp/app input/error/lexical/lexical-errors-7.txt
temp/app input/error/lexical/lexical-errors-8.txt

temp/app input/error/sintatic/sintatic-errors-1.txt
temp/app input/error/sintatic/sintatic-errors-2.txt`
