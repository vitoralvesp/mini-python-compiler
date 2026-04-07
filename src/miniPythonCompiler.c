/** @name MiniPython Compiler Project
 *  @brief Este projeto implementa um compilador para uma versão simplificada da linguagem Python, denominada MiniPython. O compilador é dividido em duas fases principais: o Analisador Léxico, que segmenta o código fonte em tokens, e o Analisador Sintático, que valida a estrutura gramatical do código de acordo com as regras da linguagem.
 *  @authors Anna Luíza Stella Santos (10417401); Vitor Alves Pereira (10410862).
 *  @date 2026-04-07
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define FILE_OUTPUT_PATH "./output/"
#define FILE_TEMP_PATH "../temp/"
#define FILEPATH_COMMENTARIES_DEDICATED_FILE "./logs/commentariesIdentified.txt"
#define FILEPATH_IDENTIFIERS_DEDICATED_FILE "./logs/identifiersIdentified.txt"
#define FILEPATH_OPERATORS_DEDICATED_FILE "./logs/operatorsIdentified.txt"
#define FILEPATH_DELIMITERS_DEDICATED_FILE "./logs/delimitersIdentified.txt"
#define TOKENS_DEDICATED_FILE "./logs/tokensIdentified.txt"
#define LEXYCAL_ERRORS_DEDICATED_FILE "./logs/lexycalErrorsIdentified.txt"
#define SINTATIC_ERRORS_DEDICATED_FILE "./logs/sintaticErrorsIdentified.txt"

/** @brief Enumeração dos tipos de token (átomos) reconhecidos pelo Analisador Léxico. Cada tipo representa uma categoria de elementos na linguagem MiniPython, como identificadores, números, operadores, delimitadores, palavras reservadas e literais de string. O tipo ERRO é usado para tokens inválidos, e EOS (End Of Stream) indica o fim do arquivo fonte.
 * @param ERRO: Token inválido ou sequência de caracteres que não corresponde a nenhum token reconhecido.
 * @param IDENTIFICADOR: Nomes de variáveis, funções ou outros símbolos definidos pelo usuário.
 * @param NUMERO: Literais numéricos, como inteiros ou floats.
 * @param OPERADOR: Símbolos que representam operações, como +, -, *, /, etc.
 * @param DELIMITADOR: Símbolos que delimitam estruturas, como parênteses, colchetes, chaves, vírgulas, pontos e vírgulas.
 * @param PALAVRA_RESERVADA: Palavras que têm significado especial na linguagem, como if, else, while, def, etc.
 * @param STRING_LITERAL: Sequências de caracteres entre aspas, representando literais de string.
 * @param EOS: Indica o fim do arquivo fonte, sinalizando que não há mais tokens a serem processados.
 */
typedef enum {
    ERRO,
    IDENTIFICADOR,
    NUMERO,
    OPERADOR,
    DELIMITADOR,
    PALAVRA_RESERVADA,
    STRING_LITERAL,
    EOS
} TAtomo;

/** @brief Estrutura que representa um token identificado pelo Analisador Léxico. Cada token contém um tipo (atomo), o valor textual do token (valor) e a linha do código fonte onde o token foi encontrado (linha). Esta estrutura é fundamental para a análise sintática, pois fornece as informações necessárias para validar a estrutura gramatical do código.
 * @param atomo: O tipo do token, representado pela enumeração TAtomo.
 * @param valor: A representação textual do token, como o nome de um identificador, o valor de um número, o símbolo de um operador, etc.
 * @param linha: O número da linha no código fonte onde o token foi encontrado, usado para mensagens de erro e depuração.
 */
typedef struct {
    TAtomo atomo;
    char *valor;
    int linha;
} TToken;

// GLOBAL
static const char *NOMES_ATOMO[] = {
    "ERRO", "IDENTIFICADOR", "NUMERO", "OPERADOR",
    "DELIMITADOR", "PALAVRA_RESERVADA", "STRING_LITERAL", "EOS"
};

TToken *token_atual;
int linha_atual = 1;

