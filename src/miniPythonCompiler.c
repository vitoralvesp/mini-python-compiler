#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Definição dos tipos de átomos reconhecidos pelo compilador
typedef enum {
    ERRO,
    IDENTIFICADOR,
    NUMERO,
    OPERADOR,
    DELIMITADOR,
    PALAVRA_RESERVADA,
    EOS // Fim do arquivo (End of Source)
} TAtomo;

// Estrutura para armazenar as informações de cada token
typedef struct {
    TAtomo atomo;
    char *valor;
    int linha;
} TToken;

// Variáveis globais para controle do arquivo e estado da análise
TToken *token_atual;
int linha_atual = 1;
FILE *input_ptr;

// Lista de palavras reservadas da linguagem MiniPython
const char *palavras_reservadas[] = {
    "return", "from", "while", "as", "elif", "with", "else", 
    "if", "break", "len", "input", "print", "exec", "in", 
    "raise", "continue", "range", "def", "for", "True", "False", "and", "or", "not", "is"
};

// Verifica se um lexema capturado é uma palavra reservada
int eh_palavra_reservada(char *s) {
    for (int i = 0; i < 25; i++) {
        if (strcmp(s, palavras_reservadas[i]) == 0) return 1;
    }
    return 0;
}

// Analisador Léxico: Implementação do autômato finito usando saltos (goto)
TToken* obter_atomo() {
    TToken *t = (TToken *)malloc(sizeof(TToken));
    char buffer[256];
    int idx = 0;
    int c = fgetc(input_ptr);

    // Ignora espaços e tabulações, rastreando quebras de linha
    while (c != EOF && isspace(c)) {
        if (c == '\n') linha_atual++;
        c = fgetc(input_ptr);
    }

    t->linha = linha_atual;

    // Detecta o fim do arquivo fonte
    if (c == EOF) {
        t->atomo = EOS;
        t->valor = strdup("EOF");
        return t;
    }

    // Ignora comentários iniciados por '#' até o final da linha
    if (c == '#') {
        while (c != '\n' && c != EOF) c = fgetc(input_ptr);
        if (c == '\n') linha_atual++;
        return obter_atomo();
    }

    // Encaminhamento para os estados do autômato léxico
    if (isdigit(c)) goto estado_numero;
    if (isalpha(c) || c == '_') goto estado_identificador;
    if (strchr("+-*/%=><!~", c)) goto estado_operador;
    if (strchr("(){}[],:.;\"", c)) goto estado_delimitador;

    // Erro léxico: caractere inválido encontrado no estado inicial
    printf("ERRO LÉXICO na linha %d: sequência '%c' inválida\n", linha_atual, c);
    exit(1);

estado_numero:
    // Captura sequência de dígitos decimais
    while (isdigit(c)) {
        buffer[idx++] = c;
        c = fgetc(input_ptr);
    }
    ungetc(c, input_ptr); // Devolve o caractere não numérico ao arquivo
    buffer[idx] = '\0';
    t->atomo = NUMERO;
    t->valor = strdup(buffer);
    return t;

estado_identificador:
    // Captura caracteres alfanuméricos e sublinhados
    while (isalnum(c) || c == '_') {
        buffer[idx++] = c;
        c = fgetc(input_ptr);
    }
    ungetc(c, input_ptr);
    buffer[idx] = '\0';
    // Define se o termo é uma palavra reservada ou identificador
    if (eh_palavra_reservada(buffer)) t->atomo = PALAVRA_RESERVADA;
    else t->atomo = IDENTIFICADOR;
    t->valor = strdup(buffer);
    return t;

estado_operador:
    buffer[0] = c;
    int prox = fgetc(input_ptr);
    // Verifica a formação de operadores compostos (==, !=, <=, >=, **)
    if ((c == '=' && prox == '=') || (c == '!' && prox == '=') || 
        (c == '<' && prox == '=') || (c == '>' && prox == '=') || 
        (c == '*' && prox == '*')) {
        buffer[1] = prox;
        buffer[2] = '\0';
    } else {
        ungetc(prox, input_ptr);
        buffer[1] = '\0';
    }
    t->atomo = OPERADOR;
    t->valor = strdup(buffer);
    return t;

estado_delimitador:
    // Captura pontuação e símbolos delimitadores
    buffer[0] = c;
    buffer[1] = '\0';
    t->atomo = DELIMITADOR;
    t->valor = strdup(buffer);
    return t;
}

// Analisador Sintático: Validação gramatical (Análise Descendente Recursiva)

void parse_expressao(); // Protótipo para recursão mútua

// Função que valida o token atual e avança a leitura no arquivo
void consome(TAtomo esperado) {
    if (token_atual->atomo == esperado) {
        char *nomes[] = {"ERRO", "IDENTIFICADOR", "NUMERO", "OPERADOR", "DELIMITADOR", "PALAVRA_RESERVADA", "EOS"};
        // Imprime a saída formatada: Linha# NomeToken | Valor
        printf("%d# %s | %s\n", token_atual->linha, nomes[token_atual->atomo], token_atual->valor);
        token_atual = obter_atomo();
    } else {
        printf("ERRO SINTÁTICO na linha %d: esperado token tipo %d, mas recebeu '%s'\n", token_atual->linha, esperado, token_atual->valor);
        exit(1);
    }
}

