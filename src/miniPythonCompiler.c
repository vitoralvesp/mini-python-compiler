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

/** @brief Estrutura de dados para representação dos átomos (tokens) identificados pelo Analisador Léxico. Cada átomo é classificado por um tipo específico, como identificadores, números, operadores, delimitadores, palavras reservadas e literais de string. O tipo EOS (End Of Stream) é utilizado para indicar o término do fluxo de caracteres do arquivo fonte.
 * @param ERRO: Representa um token inválido ou erro de análise léxica.
 * @param IDENTIFICADOR: Representa nomes de variáveis, funções ou outros identificadores definidos pelo usuário.
 * @param NUMERO: Representa valores numéricos inteiros encontrados no código fonte.
 * @param OPERADOR: Representa símbolos de operadores aritméticos, lógicos ou de
 * comparação utilizados nas expressões do código fonte.
 * @param DELIMITADOR: Representa símbolos de pontuação e delimitadores sintáticos, como parênteses, colchetes, chaves, vírgulas e pontos.
 * @param PALAVRA_RESERVADA: Representa palavras-chave da linguagem Python que possuem significado especial e não podem ser utilizadas como identificadores, como "if", "else", "while", "for", "def", entre outras.
 * @param STRING_LITERAL: Representa sequências de caracteres literais encontradas entre aspas simples ou duplas no código fonte, utilizadas para representar textos ou mensagens a serem processados pelo programa.
 * @param EOS: Representa o fim do fluxo de caracteres do arquivo fonte, indicando que não há mais tokens a serem processados pelo Analisador Léxico
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

/** @brief Estrutura de dados para armazenar informações sobre cada token identificado pelo Analisador Léxico. Cada token é composto por um tipo (TAtomo), um valor (string) e a linha do código fonte onde o token foi encontrado. Essa estrutura é fundamental para o processo de análise sintática, permitindo que o parser acesse as informações necessárias para validar a estrutura gramatical do código fonte.
 * @param atomo: Tipo do token, representado por um valor do enum TAtomo, que indica a categoria do token (identificador, número, operador, etc.).
 * @param valor: String que armazena o valor literal do token, como o nome de um identificador, o valor de um número, o símbolo de um operador ou o conteúdo de uma string literal.
 * @param linha: Número da linha do código fonte onde o token foi encontrado, utilizado para fins de relatório de erros e acompanhamento do processo de análise sintática.
 */
typedef struct {
    TAtomo atomo;
    char *valor;
    int linha;
} TToken;

// Variáveis Globais
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

const char *palavras_reservadas[] = {
    "return", "from", "while", "as", "elif", "with", "else", "if", 
    "break", "len", "input", "print", "exec", "in", "raise", 
    "continue", "range", "def", "for", "True", "False", "and", "or", "not", "is"
};

/** @brief Função auxiliar para verificar se uma string é uma palavra reservada ou não.
 *  @param s: String a ser verificada contra a lista de palavras reservadas.
 *  @return Retorna 1 se a string for uma palavra reservada, ou 0 caso contrário.
 */
int eh_palavra_reservada(char *s) {
    for (int i = 0; i < 25; i++) {
        if (strcmp(s, palavras_reservadas[i]) == 0) return 1;
    }
    return 0;
}

/** @brief Função para limpar o conteúdo de um arquivo, utilizada para garantir que os arquivos de log estejam vazios antes de iniciar um novo processo de compilação. A função abre o arquivo em modo de escrita ("w"), o que automaticamente limpa seu conteúdo, e em seguida fecha o arquivo. Em caso de erro ao abrir o arquivo, a função imprime uma mensagem de erro e retorna 1; caso contrário, retorna 0 indicando sucesso na limpeza do arquivo.
 * @return Retorna 0 se o arquivo foi limpo com sucesso, ou 1 em caso de erro ao abrir o arquivo para limpeza.
 */
int clearFile(const char *filePath) {
    FILE *file = fopen(filePath, "w");
    if (!file) {
        perror("Erro ao limpar o arquivo");
        return 1;
    }
    fclose(file);
    return 0;
}

/** @brief Função principal do Analisador Léxico, responsável por ler o arquivo fonte e segmentar o código em tokens (átomos) de acordo com as regras da linguagem MiniPython. A função lida com a identificação de diferentes tipos de tokens, como identificadores, números, operadores, delimitadores, palavras reservadas e literais de string. Além disso, a função também gerencia a contagem de linhas para fins de relatório de erros e ignora comentários e espaços em branco.
 * @return Retorna um ponteiro para a estrutura TToken contendo as informações do token identificado, incluindo seu tipo, valor e linha de ocorrência no código fonte. Em caso de erro léxico, a função imprime uma mensagem de erro e encerra o programa.
 */