FILE *input_ptr;
FILE *commentariesFile;
FILE *identifiersFile;
FILE *operatorsFile;
FILE *delimitersFile;
FILE *tokensFile;
FILE *lexicalErrorsFile;
FILE *sintaticErrorsFile;

static const char *palavras_reservadas[] = {
    "return", "from", "while", "as", "elif", "with", "else", "if",
    "break", "len", "input", "print", "exec", "in", "raise",
    "continue", "range", "def", "for", "True", "False", "and", "or", "not", "is"
};

#define NUM_PALAVRAS_RESERVADAS ((int)(sizeof(palavras_reservadas)/sizeof(palavras_reservadas[0])))

/** @brief Função auxiliar para verificar se uma palavra é reservada.
 *  @param s A string a ser verificada.
 *  @return 1 se a string for uma palavra reservada, 0 caso contrário.
 */
int eh_palavra_reservada(const char *s) {
    for (int i = 0; i < NUM_PALAVRAS_RESERVADAS; i++)
        if (strcmp(s, palavras_reservadas[i]) == 0) return 1;
    return 0;
}

/** @brief Função auxiliar para limpar o conteúdo de um arquivo (usada para zerar os arquivos de log no início do programa). 
 * @param filePath O caminho do arquivo a ser limpo.
 * @return 0 em caso de sucesso, 1 em caso de erro.
*/
int clearFile(const char *filePath) {
    FILE *f = fopen(filePath, "w");
    if (!f) { perror("Erro ao limpar o arquivo"); return 1; }
    fclose(f);
    return 0;
}

/** @brief Registra um erro léxico no terminal e no arquivo dedicado, depois encerra o programa.
 *  @param linha O número da linha onde o erro foi encontrado.
 *  @param contexto A sequência de caracteres que causou o erro, usada para fornecer uma mensagem de erro informativa.
 *  @return Esta função não retorna, pois chama exit(1) para encerrar o programa após registrar o erro.
 */
void registrar_erro_lexico(int linha, const char *contexto) {
    fprintf(stdout, "ERRO LEXICO na linha %d: sequencia '%s' invalida\n", linha, contexto);
    fflush(stdout);

    FILE *f = fopen(LEXYCAL_ERRORS_DEDICATED_FILE, "a");
    
    if (f) {
        fprintf(f, "ERRO LEXICO na linha %d: sequencia '%s' invalida\n", linha, contexto);
        fclose(f);
    } else {
        perror("Nao foi possivel abrir o arquivo de erros lexicos");
    }
    exit(1);
}

/** @brief Registra um erro sintático no terminal e no arquivo dedicado, depois encerra o programa.
 *  @param linha O número da linha onde o erro foi encontrado.
 *  @param esperado Uma descrição legível do que era esperado (tipo ou valor do token).
 *  @param recebido O valor do token que foi encontrado, mas não corresponde ao esperado.
 *  @return Esta função não retorna, pois chama exit(1) para encerrar o programa após registrar o erro.
 */
void registrar_erro_sintatico(int linha, const char *esperado, const char *recebido) {
    fprintf(stdout, "ERRO SINTATICO na linha %d: esperado '%s', mas recebeu '%s'\n",
            linha, esperado, recebido);
    fflush(stdout);

    FILE *f = fopen(SINTATIC_ERRORS_DEDICATED_FILE, "a");
    if (f) {
        fprintf(f, "ERRO SINTATICO na linha %d: esperado '%s', mas recebeu '%s'\n",
                linha, esperado, recebido);
        fclose(f);
    } else {
        perror("Nao foi possivel abrir o arquivo de erros sintaticos");
    }
    exit(1);
}

/** @brief Função principal do Analisador Léxico. Lê o código fonte a partir do arquivo de entrada, identifica e classifica os tokens, e registra os resultados em arquivos de log dedicados. Esta função é responsável por ignorar espaços em branco, identificar comentários, literais de string, números, identificadores, palavras reservadas, operadores e delimitadores. Em caso de caracteres inválidos, registra um erro léxico.
 *  @return Um ponteiro para a estrutura TToken que representa o token identificado. O token contém o tipo (atomo), o valor textual (valor) e a linha onde foi encontrado (linha).
 */
