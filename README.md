## Compilador de MiniPython
Projeto desenvolvido para realizar a **análise léxica** e **análise sintática** de programas em **MiniPython**.  

#### Para compilar
`gcc -Wall -Wno-unused-result -g -Og src/miniPythonCompiler.c -o temp/app`  
 
#### Para executar:
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