TToken* obter_atomo() {
    TToken *t = (TToken *)malloc(sizeof(TToken));
    char buffer[1024];
    int idx = 0;
    int c = fgetc(input_ptr);

    while (c != EOF && isspace(c)) {
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
        while (c != '\n' && c != EOF) c = fgetc(input_ptr);
        if (c == '\n') linha_atual++;
        return obter_atomo();
    }

    if (c == '\"' || c == '\'') {
        int quote = c;
        buffer[idx++] = c;
        c = fgetc(input_ptr);
        while (c != quote && c != EOF) {
            if (c == '\n') linha_atual++;
            buffer[idx++] = c;
            c = fgetc(input_ptr);
        }
        buffer[idx++] = quote;
        buffer[idx] = '\0';
        t->atomo = STRING_LITERAL;
        t->valor = strdup(buffer);
        return t;
    }

    if (isdigit(c)) goto estado_numero;
    if (isalpha(c) || c == '_') goto estado_identificador;
    if (strchr("+-*/%=><!~", c)) goto estado_operador;
    if (strchr("(){}[],:.;", c)) goto estado_delimitador;

    lexicalErrorsFile = fopen(LEXYCAL_ERRORS_DEDICATED_FILE, "a");

    printf("ERRO LEXICO na linha %d: sequencia '%c' invalida\n", linha_atual, c);

    fprintf(lexicalErrorsFile, "ERRO LEXICO na linha %d: sequencia '%c' invalida\n", linha_atual, c);

    fclose(lexicalErrorsFile);

    exit(1);

estado_numero:
    
    while (isdigit(c)) {
        buffer[idx++] = c;
        c = fgetc(input_ptr);
    }
    ungetc(c, input_ptr);
    buffer[idx] = '\0';
    t->atomo = NUMERO;
    t->valor = strdup(buffer);
    return t;

estado_identificador:

    while (isalnum(c) || c == '_') {
        buffer[idx++] = c;
        c = fgetc(input_ptr);
    }
    ungetc(c, input_ptr);
    buffer[idx] = '\0';
    
    if (eh_palavra_reservada(buffer)) t->atomo = PALAVRA_RESERVADA;
    else t->atomo = IDENTIFICADOR;
    t->valor = strdup(buffer);
    return t;

estado_operador:
    buffer[idx++] = c;
    int prox = fgetc(input_ptr);
    
    if ((c == '*' && prox == '*') || (c == '=' && prox == '=') || 
        (c == '!' && prox == '=') || (c == '<' && prox == '=') || 
        (c == '>' && prox == '=') || (c == '<' && prox == '>')) {
        buffer[idx++] = prox;
        buffer[idx] = '\0';
    } else {
        ungetc(prox, input_ptr);
        buffer[idx] = '\0';
    }
    t->atomo = OPERADOR;
    t->valor = strdup(buffer);
    return t;

estado_delimitador:
    
    buffer[0] = c;
    buffer[1] = '\0';
    t->atomo = DELIMITADOR;
    t->valor = strdup(buffer);
    return t;
}

/** @brief Função principal do Analisador Sintático, responsável por validar a estrutura gramatical do código fonte de acordo com as regras da linguagem MiniPython. A função implementa uma hierarquia de análise que respeita a precedência dos operadores e a estrutura das expressões, comandos e blocos de código. O parser é projetado para lidar com expressões aritméticas, lógicas, comparações, atribuições, estruturas de controle (if, else, while, for) e chamadas de funções, garantindo que o código fonte esteja sintaticamente correto antes de prosseguir para etapas posteriores do processo de compilação.
 * @return Não retorna um valor, mas em caso de erro sintático, a função imprime uma mensagem de erro detalhada indicando a linha e o token problemático, e encerra o programa. Se o código fonte for sintaticamente correto, a função processa todo o arquivo até o final, validando cada comando e expressão conforme as regras da linguagem MiniPython.
 */
void parse_expressao();