TToken *obter_atomo(void) {
    TToken *t = (TToken *)malloc(sizeof(TToken));
    if (!t) { perror("malloc"); exit(1); }

    char buffer[4096];
    int  idx = 0;
    int  c   = fgetc(input_ptr);

    /* Ignora espaços em branco */
    while (c != EOF && isspace((unsigned char)c)) {
        if (c == '\n') linha_atual++;
        c = fgetc(input_ptr);
    }

    t->linha = linha_atual;

    if (c == EOF) {
        t->atomo = EOS;
        t->valor = strdup("EOF");
        return t;
    }

    if (c == '#') {
        
        buffer[idx++] = (char)c;
        c = fgetc(input_ptr);
        while (c != '\n' && c != EOF) {
            if (idx < (int)sizeof(buffer) - 1) buffer[idx++] = (char)c;
            c = fgetc(input_ptr);
        }
        buffer[idx] = '\0';
        if (c == '\n') linha_atual++;

        FILE *cf = fopen(FILEPATH_COMMENTARIES_DEDICATED_FILE, "a");
        if (cf) { fprintf(cf, "linha %d: %s\n", t->linha, buffer); fclose(cf); }

        free(t);
        return obter_atomo();
    }

    if (c == '"' || c == '\'') {
        int quote = c;
        buffer[idx++] = (char)c;
        c = fgetc(input_ptr);
        while (c != quote && c != EOF) {
            if (c == '\n') {
                buffer[idx] = '\0';
                registrar_erro_lexico(t->linha, "string nao fechada");
            }
            if (idx < (int)sizeof(buffer) - 2) buffer[idx++] = (char)c;
            c = fgetc(input_ptr);
        }
        if (c == EOF) {
            buffer[idx] = '\0';
            registrar_erro_lexico(t->linha, "string nao fechada (fim de arquivo)");
        }
        buffer[idx++] = (char)quote;
        buffer[idx]   = '\0';
        t->atomo = STRING_LITERAL;
        t->valor = strdup(buffer);

        FILE *sf = fopen(TOKENS_DEDICATED_FILE, "a");
        if (sf) { fprintf(sf, "linha %d: STRING_LITERAL | %s\n", t->linha, t->valor); fclose(sf); }
        return t;
    }

    if (isdigit((unsigned char)c)) {
        while (isdigit((unsigned char)c)) {
            if (idx < (int)sizeof(buffer) - 1) buffer[idx++] = (char)c;
            c = fgetc(input_ptr);
        }
        ungetc(c, input_ptr);
        buffer[idx] = '\0';
        t->atomo = NUMERO;
        t->valor = strdup(buffer);

        FILE *tf = fopen(TOKENS_DEDICATED_FILE, "a");
        if (tf) { fprintf(tf, "linha %d: NUMERO | %s\n", t->linha, t->valor); fclose(tf); }
        return t;
    }

    if (isalpha((unsigned char)c) || c == '_') {
        while (isalnum((unsigned char)c) || c == '_') {
            if (idx < (int)sizeof(buffer) - 1) buffer[idx++] = (char)c;
            c = fgetc(input_ptr);
        }
        ungetc(c, input_ptr);
        buffer[idx] = '\0';

        if (eh_palavra_reservada(buffer)) {
            t->atomo = PALAVRA_RESERVADA;
            FILE *pf = fopen(TOKENS_DEDICATED_FILE, "a");
            if (pf) { fprintf(pf, "linha %d: PALAVRA_RESERVADA | %s\n", t->linha, buffer); fclose(pf); }
        } else {
            t->atomo = IDENTIFICADOR;
            FILE *idf = fopen(FILEPATH_IDENTIFIERS_DEDICATED_FILE, "a");
            if (idf) { fprintf(idf, "linha %d: %s\n", t->linha, buffer); fclose(idf); }
            FILE *tf = fopen(TOKENS_DEDICATED_FILE, "a");
            if (tf) { fprintf(tf, "linha %d: IDENTIFICADOR | %s\n", t->linha, buffer); fclose(tf); }
        }
        t->valor = strdup(buffer);
        return t;
    }

    if (strchr("+-*/%=><!~", c)) {
        buffer[idx++] = (char)c;
        int prox = fgetc(input_ptr);

        if ((c == '*' && prox == '*') ||
            (c == '=' && prox == '=') ||
            (c == '!' && prox == '=') ||
            (c == '<' && prox == '=') ||
            (c == '>' && prox == '=') ||
            (c == '<' && prox == '>')) {
            buffer[idx++] = (char)prox;
        } else {
            ungetc(prox, input_ptr);
        }
        buffer[idx] = '\0';
        t->atomo = OPERADOR;
        t->valor = strdup(buffer);

        FILE *of = fopen(FILEPATH_OPERATORS_DEDICATED_FILE, "a");
        if (of) { fprintf(of, "linha %d: %s\n", t->linha, buffer); fclose(of); }
        FILE *tf = fopen(TOKENS_DEDICATED_FILE, "a");
        if (tf) { fprintf(tf, "linha %d: OPERADOR | %s\n", t->linha, buffer); fclose(tf); }
        return t;
    }

    if (strchr("(){}[],:.;", c)) {
        buffer[0] = (char)c;
        buffer[1] = '\0';
        t->atomo = DELIMITADOR;
        t->valor = strdup(buffer);

        FILE *df = fopen(FILEPATH_DELIMITERS_DEDICATED_FILE, "a");
        if (df) { fprintf(df, "linha %d: %s\n", t->linha, buffer); fclose(df); }
        FILE *tf = fopen(TOKENS_DEDICATED_FILE, "a");
        if (tf) { fprintf(tf, "linha %d: DELIMITADOR | %s\n", t->linha, buffer); fclose(tf); }
        return t;
    }

    buffer[0] = (char)c;
    buffer[1] = '\0';
    registrar_erro_lexico(linha_atual, buffer);

    t->atomo = ERRO;
    t->valor = strdup(buffer);
    return t;
}

