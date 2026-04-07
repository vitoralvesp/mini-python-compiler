#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Definição dos tipos de átomos que compõem o vocabulário terminal da linguagem
typedef enum {
    ERRO,
    IDENTIFICADOR,
    NUMERO,
    OPERADOR,
    DELIMITADOR,
    PALAVRA_RESERVADA,
    STRING_LITERAL,
    EOS // Representa o fim do fluxo de caracteres do arquivo fonte
} TAtomo;

// Estrutura para carregar os metadados de cada token identificado pelo léxico
typedef struct {
    TAtomo atomo;
    char *valor;
    int linha;
} TToken;

// Ponteiros e variáveis globais para controle da leitura e estado do parser
TToken *token_atual;
int linha_atual = 1;
FILE *input_ptr;

// Vetor de palavras-chave para diferenciação entre nomes de variáveis e comandos
const char *palavras_reservadas[] = {
    "return", "from", "while", "as", "elif", "with", "else", "if", 
    "break", "len", "input", "print", "exec", "in", "raise", 
    "continue", "range", "def", "for", "True", "False", "and", "or", "not", "is"
};

// Realiza busca linear para validar se um identificador é uma palavra protegida
int eh_palavra_reservada(char *s) {
    for (int i = 0; i < 25; i++) {
        if (strcmp(s, palavras_reservadas[i]) == 0) return 1;
    }
    return 0;
}

// Analisador Léxico: Implementa um Automato Finito para segmentar o código em tokens
TToken* obter_atomo() {
    TToken *t = (TToken *)malloc(sizeof(TToken));
    char buffer[1024];
    int idx = 0;
    int c = fgetc(input_ptr);

    // Salta espaços e tabulações, mantendo o rastreio da linha atual para relatórios
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

    // Tratamento de comentários de linha: ignora o fluxo até encontrar o caractere de nova linha
    if (c == '#') {
        while (c != '\n' && c != EOF) c = fgetc(input_ptr);
        if (c == '\n') linha_atual++;
        return obter_atomo();
    }

    // Identificação de strings: lê o conteúdo entre aspas como um único bloco literal
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

    // Transições de estados baseadas no primeiro caractere capturado
    if (isdigit(c)) goto estado_numero;
    if (isalpha(c) || c == '_') goto estado_identificador;
    if (strchr("+-*/%=><!~", c)) goto estado_operador;
    if (strchr("(){}[],:.;", c)) goto estado_delimitador;

    // Reporte de erro léxico para caracteres fora do alfabeto da linguagem
    printf("ERRO LEXICO na linha %d: sequencia '%c' invalida\n", linha_atual, c);
    exit(1);

estado_numero:
    // Captura sequência de dígitos decimais formando um átomo numérico
    while (isdigit(c)) {
        buffer[idx++] = c;
        c = fgetc(input_ptr);
    }
    ungetc(c, input_ptr); // Devolve o caractere excedente ao buffer de leitura
    buffer[idx] = '\0';
    t->atomo = NUMERO;
    t->valor = strdup(buffer);
    return t;

estado_identificador:
    // Agrupamento de caracteres alfanuméricos para formação de nomes
    while (isalnum(c) || c == '_') {
        buffer[idx++] = c;
        c = fgetc(input_ptr);
    }
    ungetc(c, input_ptr);
    buffer[idx] = '\0';
    // Classifica o identificador após a leitura completa do buffer
    if (eh_palavra_reservada(buffer)) t->atomo = PALAVRA_RESERVADA;
    else t->atomo = IDENTIFICADOR;
    t->valor = strdup(buffer);
    return t;

estado_operador:
    buffer[idx++] = c;
    int prox = fgetc(input_ptr);
    // Verificação de operadores compostos (2 caracteres) para evitar ambiguidade
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
    // Captura de símbolos de pontuação e delimitadores sintáticos
    buffer[0] = c;
    buffer[1] = '\0';
    t->atomo = DELIMITADOR;
    t->valor = strdup(buffer);
    return t;
}

// --- ANALISADOR SINTÁTICO ---

void parse_expressao();

// Ponte entre o léxico e o sintático: valida o token atual e avança a leitura
void consome(TAtomo esperado) {
    if (token_atual->atomo == esperado) {
        char *nomes[] = {"ERRO", "IDENTIFICADOR", "NUMERO", "OPERADOR", "DELIMITADOR", "PALAVRA_RESERVADA", "STRING_LITERAL", "EOS"};
        // Impressão dos tokens consumidos para acompanhamento da análise
        printf("%d# %s | %s\n", token_atual->linha, nomes[token_atual->atomo], token_atual->valor);
        token_atual = obter_atomo();
    } else {
        printf("ERRO SINTATICO na linha %d: esperado token tipo %d, mas recebeu '%s'\n", token_atual->linha, esperado, token_atual->valor);
        exit(1);
    }
}