/** @brief Função auxiliar para consumir tokens durante a análise sintática. A função verifica se o token atual corresponde ao tipo esperado e, em caso afirmativo, avança para o próximo token. Se o token atual não corresponder ao tipo esperado, a função imprime uma mensagem de erro detalhada indicando a linha e o token problemático, e encerra o programa. Além disso, a função também imprime os tokens consumidos para acompanhamento da análise sintática, facilitando a identificação de erros e a compreensão do fluxo de análise do código fonte.
 * @param esperado: Tipo do token esperado, representado por um valor do enum TAtomo, que indica a categoria do token que deve ser consumido para que a análise sintática continue corretamente.
 * @return Não retorna um valor, mas em caso de erro sintático, a função imprime uma mensagem de erro detalhada indicando a linha e o token problemático, e encerra o programa. Se o token atual corresponder ao tipo esperado, a função avança para o próximo token e imprime os detalhes do token consumido para acompanhamento da análise sintática.
 */
void consome(TAtomo esperado) {
    if (token_atual->atomo == esperado) {
        char *nomes[] = {"ERRO", "IDENTIFICADOR", "NUMERO", "OPERADOR", "DELIMITADOR", "PALAVRA_RESERVADA", "STRING_LITERAL", "EOS"};
        
        printf("%d# %s | %s\n", token_atual->linha, nomes[token_atual->atomo], token_atual->valor);
        token_atual = obter_atomo();
    
    } else {

        sintaticErrorsFile = fopen(SINTATIC_ERRORS_DEDICATED_FILE, "a");

        printf("ERRO SINTATICO na linha %d: esperado token tipo %d, mas recebeu '%s'\n", token_atual->linha, esperado, token_atual->valor);

        fprintf(sintaticErrorsFile, "ERRO SINTATICO na linha %d: esperado token tipo %d, mas recebeu '%s'\n", token_atual->linha, esperado, token_atual->valor);

        fclose(sintaticErrorsFile);

        exit(1);
    }
}

/** @brief Função para analisar fatores em expressões, incluindo identificadores, números, literais de string, palavras reservadas e expressões entre parênteses. A função também lida com chamadas de funções e indexação de vetores, permitindo a construção de expressões complexas e a validação da sintaxe correta para cada tipo de fator. Em caso de fator inválido, a função imprime uma mensagem de erro detalhada indicando a linha e o token problemático, e encerra o programa.
 * @return Não retorna um valor, mas em caso de fator inválido, a função imprime uma mensagem de erro detalhada indicando a linha e o token problemático, e encerra o programa. Se o fator for válido, a função processa o token correspondente e avança para o próximo token para continuar a análise sintática.
 */
void parse_fator() {
    if (token_atual->atomo == IDENTIFICADOR) {
        consome(IDENTIFICADOR);
        
        if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "[") == 0) {
            consome(DELIMITADOR);
            parse_expressao();
            consome(DELIMITADOR); 
        }
        
        else if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "(") == 0) {
            consome(DELIMITADOR);
            if (strcmp(token_atual->valor, ")") != 0) parse_expressao();
            consome(DELIMITADOR);
        }

    } else if (token_atual->atomo == NUMERO) {
        consome(NUMERO);

    } else if (token_atual->atomo == STRING_LITERAL) {
        consome(STRING_LITERAL);

    } else if (token_atual->atomo == PALAVRA_RESERVADA) {
        consome(PALAVRA_RESERVADA);
        
        if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "(") == 0) {
            consome(DELIMITADOR);
            if (strcmp(token_atual->valor, ")") != 0) parse_expressao();
            consome(DELIMITADOR);
        }

    } else if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "[") == 0) {
        
        consome(DELIMITADOR);
        if (strcmp(token_atual->valor, "]") != 0) {
            parse_expressao();
            while (strcmp(token_atual->valor, ",") == 0) {
                consome(DELIMITADOR);
                parse_expressao();
            }
        }
        consome(DELIMITADOR);

    } else if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "(") == 0) {
        
        consome(DELIMITADOR);
        parse_expressao();
        consome(DELIMITADOR);

    } else {

        sintaticErrorsFile = fopen(SINTATIC_ERRORS_DEDICATED_FILE, "a");

        printf("ERRO SINTATICO na linha %d: fator invalido '%s'\n", token_atual->linha, token_atual->valor);

        fprintf(sintaticErrorsFile, "ERRO SINTATICO na linha %d: fator invalido '%s'\n", token_atual->linha, token_atual->valor);

        fclose(sintaticErrorsFile);
        
        exit(1);
    }
}

/** @brief Função para analisar expressões de potência, garantindo a correta precedência dos operadores. A função chama parse_fator() para processar o fator base da potência e, em seguida, verifica se o próximo token é o operador de potência "**". Se for, a função consome o operador e chama parse_potencia() recursivamente para processar o expoente da potência. Essa abordagem permite a construção de expressões de potência aninhadas, respeitando a associatividade correta dos operadores.
 * @return Não retorna um valor, mas em caso de expressão de potência inválida, a função imprime uma mensagem de erro detalhada indicando a linha e o token problemático, e encerra o programa. Se a expressão de potência for válida, a função processa os tokens correspondentes e avança para o próximo token para continuar a análise sintática.
 */