/** @brief Função principal do Analisador Sintático. Esta função e suas auxiliares implementam a análise sintática do código fonte, validando a estrutura gramatical de acordo com as regras da linguagem MiniPython. A função parse_programa() inicia a análise, que é composta por uma sequência de comandos (parse_comando) até o final do arquivo (EOS). Cada comando pode ser uma atribuição, uma chamada de função, um controle de fluxo (if, while, for), uma definição de função (def), ou outras construções válidas. A função parse_expressao() é responsável por analisar expressões aritméticas, relacionais e lógicas, garantindo a correta precedência e associatividade dos operadores.
 * @return Esta função não retorna um valor, mas pode chamar registrar_erro_sintatico() para reportar erros de sintaxe encontrados durante a análise. Se a análise for concluída com sucesso, imprime uma mensagem indicando que a análise sintática foi concluída sem erros.
 */
void parse_expressao(void);

/** @brief Função auxiliar para consumir um token esperado. Verifica se o token atual corresponde ao tipo esperado (atomo) e, em caso afirmativo, avança para o próximo token. Se o token atual não corresponder ao esperado, registra um erro sintático com uma mensagem informativa que inclui o tipo ou valor esperado e o valor recebido. Esta função é fundamental para a validação da estrutura gramatical do código fonte durante a análise sintática.
 * @param esperado O tipo de token (atomo) que é esperado no ponto atual da análise sintática. Este parâmetro é usado para comparar com o token atual e determinar se a estrutura do código está correta.
 * @return Esta função não retorna um valor, mas pode chamar registrar_erro_sintatico() para reportar erros de sintaxe encontrados durante a análise. Se o token atual corresponder ao esperado, a função avança para o próximo token e continua a análise normalmente.
 */
void consome(TAtomo esperado) {
    if (token_atual->atomo == esperado) {
        printf("%d# %s | %s\n",
               token_atual->linha,
               NOMES_ATOMO[token_atual->atomo],
               token_atual->valor);
        fflush(stdout);

        free(token_atual->valor);
        free(token_atual);
        token_atual = obter_atomo();
    } else {
        registrar_erro_sintatico(
            token_atual->linha,
            NOMES_ATOMO[esperado],   
            token_atual->valor     
        );
    }
}

