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

/** @brief Estrutura de dados para representação dos átomos (tokens) identificados pelo Analisador Léxico. */
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

/** @brief Estrutura de dados para armazenar informações sobre cada token identificado pelo Analisador Léxico. */
typedef struct {
    TAtomo atomo;
    char *valor;
    int linha;
} TToken;

/* ── Nomes dos tipos de token para impressão ── */
static const char *NOMES_ATOMO[] = {
    "ERRO", "IDENTIFICADOR", "NUMERO", "OPERADOR",
    "DELIMITADOR", "PALAVRA_RESERVADA", "STRING_LITERAL", "EOS"
};

/* ── Variáveis Globais ── */
TToken *token_atual;
int     linha_atual = 1;

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

/** @brief Verifica se uma string é palavra reservada. */
int eh_palavra_reservada(const char *s) {
    for (int i = 0; i < NUM_PALAVRAS_RESERVADAS; i++)
        if (strcmp(s, palavras_reservadas[i]) == 0) return 1;
    return 0;
}

/** @brief Limpa (zera) um arquivo de log. */
int clearFile(const char *filePath) {
    FILE *f = fopen(filePath, "w");
    if (!f) { perror("Erro ao limpar o arquivo"); return 1; }
    fclose(f);
    return 0;
}

/* ──────────────────────────────────────────────────────────────
   REGISTRO DE ERROS
   Todas as mensagens vão para: terminal + arquivo dedicado.
   Os arquivos já estão abertos (modo "w" no main); usamos "a"
   apenas para garantir que chamadas sucessivas não se percam.
   ────────────────────────────────────────────────────────────── */

/** @brief Registra um erro léxico no terminal e no arquivo dedicado, depois encerra. */
void registrar_erro_lexico(int linha, const char *contexto) {
    fprintf(stdout, "ERRO LEXICO na linha %d: sequencia '%s' invalida\n", linha, contexto);
    fflush(stdout);

    /* Re-abre em modo "a" para acrescentar (o arquivo foi criado/zerado no main) */
    FILE *f = fopen(LEXYCAL_ERRORS_DEDICATED_FILE, "a");
    if (f) {
        fprintf(f, "ERRO LEXICO na linha %d: sequencia '%s' invalida\n", linha, contexto);
        fclose(f);
    } else {
        perror("Nao foi possivel abrir o arquivo de erros lexicos");
    }
    exit(1);
}

/** @brief Registra um erro sintático no terminal e no arquivo dedicado, depois encerra. */
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

/* ──────────────────────────────────────────────────────────────
   ANALISADOR LÉXICO
   ────────────────────────────────────────────────────────────── */

/** @brief Lê o arquivo fonte e retorna o próximo token. */
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

    /* Fim de arquivo */
    if (c == EOF) {
        t->atomo = EOS;
        t->valor = strdup("EOF");
        return t;
    }

    /* ── Comentário de linha ── */
    if (c == '#') {
        /* Acumula o comentário para registro */
        buffer[idx++] = (char)c;
        c = fgetc(input_ptr);
        while (c != '\n' && c != EOF) {
            if (idx < (int)sizeof(buffer) - 1) buffer[idx++] = (char)c;
            c = fgetc(input_ptr);
        }
        buffer[idx] = '\0';
        if (c == '\n') linha_atual++;

        /* Registra no arquivo de comentários */
        FILE *cf = fopen(FILEPATH_COMMENTARIES_DEDICATED_FILE, "a");
        if (cf) { fprintf(cf, "linha %d: %s\n", t->linha, buffer); fclose(cf); }

        free(t);
        return obter_atomo();   /* descarta e retorna próximo token */
    }

    /* ── String literal ── */
    if (c == '"' || c == '\'') {
        int quote = c;
        buffer[idx++] = (char)c;
        c = fgetc(input_ptr);
        while (c != quote && c != EOF) {
            if (c == '\n') {
                /* String não fechada na mesma linha — erro léxico */
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

    /* ── Número ── */
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

    /* ── Identificador / Palavra reservada ── */
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

    /* ── Operador ── */
    if (strchr("+-*/%=><!~", c)) {
        buffer[idx++] = (char)c;
        int prox = fgetc(input_ptr);

        /* Operadores de dois caracteres */
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

    /* ── Delimitador ── */
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

    /* ── Caractere inválido — ERRO LÉXICO ── */
    buffer[0] = (char)c;
    buffer[1] = '\0';
    registrar_erro_lexico(linha_atual, buffer);

    /* Nunca alcançado, mas necessário para o compilador */
    t->atomo = ERRO;
    t->valor = strdup(buffer);
    return t;
}

/* ──────────────────────────────────────────────────────────────
   ANALISADOR SINTÁTICO
   ────────────────────────────────────────────────────────────── */

void parse_expressao(void);   /* forward declaration */

/** @brief Consome o token atual se corresponder ao tipo esperado; caso contrário, registra erro sintático. */
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
            NOMES_ATOMO[esperado],   /* descrição legível do esperado */
            token_atual->valor       /* o que foi encontrado          */
        );
    }
}