void parse_potencia() {
    parse_fator();
    if (token_atual->atomo == OPERADOR && strcmp(token_atual->valor, "**") == 0) {
        consome(OPERADOR);
        parse_potencia();
    }
}

/** @brief Função para analisar termos em expressões aritméticas, respeitando a precedência dos operadores de multiplicação, divisão e módulo.
 * @return Não retorna um valor, mas em caso de termo inválido, a função imprime uma mensagem de erro detalhada indicando a linha e o token problemático, e encerra o programa.
 */
void parse_termo() {
    parse_potencia();
    while (token_atual->atomo == OPERADOR && strchr("*/%", token_atual->valor[0]) && strlen(token_atual->valor) == 1) {
        consome(OPERADOR);
        parse_potencia();
    }
}

/** @brief Função para analisar expressões aritméticas, respeitando a precedência dos operadores de adição e subtração. A função chama parse_termo() para processar os termos da expressão e, em seguida, verifica se o próximo token é um operador de adição "+" ou subtração "-". Se for, a função consome o operador e chama parse_termo() novamente para processar o próximo termo da expressão. Essa abordagem permite a construção de expressões aritméticas complexas, respeitando a associatividade correta dos operadores.
 * @return Não retorna um valor, mas em caso de expressão aritmética inválida, a função imprime uma mensagem de erro detalhada indicando a linha e o token problemático, e encerra o programa. Se a expressão aritmética for válida, a função processa os tokens correspondentes e avança para o próximo token para continuar a análise sintática.
 */
void parse_expressao_aritmetica() {
    parse_termo();
    while (token_atual->atomo == OPERADOR && strchr("+-", token_atual->valor[0])) {
        consome(OPERADOR);
        parse_termo();
    }
}

/** @brief Função para analisar expressões relacionais, incluindo operadores de comparação e palavras reservadas relacionadas a comparações. A função chama parse_expressao_aritmetica() para processar os operandos da comparação e, em seguida, verifica se o próximo token é um operador de comparação (">", "<", ">=", "<=", "==", "!=") ou uma palavra reservada relacionada a comparações ("in", "is"). Se for, a função consome o operador ou palavra reservada e chama parse_expressao_aritmetica() novamente para processar o próximo operando da comparação. Essa abordagem permite a construção de expressões relacionais complexas, respeitando a associatividade correta dos operadores de comparação.
 * @return Não retorna um valor, mas em caso de expressão relacional inválida, a função imprime uma mensagem de erro detalhada indicando a linha e o token problemático, e encerra o programa. Se a expressão relacional for válida, a função processa os tokens correspondentes e avança para o próximo token para continuar a análise sintática.
 */
void parse_relacional() {
    parse_expressao_aritmetica();
    if ((token_atual->atomo == OPERADOR && strchr("><=!", token_atual->valor[0])) ||
        (token_atual->atomo == PALAVRA_RESERVADA && (strcmp(token_atual->valor, "in") == 0 || strcmp(token_atual->valor, "is") == 0))) {
        if (token_atual->atomo == OPERADOR) consome(OPERADOR);
        else consome(PALAVRA_RESERVADA);
        parse_expressao_aritmetica();
    }
}

/** @brief Função para analisar expressões lógicas, incluindo operadores lógicos "and" e "or". A função chama parse_relacional() para processar as expressões relacionais que compõem a expressão lógica e, em seguida, verifica se o próximo token é um operador lógico "and" ou "or". Se for, a função consome o operador lógico e chama parse_relacional() novamente para processar a próxima expressão relacional da expressão lógica. Essa abordagem permite a construção de expressões lógicas complexas, respeitando a associatividade correta dos operadores lógicos.
 * @return Não retorna um valor, mas em caso de expressão lógica inválida, a função imprime uma mensagem de erro detalhada indicando a linha e o token problemático, e encerra o programa. Se a expressão lógica for válida, a função processa os tokens correspondentes e avança para o próximo token para continuar a análise sintática.
 */
void parse_expressao() {
    parse_relacional();
    while (token_atual->atomo == PALAVRA_RESERVADA && (strcmp(token_atual->valor, "and") == 0 || strcmp(token_atual->valor, "or") == 0)) {
        consome(PALAVRA_RESERVADA);
        parse_relacional();
    }
}