/** @brief Função auxiliar para consumir um token esperado com valor específico. Verifica se o token atual corresponde ao tipo esperado (atomo) e se seu valor textual é igual ao valor esperado. Se ambos os critérios forem atendidos, avança para o próximo token. Caso contrário, registra um erro sintático com uma mensagem informativa que inclui o valor esperado e o valor recebido. Esta função é especialmente útil para validar a presença de delimitadores específicos (como parênteses, colchetes, vírgulas) ou palavras reservadas em pontos específicos da análise sintática.
 * @param esperado O tipo de token (atomo) que é esperado no ponto atual da análise sintática.
 * @param valor_esperado O valor textual específico que o token atual deve ter para ser considerado válido.
 * @return Esta função não retorna um valor, mas pode chamar registrar_erro_sintatico() para reportar erros de sintaxe encontrados durante a análise. Se o token atual corresponder ao tipo e valor esperados, a função avança para o próximo token e continua a análise normalmente.
 */
void consome_valor(TAtomo esperado, const char *valor_esperado) {
    if (token_atual->atomo == esperado &&
        strcmp(token_atual->valor, valor_esperado) == 0) {
        printf("%d# %s | %s\n",
               token_atual->linha,
               NOMES_ATOMO[token_atual->atomo],
               token_atual->valor);
        fflush(stdout);

        free(token_atual->valor);
        free(token_atual);
        token_atual = obter_atomo();
    } else {
        
        char esperado_str[128];
        snprintf(esperado_str, sizeof(esperado_str), "'%s'", valor_esperado);
        registrar_erro_sintatico(
            token_atual->linha,
            esperado_str,
            token_atual->valor
        );
    }
}

/** @brief Analisa um fator, que pode ser um identificador (com possível indexação ou chamada de função), um número, uma string literal, uma palavra reservada usada como valor (como True, False, None), uma lista literal ou uma subexpressão entre parênteses. Esta função é fundamental para a análise de expressões mais complexas, pois trata os elementos básicos que podem compor expressões aritméticas, relacionais e lógicas.
 * @return Esta função não retorna um valor, mas pode chamar registrar_erro_sintatico() para reportar erros de sintaxe encontrados durante a análise. Se o fator for válido, a função consome os tokens correspondentes e continua a análise normalmente.
 */
void parse_fator() {
    if (token_atual->atomo == IDENTIFICADOR) {
        consome(IDENTIFICADOR);

        if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "[") == 0) {
            consome_valor(DELIMITADOR, "[");
            parse_expressao();
            consome_valor(DELIMITADOR, "]");

        } else if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "(") == 0) {
            consome_valor(DELIMITADOR, "(");
            if (!(token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, ")") == 0)) {
                parse_expressao();
                while (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, ",") == 0) {
                    consome_valor(DELIMITADOR, ",");
                    parse_expressao();
                }
            }
            consome_valor(DELIMITADOR, ")");
        }

    } else if (token_atual->atomo == NUMERO) {
        consome(NUMERO);

    } else if (token_atual->atomo == STRING_LITERAL) {
        consome(STRING_LITERAL);

    } else if (token_atual->atomo == PALAVRA_RESERVADA) {
        consome(PALAVRA_RESERVADA);
        
        if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "(") == 0) {
            consome_valor(DELIMITADOR, "(");
            if (!(token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, ")") == 0)) {
                parse_expressao();
                while (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, ",") == 0) {
                    consome_valor(DELIMITADOR, ",");
                    parse_expressao();
                }
            }
            consome_valor(DELIMITADOR, ")");
        }

    } else if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "[") == 0) {
        consome_valor(DELIMITADOR, "[");
        if (!(token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "]") == 0)) {
            parse_expressao();
            while (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, ",") == 0) {
                consome_valor(DELIMITADOR, ",");
                parse_expressao();
            }
        }
        consome_valor(DELIMITADOR, "]");

    } else if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "(") == 0) {
        consome_valor(DELIMITADOR, "(");
        parse_expressao();
        consome_valor(DELIMITADOR, ")");

    } else {
        registrar_erro_sintatico(
            token_atual->linha,
            "fator (identificador, numero, string ou '(')",
            token_atual->valor
        );
    }
}