// Regra de Fatores: Trata as unidades básicas que compõem uma expressão
void parse_fator() {
    if (token_atual->atomo == IDENTIFICADOR) {
        consome(IDENTIFICADOR);
        // Implementação de indexação de vetores (colchetes após identificador)
        if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "[") == 0) {
            consome(DELIMITADOR);
            parse_expressao();
            consome(DELIMITADOR); 
        }
        // Chamadas de funções com parênteses (ex: range, len)
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
        // Tratamento de funções reservadas seguidas por parênteses
        if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "(") == 0) {
            consome(DELIMITADOR);
            if (strcmp(token_atual->valor, ")") != 0) parse_expressao();
            consome(DELIMITADOR);
        }
    } else if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "[") == 0) {
        // Regra para reconhecimento de listas literais entre colchetes
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
        // Expressões entre parênteses para alteração de precedência
        consome(DELIMITADOR);
        parse_expressao();
        consome(DELIMITADOR);
    } else {
        printf("ERRO SINTATICO na linha %d: fator invalido '%s'\n", token_atual->linha, token_atual->valor);
        exit(1);
    }
}

// Ordem de precedência: Funções matemáticas recursivas respeitando a hierarquia
void parse_potencia() {
    parse_fator();
    if (token_atual->atomo == OPERADOR && strcmp(token_atual->valor, "**") == 0) {
        consome(OPERADOR);
        parse_potencia();
    }
}

void parse_termo() {
    parse_potencia();
    while (token_atual->atomo == OPERADOR && strchr("*/%", token_atual->valor[0]) && strlen(token_atual->valor) == 1) {
        consome(OPERADOR);
        parse_potencia();
    }
}

void parse_expressao_aritmetica() {
    parse_termo();
    while (token_atual->atomo == OPERADOR && strchr("+-", token_atual->valor[0])) {
        consome(OPERADOR);
        parse_termo();
    }
}

// Avaliação de operadores de comparação e lógica relacional
void parse_relacional() {
    parse_expressao_aritmetica();
    if ((token_atual->atomo == OPERADOR && strchr("><=!", token_atual->valor[0])) ||
        (token_atual->atomo == PALAVRA_RESERVADA && (strcmp(token_atual->valor, "in") == 0 || strcmp(token_atual->valor, "is") == 0))) {
        if (token_atual->atomo == OPERADOR) consome(OPERADOR);
        else consome(PALAVRA_RESERVADA);
        parse_expressao_aritmetica();
    }
}

// Expressão lógica final agregando operadores AND e OR
void parse_expressao() {
    parse_relacional();
    while (token_atual->atomo == PALAVRA_RESERVADA && (strcmp(token_atual->valor, "and") == 0 || strcmp(token_atual->valor, "or") == 0)) {
        consome(PALAVRA_RESERVADA);
        parse_relacional();
    }
}

// Parser de comandos: Gerencia as estruturas de controle e atribuições
void parse_comando() {
    if (token_atual->atomo == IDENTIFICADOR) {
        // Validação de atribuições simples ou em vetores
        parse_fator(); 
        if (token_atual->atomo == OPERADOR && strcmp(token_atual->valor, "=") == 0) {
            consome(OPERADOR);
            parse_expressao();
        }
    } else if (token_atual->atomo == PALAVRA_RESERVADA) {
        if (strcmp(token_atual->valor, "print") == 0) {
            consome(PALAVRA_RESERVADA);
            parse_expressao();
            // Permite impressão de múltiplos valores separados por vírgulas
            while (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, ",") == 0) {
                consome(DELIMITADOR);
                parse_expressao();
            }
        } else if (strcmp(token_atual->valor, "if") == 0) {
            consome(PALAVRA_RESERVADA);
            parse_expressao();
            consome(DELIMITADOR); // Caractere ':' esperado em estruturas Python
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
            consome(PALAVRA_RESERVADA); // palavra 'in'
            parse_fator();             // chamada 'range()'
            consome(DELIMITADOR);      // caractere ':'
            parse_comando();
        } else {
            consome(PALAVRA_RESERVADA);
        }
    } else if (token_atual->atomo != EOS) {
        // Mecanismo de recuperação simples: avança o token se nenhum comando for iniciado
        token_atual = obter_atomo(); 
    }
}

// Ponto de entrada da gramática: Processa o arquivo até a detecção do fim do código
void parse_programa() {
    while (token_atual->atomo != EOS) {
        parse_comando();
    }
}

int main(int argc, char *argv[]) {
    // Verificação de parâmetros de entrada via linha de comando
    if (argc < 2) {
        printf("Uso: %s <arquivo_fonte>\n", argv[0]);
        return 1;
    }
    input_ptr = fopen(argv[1], "r");
    if (!input_ptr) {
        printf("Erro ao abrir arquivo %s\n", argv[1]);
        return 1;
    }
    
    // Inicializa o processo buscando o primeiro átomo e disparando o parser
    token_atual = obter_atomo();
    parse_programa();
    
    fclose(input_ptr);
    return 0;
}