/** @brief Função para analisar comandos e estruturas de controle em MiniPython, incluindo atribuições, chamadas de funções, estruturas condicionais (if, else) e loops (while, for). A função verifica o tipo do comando com base no token atual e chama as funções de análise apropriadas para processar cada tipo de comando. Em caso de comando inválido, a função implementa um mecanismo de recuperação simples, avançando para o próximo token para tentar continuar a análise sintática. Essa abordagem permite a construção de programas MiniPython complexos, respeitando a sintaxe correta para cada tipo de comando e estrutura de controle.
 * @return Não retorna um valor, mas em caso de comando inválido, a função implementa um mecanismo de recuperação simples, avançando para o próximo token para tentar continuar a análise sintática. Se o comando for válido, a função processa os tokens correspondentes e avança para o próximo token para continuar a análise sintática.
 */
void parse_comando() {
    if (token_atual->atomo == IDENTIFICADOR) {
        
        parse_fator(); 
        if (token_atual->atomo == OPERADOR && strcmp(token_atual->valor, "=") == 0) {
            consome(OPERADOR);
            parse_expressao();
        }
    } else if (token_atual->atomo == PALAVRA_RESERVADA) {
        if (strcmp(token_atual->valor, "print") == 0) {
            consome(PALAVRA_RESERVADA);
            parse_expressao();
            
            while (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, ",") == 0) {
                consome(DELIMITADOR);
                parse_expressao();
            }
        } else if (strcmp(token_atual->valor, "if") == 0) {
            consome(PALAVRA_RESERVADA);
            parse_expressao();
            consome(DELIMITADOR);
            parse_comando();

        } else if (strcmp(token_atual->valor, "else") == 0) {
            consome(PALAVRA_RESERVADA);
            consome(DELIMITADOR); 
            parse_comando();

        } else if (strcmp(token_atual->valor, "while") == 0) {
            consome(PALAVRA_RESERVADA);
            parse_expressao();
            consome(DELIMITADOR);
            parse_comando();

        } else if (strcmp(token_atual->valor, "for") == 0) {
            consome(PALAVRA_RESERVADA);
            consome(IDENTIFICADOR);
            consome(PALAVRA_RESERVADA); 
            parse_fator();             
            consome(DELIMITADOR);      
            parse_comando();

        } else {
            consome(PALAVRA_RESERVADA);
        }

    } else if (token_atual->atomo != EOS) {
        
        token_atual = obter_atomo(); 
    }
}

/** @brief Função para analisar o programa completo, processando cada comando até o final do arquivo fonte. A função chama parse_comando() em um loop enquanto o token atual não for EOS (End Of Stream), garantindo que todo o código fonte seja analisado sintaticamente. Em caso de erro sintático em qualquer comando, a função imprime uma mensagem de erro detalhada indicando a linha e o token problemático, e encerra o programa. Se o programa for sintaticamente correto, a função processa todo o arquivo até o final, validando cada comando e expressão conforme as regras da linguagem MiniPython.
 * @return Não retorna um valor, mas em caso de erro sintático em qualquer comando, a função imprime uma mensagem de erro detalhada indicando a linha e o token problemático, e encerra o programa. Se o programa for sintaticamente correto, a função processa todo o arquivo até o final, validando cada comando e expressão conforme as regras da linguagem MiniPython.
 */
void parse_programa() {
    while (token_atual->atomo != EOS) {
        parse_comando();
    }
}

int main(int argc, char *argv[]) {

    commentariesFile = fopen(FILEPATH_COMMENTARIES_DEDICATED_FILE, "w");
    identifiersFile = fopen(FILEPATH_IDENTIFIERS_DEDICATED_FILE, "w");
    operatorsFile = fopen(FILEPATH_OPERATORS_DEDICATED_FILE, "w");
    delimitersFile = fopen(FILEPATH_DELIMITERS_DEDICATED_FILE, "w");
    tokensFile = fopen(TOKENS_DEDICATED_FILE, "w");
    lexicalErrorsFile = fopen(LEXYCAL_ERRORS_DEDICATED_FILE, "w");
    sintaticErrorsFile = fopen(SINTATIC_ERRORS_DEDICATED_FILE, "w");
    
    if (argc < 2) {
        printf("Uso: %s <arquivo_fonte>\n", argv[0]);
        return 1;
    }
    
    input_ptr = fopen(argv[1], "r");
    
    if (!input_ptr) {
        printf("Erro ao abrir arquivo %s\n", argv[1]);
        return 1;
    }    
    
    token_atual = obter_atomo();
    
    parse_programa();
    
    fclose(input_ptr);
    
    return 0;

}