/** @brief Analisa uma potência, que é composta por um fator seguido opcionalmente pelo operador de potência '**' e outra potência. Esta função é responsável por garantir a correta precedência do operador de potência em expressões aritméticas, permitindo que fatores sejam elevados a potências de forma aninhada.
 * @return Esta função não retorna um valor, mas pode chamar registrar_erro_sintatico() para reportar erros de sintaxe encontrados durante a análise. Se a estrutura da potência for válida, a função consome os tokens correspondentes e continua a análise normalmente.
 */
void parse_potencia() {
    parse_fator();
    if (token_atual->atomo == OPERADOR && strcmp(token_atual->valor, "**") == 0) {
        consome(OPERADOR);
        parse_potencia();
    }
}

/** @brief Analisa um termo, que é composto por uma potência seguida opcionalmente por operadores de multiplicação, divisão ou módulo ('*', '/', '%') e outras potências. Esta função é responsável por garantir a correta precedência dos operadores de multiplicação, divisão e módulo em expressões aritméticas, permitindo que potências sejam combinadas com esses operadores de forma aninhada.
 * @return Esta função não retorna um valor, mas pode chamar registrar_erro_sintatico() para reportar erros de sintaxe encontrados durante a análise. Se a estrutura do termo for válida, a função consome os tokens correspondentes e continua a análise normalmente.
 */
void parse_termo() {
    parse_potencia();
    while (token_atual->atomo == OPERADOR &&
           strlen(token_atual->valor) == 1 &&
           strchr("*/%", token_atual->valor[0])) {
        consome(OPERADOR);
        parse_potencia();
    }
}

/** @brief Analisa uma expressão aritmética, que é composta por um termo seguido opcionalmente por operadores de adição ou subtração ('+' ou '-') e outros termos. Esta função é responsável por garantir a correta precedência dos operadores de adição e subtração em expressões aritméticas, permitindo que termos sejam combinados com esses operadores de forma aninhada.
 * @return Esta função não retorna um valor, mas pode chamar registrar_erro_sintatico() para reportar erros de sintaxe encontrados durante a análise. Se a estrutura da expressão aritmética for válida, a função consome os tokens correspondentes e continua a análise normalmente.
 */
void parse_expressao_aritmetica() {
    parse_termo();
    while (token_atual->atomo == OPERADOR &&
           strlen(token_atual->valor) == 1 &&
           strchr("+-", token_atual->valor[0])) {
        consome(OPERADOR);
        parse_termo();
    }
}

/** @brief Analisa uma expressão relacional, que é composta por uma expressão aritmética seguida opcionalmente por operadores relacionais ('>', '<', '>=', '<=', '==', '!=', '<>') ou palavras reservadas de comparação ('in', 'is') e outras expressões aritméticas. Esta função é responsável por garantir a correta estrutura das expressões relacionais, permitindo que expressões aritméticas sejam comparadas usando os operadores relacionais e palavras reservadas de comparação.
 * @return Esta função não retorna um valor, mas pode chamar registrar_erro_sintatico() para reportar erros de sintaxe encontrados durante a análise. Se a estrutura da expressão relacional for válida, a função consome os tokens correspondentes e continua a análise normalmente.
 */
void parse_relacional() {
    parse_expressao_aritmetica();

    while ((token_atual->atomo == OPERADOR &&
            (strcmp(token_atual->valor, ">")  == 0 ||
             strcmp(token_atual->valor, "<")  == 0 ||
             strcmp(token_atual->valor, ">=") == 0 ||
             strcmp(token_atual->valor, "<=") == 0 ||
             strcmp(token_atual->valor, "==") == 0 ||
             strcmp(token_atual->valor, "!=") == 0 ||
             strcmp(token_atual->valor, "<>") == 0)) ||
           (token_atual->atomo == PALAVRA_RESERVADA &&
            (strcmp(token_atual->valor, "in") == 0 ||
             strcmp(token_atual->valor, "is") == 0))) {

        if (token_atual->atomo == OPERADOR) consome(OPERADOR);
        else consome(PALAVRA_RESERVADA);

        parse_expressao_aritmetica();
    }
}

