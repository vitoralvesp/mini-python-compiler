================================================================================
PROJETO: COMPILADOR MINIPYTHON (FASE 1 - LÉXICO E SINTÁTICO)
DISCIPLINA: COMPILADORES
================================================================================

1. INTEGRANTES DO GRUPO
--------------------------------------------------------------------------------
- Anna Luiza Stella Santos - RA: 10417401
- Vitor Alves Pereira - RA: 10410862

2. STATUS DO PROJETO
--------------------------------------------------------------------------------
Todas as etapas solicitadas para a Fase 1 foram concluídas com sucesso:
- ETAPA #1: Definição de Expressões Regulares e Autômatos Finitos (JFLAP).
- ETAPA #2: Implementação do Analisador Léxico (obter_atomo).
- ETAPA #3: Implementação do Analisador Sintático (consome e gramática).

O compilador é capaz de processar arquivos fonte em MiniPython, identificar 
lexemas, validar a sintaxe e reportar erros detalhados.

3. COMO EXECUTAR O CÓDIGO
--------------------------------------------------------------------------------
Para compilar o projeto, utilize o compilador GCC (conforme especificado no 
enunciado) com o seguinte comando no terminal:

    gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador

Para executar o compilador em um arquivo fonte MiniPython:

    ./compilador [nome_do_arquivo].txt

Exemplo:
    ./compilador exemplo-1.txt

4. DECISÕES DE DESIGN E IMPLEMENTAÇÃO
--------------------------------------------------------------------------------
- LINGUAGEM: O projeto foi desenvolvido integralmente em linguagem C.
- ANALISADOR LÉXICO: Implementado utilizando a técnica de Autômatos Finitos 
  através de uma máquina de estados com rótulos (goto), garantindo eficiência 
  na captura de átomos como Identificadores, Números, Operadores e Delimitadores.
- ANALISADOR SINTÁTICO: Foi utilizada a técnica de Análise Descendente 
  Recursiva (Top-Down), onde cada regra da gramática MiniPython (If, While, 
  For, Print, Atribuição) possui sua própria função de validação.
- INTERAÇÃO LÉXICO-SINTÁTICO: O sintático atua como o driver principal, 
  chamando a função 'consome()' que, por sua vez, solicita o próximo token à 
  função 'obter_atomo()'.
- CASE SENSITIVITY: O compilador diferencia letras maiúsculas de minúsculas 
  em identificadores, conforme exigido.
- TRATAMENTO DE ERROS: Ao encontrar um erro léxico ou sintático, o compilador 
  exibe a linha exata do erro, o token problemático e encerra o processo 
  imediatamente (exit 1).

5. BUGS CONHECIDOS E OBSERVAÇÕES
--------------------------------------------------------------------------------
- INDENTAÇÃO: Conforme as restrições da página 2 do PDF, este compilador 
  ignora a indentação do Python original. Blocos de código (If/While/For) 
  aceitam apenas um comando simples por estrutura.
- ESPAÇOS: O analisador léxico utiliza espaços em branco como delimitadores 
  naturais, portanto, tokens devem estar devidamente separados para correta 
  identificação.
- Não foram identificados bugs críticos nos exemplos de teste fornecidos.

6. ESTRUTURA DE ARQUIVOS
--------------------------------------------------------------------------------
- compilador.c          : Código fonte principal com Léxico e Sintático.
- automato_completo.jff : Arquivo JFLAP com o autômato único da linguagem.
- Readme.txt            : Este arquivo de documentação.

================================================================================