/** @brief Versão de consome que verifica tipo E valor exato do token (para delimitadores e palavras reservadas específicas). */
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
        /* Mensagem informativa: mostra o valor esperado entre aspas */
        char esperado_str[128];
        snprintf(esperado_str, sizeof(esperado_str), "'%s'", valor_esperado);
        registrar_erro_sintatico(
            token_atual->linha,
            esperado_str,
            token_atual->valor
        );
    }
}

/** @brief Analisa um fator: literal, identificador, chamada, indexação ou subexpressão. */
void parse_fator(void) {
    if (token_atual->atomo == IDENTIFICADOR) {
        consome(IDENTIFICADOR);

        /* Indexação: id[ expr ] */
        if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "[") == 0) {
            consome_valor(DELIMITADOR, "[");
            parse_expressao();
            consome_valor(DELIMITADOR, "]");

        /* Chamada de função: id( [args] ) */
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

    /* Palavra reservada usada como valor (True, False, None, not …) */
    } else if (token_atual->atomo == PALAVRA_RESERVADA) {
        consome(PALAVRA_RESERVADA);
        /* Chamada de built-in: print(...), len(...), range(...) etc. */
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

    /* Lista literal: [ expr, … ] */
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

    /* Subexpressão: ( expr ) */
    } else if (token_atual->atomo == DELIMITADOR && strcmp(token_atual->valor, "(") == 0) {
        consome_valor(DELIMITADOR, "(");
        parse_expressao();
        consome_valor(DELIMITADOR, ")");

    } else {
        /* Nenhum fator válido encontrado */
        registrar_erro_sintatico(
            token_atual->linha,
            "fator (identificador, numero, string ou '(')",
            token_atual->valor
        );
    }
}

/** @brief Potenciação (associatividade direita): fator ** potencia */
void parse_potencia(void) {
    parse_fator();
    if (token_atual->atomo == OPERADOR && strcmp(token_atual->valor, "**") == 0) {
        consome(OPERADOR);
        parse_potencia();
    }
}

/** @brief Termos: potencia { ('*' | '/' | '%') potencia } */
void parse_termo(void) {
    parse_potencia();
    while (token_atual->atomo == OPERADOR &&
           strlen(token_atual->valor) == 1 &&
           strchr("*/%", token_atual->valor[0])) {
        consome(OPERADOR);
        parse_potencia();
    }
}

/** @brief Expressão aritmética: termo { ('+' | '-') termo } */
void parse_expressao_aritmetica(void) {
    parse_termo();
    while (token_atual->atomo == OPERADOR &&
           strlen(token_atual->valor) == 1 &&
           strchr("+-", token_atual->valor[0])) {
        consome(OPERADOR);
        parse_termo();
    }
}

/** @brief Expressão relacional: aritm { op_relacional aritm } */
void parse_relacional(void) {
    parse_expressao_aritmetica();

    /* Suporta: > < >= <= == != <> in is (e "not in" seria extensão futura) */
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
        else                                consome(PALAVRA_RESERVADA);

        parse_expressao_aritmetica();
    }
}

/** @brief Expressão lógica completa: relacional { ('and' | 'or') relacional } */
void parse_expressao(void) {
    parse_relacional();
    while (token_atual->atomo == PALAVRA_RESERVADA &&
           (strcmp(token_atual->valor, "and") == 0 ||
            strcmp(token_atual->valor, "or")  == 0)) {
        consome(PALAVRA_RESERVADA);
        parse_relacional();
    }
}