/** @brief Analisa uma expressão, que é composta por uma expressão relacional seguida opcionalmente por operadores lógicos 'and' ou 'or' e outras expressões relacionais. Esta função é responsável por garantir a correta estrutura das expressões lógicas, permitindo que expressões relacionais sejam combinadas usando os operadores lógicos 'and' e 'or'.
 * @return Esta função não retorna um valor, mas pode chamar registrar_erro_sintatico() para reportar erros de sintaxe encontrados durante a análise. Se a estrutura da expressão lógica for válida, a função consome os tokens correspondentes e continua a análise normalmente.
 */
void parse_expressao() {
    parse_relacional();
    while (token_atual->atomo == PALAVRA_RESERVADA &&
           (strcmp(token_atual->valor, "and") == 0 ||
            strcmp(token_atual->valor, "or")  == 0)) {
        consome(PALAVRA_RESERVADA);
        parse_relacional();
    }
}

void parse_bloco();
void parse_comando();

/** @brief Analisa um bloco de código, que é composto por uma sequência de comandos. Esta função é responsável por validar a estrutura de blocos de código em construções como if, while, for e def, garantindo que cada comando dentro do bloco seja analisado corretamente. O bloco é considerado encerrado quando o token atual for EOS (End Of Stream), indicando o fim do arquivo fonte.
 * @return Esta função não retorna um valor, mas pode chamar registrar_erro_sintatico() para reportar erros de sintaxe encontrados durante a análise. Se a estrutura do bloco for válida, a função consome os tokens correspondentes e continua a análise normalmente até o final do arquivo.
 */
void parse_bloco() {
    parse_comando();
}

/** @brief Analisa um comando, que pode ser uma atribuição, uma chamada de função, um controle de fluxo (if, while, for), uma definição de função (def), ou outras construções válidas na linguagem MiniPython. Esta função é responsável por validar a estrutura de cada comando, garantindo que os tokens estejam na ordem correta e que as expressões sejam analisadas adequadamente. O comando é considerado encerrado quando o token atual for EOS (End Of Stream), indicando o fim do arquivo fonte.
 * @return Esta função não retorna um valor, mas pode chamar registrar_erro_sintatico() para reportar erros de sintaxe encontrados durante a análise. Se a estrutura do comando for válida, a função consome os tokens correspondentes e continua a análise normalmente até o próximo comando ou o final do arquivo.
 */