// Regra: fator -> ID | NUM | ( expressão )
void parse_fator() {
    if (token_atual->atomo == IDENTIFICADOR) {
        consome(IDENTIFICADOR);
    } else if (token_atual->atomo == NUMERO) {
        consome(NUMERO);
    } else if (strcmp(token_atual->valor, "(") == 0) {
        consome(DELIMITADOR);
        parse_expressao();
        if (strcmp(token_atual->valor, ")") == 0) {
            consome(DELIMITADOR);
        } else {
            printf("ERRO SINTÁTICO na linha %d: esperado ')'\n", token_atual->linha);
            exit(1);
        }
    } else {
        printf("ERRO SINTÁTICO na linha %d: fator inválido '%s'\n", token_atual->linha, token_atual->valor);
        exit(1);
    }
}

// Regra: potência -> fator [ ** potência ]
void parse_potencia() {
    parse_fator();
    if (token_atual->atomo == OPERADOR && strcmp(token_atual->valor, "**") == 0) {
        consome(OPERADOR);
        parse_potencia();
    }
}

// Regra: termo -> potência { (* | / | %) potência }
void parse_termo() {
    parse_potencia();
    while (token_atual->atomo == OPERADOR && 
          (token_atual->valor[0] == '*' || token_atual->valor[0] == '/' || token_atual->valor[0] == '%')) {
        consome(OPERADOR);
        parse_potencia();
    }
}

// Regra: expressão_aritmética -> termo { (+ | -) termo }
void parse_expressao_aritmetica() {
    parse_termo();
    while (token_atual->atomo == OPERADOR && (token_atual->valor[0] == '+' || token_atual->valor[0] == '-')) {
        consome(OPERADOR);
        parse_termo();
    }
}

// Regra: relacional -> expressão_aritmética [ OP_REL expressão_aritmética ]
void parse_relacional() {
    parse_expressao_aritmetica();
    // Trata operadores de comparação e palavras-chave relacionais (in, is)
    if ((token_atual->atomo == OPERADOR && strchr("><=", token_atual->valor[0])) ||
        (token_atual->atomo == PALAVRA_RESERVADA && (strcmp(token_atual->valor, "in") == 0 || strcmp(token_atual->valor, "is") == 0))) {
        if (token_atual->atomo == OPERADOR) consome(OPERADOR);
        else consome(PALAVRA_RESERVADA);
        parse_expressao_aritmetica();
    }
}

// Regra: expressão_and -> relacional { "and" relacional }
void parse_and() {
    parse_relacional();
    while (token_atual->atomo == PALAVRA_RESERVADA && strcmp(token_atual->valor, "and") == 0) {
        consome(PALAVRA_RESERVADA);
        parse_relacional();
    }
}

// Regra: expressão_or -> expressão_and { "or" expressão_and }
void parse_or() {
    parse_and();
    while (token_atual->atomo == PALAVRA_RESERVADA && strcmp(token_atual->valor, "or") == 0) {
        consome(PALAVRA_RESERVADA);
        parse_and();
    }
}

// Regra de entrada para qualquer expressão
void parse_expressao() {
    parse_or();
}

// Regra: comando -> atribuição | print | if | while | for
void parse_comando() {
    if (token_atual->atomo == IDENTIFICADOR) {
        consome(IDENTIFICADOR);
        // Atribuição simples: x = 10
        if (token_atual->atomo == OPERADOR && strcmp(token_atual->valor, "=") == 0) {
            consome(OPERADOR);
            parse_expressao();
        } 
        // Acesso a vetor: vetor[i] = 10
        else if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "[") == 0) {
            consome(DELIMITADOR);
            parse_expressao();
            consome(DELIMITADOR);
            if (token_atual->atomo == OPERADOR && strcmp(token_atual->valor, "=") == 0) {
                consome(OPERADOR);
                parse_expressao();
            }
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
            consome(DELIMITADOR); // espera ':'
            parse_comando();
        } else if (strcmp(token_atual->valor, "while") == 0) {
            consome(PALAVRA_RESERVADA);
            parse_expressao();
            consome(DELIMITADOR); // espera ':'
            parse_comando();
        } else if (strcmp(token_atual->valor, "for") == 0) {
            consome(PALAVRA_RESERVADA);
            consome(IDENTIFICADOR);
            consome(PALAVRA_RESERVADA); // in
            consome(PALAVRA_RESERVADA); // range
            consome(DELIMITADOR); // (
            parse_expressao();
            consome(DELIMITADOR); // )
            consome(DELIMITADOR); // :
            parse_comando();
        }
    }
}

// Inicia o processamento de comandos até o fim do arquivo fonte
void parse_programa() {
    while (token_atual->atomo != EOS) {
        parse_comando();
    }
}

int main(int argc, char *argv[]) {
    // Valida o recebimento do arquivo fonte via linha de comando
    if (argc < 2) {
        printf("Uso: %s <arquivo_fonte>\n", argv[0]);
        return 1;
    }

    input_ptr = fopen(argv[1], "r");
    if (!input_ptr) {
        printf("Erro ao abrir arquivo %s\n", argv[1]);
        return 1;
    }

    // Inicializa o primeiro token para disparar o processo sintático
    token_atual = obter_atomo();
    
    // Início da análise do programa principal
    parse_programa();

    fclose(input_ptr);
    return 0;
}