/* Forward declaration para blocos recorrentes */
void parse_bloco(void);
void parse_comando(void);

/** @brief Analisa um bloco de comandos.
 *  Em MiniPython simplificado tratamos cada linha como um comando independente;
 *  após ':' esperamos pelo menos um comando (sem indentação formal).
 */
void parse_bloco(void) {
    parse_comando();
}

/** @brief Analisa um único comando. */
void parse_comando(void) {

    if (token_atual->atomo == EOS) return;

    /* ── Atribuição ou chamada via identificador ── */
    if (token_atual->atomo == IDENTIFICADOR) {
        parse_fator();   /* consome id (e possível indexação/chamada) */

        if (token_atual->atomo == OPERADOR && strcmp(token_atual->valor, "=") == 0) {
            consome_valor(OPERADOR, "=");
            parse_expressao();
        }
        /* Caso contrário foi só uma chamada de função — já consumida em parse_fator */

    /* ── Comandos iniciados por palavra reservada ── */
    } else if (token_atual->atomo == PALAVRA_RESERVADA) {

        /* print expr [, expr]* */
        if (strcmp(token_atual->valor, "print") == 0) {
            consome(PALAVRA_RESERVADA);
            /* print pode vir sem parênteses (Python 2) ou com (Python 3) */
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

        /* return [expr] */
        } else if (strcmp(token_atual->valor, "return") == 0) {
            consome(PALAVRA_RESERVADA);
            if (token_atual->atomo != EOS &&
                !(token_atual->atomo == PALAVRA_RESERVADA &&
                  (strcmp(token_atual->valor, "else")  == 0 ||
                   strcmp(token_atual->valor, "elif")  == 0 ||
                   strcmp(token_atual->valor, "def")   == 0))) {
                parse_expressao();
            }

        /* if expr : bloco [elif/else] */
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

        /* else : bloco  (sozinho, para recuperação de erro) */
        } else if (strcmp(token_atual->valor, "else") == 0) {
            consome(PALAVRA_RESERVADA);
            consome_valor(DELIMITADOR, ":");
            parse_bloco();

        /* while expr : bloco */
        } else if (strcmp(token_atual->valor, "while") == 0) {
            consome(PALAVRA_RESERVADA);
            parse_expressao();
            consome_valor(DELIMITADOR, ":");
            parse_bloco();

        /* for id in fator : bloco */
        } else if (strcmp(token_atual->valor, "for") == 0) {
            consome(PALAVRA_RESERVADA);
            consome(IDENTIFICADOR);
            consome_valor(PALAVRA_RESERVADA, "in");
            parse_fator();
            consome_valor(DELIMITADOR, ":");
            parse_bloco();

        /* def id ( [params] ) : bloco */
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

        /* break / continue / pass — palavras reservadas sem corpo */
        } else if (strcmp(token_atual->valor, "break")    == 0 ||
                   strcmp(token_atual->valor, "continue") == 0) {
            consome(PALAVRA_RESERVADA);

        /* raise expr */
        } else if (strcmp(token_atual->valor, "raise") == 0) {
            consome(PALAVRA_RESERVADA);
            parse_expressao();

        /* Qualquer outra palavra reservada usada como expressão (len, range…) */
        } else {
            parse_fator();
        }

    /* ── Token inesperado no início de um comando ── */
    } else if (token_atual->atomo != EOS) {
        registrar_erro_sintatico(
            token_atual->linha,
            "inicio de comando valido (identificador ou palavra reservada)",
            token_atual->valor
        );
    }
}

/** @brief Analisa o programa completo: sequência de comandos até EOS. */
void parse_programa(void) {
    while (token_atual->atomo != EOS) {
        parse_comando();
    }
    printf("Analise sintatica concluida com sucesso.\n");
}

/* ──────────────────────────────────────────────────────────────
   FUNÇÃO PRINCIPAL
   ────────────────────────────────────────────────────────────── */
int main(int argc, char *argv[]) {

    /* Cria/zera todos os arquivos de log */
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

    /* Inicia análise */
    token_atual = obter_atomo();
    parse_programa();

    fclose(input_ptr);
    return 0;
}