void parse_comando() {

    if (token_atual->atomo == EOS) return;

    if (token_atual->atomo == IDENTIFICADOR) {
        parse_fator(); 

        if (token_atual->atomo == OPERADOR && strcmp(token_atual->valor, "=") == 0) {
            consome_valor(OPERADOR, "=");
            parse_expressao();
        }

    } else if (token_atual->atomo == PALAVRA_RESERVADA) {

        if (strcmp(token_atual->valor, "print") == 0) {
            consome(PALAVRA_RESERVADA);
            
            if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "(") == 0) {
                consome_valor(DELIMITADOR, "(");
                if (!(token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, ")") == 0)) {
                    parse_expressao();
                    while (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, ",") == 0) {
                        consome_valor(DELIMITADOR, ",");
                        parse_expressao();
                    }
                }
                consome_valor(DELIMITADOR, ")");
            } else {
                parse_expressao();
                while (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, ",") == 0) {
                    consome_valor(DELIMITADOR, ",");
                    parse_expressao();
                }
            }

        } else if (strcmp(token_atual->valor, "return") == 0) {
            consome(PALAVRA_RESERVADA);
            if (token_atual->atomo != EOS &&
                !(token_atual->atomo == PALAVRA_RESERVADA &&
                  (strcmp(token_atual->valor, "else")  == 0 ||
                   strcmp(token_atual->valor, "elif")  == 0 ||
                   strcmp(token_atual->valor, "def")   == 0))) {
                parse_expressao();
            }

        } else if (strcmp(token_atual->valor, "if") == 0) {
            consome(PALAVRA_RESERVADA);
            parse_expressao();
            consome_valor(DELIMITADOR, ":");
            parse_bloco();

            while (token_atual->atomo == PALAVRA_RESERVADA &&
                   strcmp(token_atual->valor, "elif") == 0) {
                consome(PALAVRA_RESERVADA);
                parse_expressao();
                consome_valor(DELIMITADOR, ":");
                parse_bloco();
            }

            if (token_atual->atomo == PALAVRA_RESERVADA &&
                strcmp(token_atual->valor, "else") == 0) {
                consome(PALAVRA_RESERVADA);
                consome_valor(DELIMITADOR, ":");
                parse_bloco();
            }

        } else if (strcmp(token_atual->valor, "else") == 0) {
            consome(PALAVRA_RESERVADA);
            consome_valor(DELIMITADOR, ":");
            parse_bloco();

        } else if (strcmp(token_atual->valor, "while") == 0) {
            consome(PALAVRA_RESERVADA);
            parse_expressao();
            consome_valor(DELIMITADOR, ":");
            parse_bloco();

        } else if (strcmp(token_atual->valor, "for") == 0) {
            consome(PALAVRA_RESERVADA);
            consome(IDENTIFICADOR);
            consome_valor(PALAVRA_RESERVADA, "in");
            parse_fator();
            consome_valor(DELIMITADOR, ":");
            parse_bloco();

        } else if (strcmp(token_atual->valor, "def") == 0) {
            consome(PALAVRA_RESERVADA);
            consome(IDENTIFICADOR);
            consome_valor(DELIMITADOR, "(");
            if (!(token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, ")") == 0)) {
                consome(IDENTIFICADOR);
                while (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, ",") == 0) {
                    consome_valor(DELIMITADOR, ",");
                    consome(IDENTIFICADOR);
                }
            }
            consome_valor(DELIMITADOR, ")");
            consome_valor(DELIMITADOR, ":");
            parse_bloco();

        } else if (strcmp(token_atual->valor, "break")    == 0 ||
                   strcmp(token_atual->valor, "continue") == 0) {
            consome(PALAVRA_RESERVADA);

        } else if (strcmp(token_atual->valor, "raise") == 0) {
            consome(PALAVRA_RESERVADA);
            parse_expressao();

        } else {
            parse_fator();
        }

    } else if (token_atual->atomo != EOS) {
        registrar_erro_sintatico(
            token_atual->linha,
            "inicio de comando valido (identificador ou palavra reservada)",
            token_atual->valor
        );
    }
}

/** @brief Função principal do Analisador Sintático. Esta função inicia a análise sintática do código fonte, processando uma sequência de comandos até o final do arquivo (EOS). A função chama parse_comando() para analisar cada comando encontrado, garantindo que a estrutura gramatical do código esteja correta. Se a análise for concluída sem erros, imprime uma mensagem indicando que a análise sintática foi concluída com sucesso.
 * @return Esta função não retorna um valor, mas pode chamar registrar_erro_sintatico() para reportar erros de sintaxe encontrados durante a análise. Se a análise for concluída com sucesso, imprime uma mensagem indicando que a análise sintática foi concluída sem erros.
 */
void parse_programa(void) {
    while (token_atual->atomo != EOS) {
        parse_comando();
    }
    printf("Analise sintatica concluida com sucesso.\n");
}

int main(int argc, char *argv[]) {

    clearFile(FILEPATH_COMMENTARIES_DEDICATED_FILE);
    clearFile(FILEPATH_IDENTIFIERS_DEDICATED_FILE);
    clearFile(FILEPATH_OPERATORS_DEDICATED_FILE);
    clearFile(FILEPATH_DELIMITERS_DEDICATED_FILE);
    clearFile(TOKENS_DEDICATED_FILE);
    clearFile(LEXYCAL_ERRORS_DEDICATED_FILE);
    clearFile(SINTATIC_ERRORS_DEDICATED_FILE);

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_fonte>\n", argv[0]);
        return 1;
    }

    input_ptr = fopen(argv[1], "r");
    if (!input_ptr) {
        fprintf(stderr, "Erro ao abrir arquivo '%s'\n", argv[1]);
        return 1;
    }

    token_atual = obter_atomo();
    parse_programa();

    fclose(input_ptr);
    return 0;
}