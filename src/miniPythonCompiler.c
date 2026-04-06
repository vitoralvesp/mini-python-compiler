#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wctype.h>

// ******************************************************
// ETAPA 0: RECEBIMENTO DO ARQUIVO 
// ******************************************************

#define FILE_OUTPUT_PATH "./output/"

#define FILE_TEMP_PATH "../temp/"

#define FILEPATH_COMMENTARIES_DEDICATED_FILE "./logs/commentariesIdentified.txt"

#define FILEPATH_IDENTIFIERS_DEDICATED_FILE "./logs/identifiersIdentified.txt"

#define FILEPATH_OPERATORS_DEDICATED_FILE "./logs/operatorsIdentified.txt"

#define FILEPATH_DELIMITERS_DEDICATED_FILE "./logs/delimitersIdentified.txt"

#define TOKENS_DEDICATED_FILE "./logs/tokensIdentified.txt"

#define LEXYCAL_ERRORS_DEDICATED_FILE "./logs/lexycalErrorsIdentified.txt"

#define SINTATIC_ERRORS_DEDICATED_FILE "./logs/sintaticErrorsIdentified.txt"

typedef struct Token {
    int id;
    char *type;
    char *value;
} Token;

typedef struct SymbolTable {
    int id[256];
    char *name[256];
    char *type[256];
} SymbolTable;

typedef struct IdentifiedErrors {
    int *id;
    char *errorType;
    char *errorMessage;
} IdentifiedErrors;

typedef struct ASTNode {
    Token token;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

// GLOBAL
int id = 1;
int i = 0;
int j = 0;
int k = 0;
int l = 0;
int m = 0;

Token *CurrentToken;
Token *LookaheadToken;
IdentifiedErrors *identifiedErrors;
FILE *sintaticErrorsIdentified;

int findClosureCharacter(char *string, char characterToLookFor, int length);
int readFile(int argc, char *argv[]);
FILE *createFileCopy(int argc, char *argv[]);
int removeCharacters(char *fileCopyPath, char characterToRemove);
int slice(char *newString, char *string, char characterToLookFor, int length);
Token *getNextToken(FILE *copy);
int insertTokenIntoSymbolTable(Token token, SymbolTable *symbolTable);
int fillSymbolTable(SymbolTable *symbolTable);
int printSymbolTable(SymbolTable *symbolTable);
int clearFile(const char *filePath);
int isValidChar(char c);
ASTNode *createASTNode(Token token, ASTNode *left, ASTNode *right);
int isRelOp(Token token);
void sync(FILE *file);
ASTNode *parseProgram(FILE *file);
ASTNode *parseStatement(FILE *file);
ASTNode *parseFactor(FILE *file);
ASTNode *parseUnary(FILE *file);
ASTNode *parsePower(FILE *file);
ASTNode *parseTerm(FILE *file);
ASTNode *parseArithExpr(FILE *file);
ASTNode *parseRelExpr(FILE *file);
ASTNode *parseNotExpr(FILE *file);
ASTNode *parseAndExpr(FILE *file);
ASTNode *parseOrExpr(FILE *file);
ASTNode *parseExpression(FILE *file);

int main(int argc, char *argv[]) {

    if (readFile(argc, argv) != 0) {
        puts("[ ERRO ] Não foi possível abrir/ler o arquivo de entrada.");
        return 1;
    } else {
        puts("[ SUCESSO ] Arquivo lido com sucesso!");
    }

    FILE *copy = createFileCopy(argc, argv);

    if (copy == NULL) {
        puts("[ ERRO ] Não foi possível criar uma cópia do arquivo de entrada.");
        return 1;
    } else {
        puts("[ SUCESSO ] Arquivo copiado em '/output' com sucesso!");
    }

    char fileCopyPath[256];
    char fileName[256];

    identifiedErrors = (IdentifiedErrors *)malloc(sizeof(IdentifiedErrors));
    identifiedErrors->errorType = "LEXYCAL";

    int result = slice(fileName, argv[1], '/', strlen(argv[1]));

    snprintf(fileCopyPath, sizeof(fileCopyPath), "%s%s", FILE_OUTPUT_PATH, fileName);

    if (result == -1) {
        puts("[ ERRO ] Não foi possível extrair o nome do arquivo de entrada para o processo de remoção de caracteres.");
        return -1;
    }

    printf("Caminho do arquivo copiado: %s\n", fileCopyPath);

    clearFile(FILEPATH_COMMENTARIES_DEDICATED_FILE);
    clearFile(FILEPATH_IDENTIFIERS_DEDICATED_FILE);
    clearFile(FILEPATH_OPERATORS_DEDICATED_FILE);
    clearFile(FILEPATH_DELIMITERS_DEDICATED_FILE);
    clearFile(TOKENS_DEDICATED_FILE);
    clearFile(LEXYCAL_ERRORS_DEDICATED_FILE);
    clearFile(SINTATIC_ERRORS_DEDICATED_FILE);

    copy = fopen(fileCopyPath, "rb");

    if (copy == NULL) {
        puts("[ ERRO ] Não foi possível abrir o arquivo copiado para análise léxica.");
        return 1;
    }

    SymbolTable *symbolTable = (SymbolTable *)malloc(sizeof(SymbolTable));
    memset(symbolTable, 0, sizeof(SymbolTable));

    while (1) {
        Token *t = getNextToken(copy);
        if (t == NULL) break;
        if (t->value != NULL && t->type != NULL) {
            insertTokenIntoSymbolTable(*t, symbolTable);
        }
    }

    fclose(copy);

    puts("[ SUCESSO ] Prosseguindo para a análise léxica!");
    printf("\n");
    printSymbolTable(symbolTable);
    printf("\n");
    puts("[ SUCESSO ] Análise léxica concluída com sucesso!");
    puts("[ SUCESSO ] Prosseguindo para a análise sintática!");

    int didCommentsRemovalProcessSucceed = removeCharacters(fileCopyPath, '#');

    if (didCommentsRemovalProcessSucceed != 0) {
        puts("[ ERRO ] Não foi possível remover os comentários do arquivo copiado.");
    } else {
        puts("[ SUCESSO ] Todos os comentários foram removidos do arquivo copiado com sucesso!");
    }

    copy = fopen(fileCopyPath, "rb");
    sintaticErrorsIdentified = fopen(SINTATIC_ERRORS_DEDICATED_FILE, "a");

    if (copy == NULL || sintaticErrorsIdentified == NULL) {
        printf("Erro ao abrir arquivo\n");
        return 1;
    }

    // Resetar o contador de linha para a análise sintática
    id = 1;

    CurrentToken = getNextToken(copy);

    if (CurrentToken == NULL || CurrentToken->value == NULL) {
        printf("Erro: arquivo vazio ou sem tokens válidos\n");
        fclose(copy);
        fclose(sintaticErrorsIdentified);
        return 1;
    }

    ASTNode *root = parseProgram(copy);

    if (CurrentToken != NULL && CurrentToken->value != NULL) {
        char lineError[256];
        snprintf(lineError, sizeof(lineError),
            "Erro sintático na linha %d: token inesperado '%s'\n",
            CurrentToken->id,
            CurrentToken->value);
        fputs(lineError, sintaticErrorsIdentified);
        printf("[ ERRO ] %s", lineError);
    }

    printf("[ SUCESSO ] Análise sintática concluída!\n");

    fclose(copy);
    fclose(sintaticErrorsIdentified);

    return 0;
}

int readFile(int argc, char *argv[]) {
    if (argc < 2) return 1;
    FILE *file = fopen(argv[1], "r");
    if (!file) return 1;
    fclose(file);
    return 0;
}

int isValidChar(char c) {
    return iswalpha(c) ||
           iswdigit(c) ||
           c == '_'  ||
           c == '#'  ||
           c == '+'  ||
           c == '-'  ||
           c == '*'  ||
           c == '/'  ||
           c == '~'  ||
           c == '%'  ||
           c == '='  ||
           c == '<'  ||
           c == '>'  ||
           c == '!'  ||
           c == '('  ||
           c == ')'  ||
           c == '{'  ||
           c == '}'  ||
           c == '['  ||
           c == ']'  ||
           c == ','  ||
           c == ';'  ||
           c == ':'  ||
           c == '.'  ||
           c == '\"' ||
           c == '\'' ||
           c == ' '  ||
           c == '\t' ||
           c == '\n' ||
           c == '\0' ||
           (c & 0x80);
}

int clearFile(const char *filePath) {
    FILE *file = fopen(filePath, "w");
    if (!file) {
        perror("Erro ao limpar o arquivo");
        return 1;
    }
    fclose(file);
    return 0;
}

FILE *createFileCopy(int argc, char *argv[]) {
    if (argc < 2) return NULL;

    const char *base = strrchr(argv[1], '/');
    base = base ? base + 1 : argv[1];

    char outputFilePath[256];
    snprintf(outputFilePath, sizeof(outputFilePath), "%s%s", FILE_OUTPUT_PATH, argv[1] + strlen("input/"));

    FILE *file = fopen(argv[1], "rb");
    FILE *copiedFile = fopen(outputFilePath, "wb");

    if (!file || !copiedFile) {
        if (file)       fclose(file);
        if (copiedFile) fclose(copiedFile);
        return NULL;
    }

    int c;
    while ((c = fgetc(file)) != EOF) {
        fputc(c, copiedFile);
    }

    fclose(file);
    fclose(copiedFile);

    // Retorna NULL como sinal de "ok mas fechado" — o chamador reabre pelo path
    // Mantemos compatibilidade com o código original que checa != NULL
    return (FILE *)1; // valor sentinela; o main usa fileCopyPath para reabrir
}

int findClosureCharacter(char *string, char characterToLookFor, int length) {
    for (int i = 0; i < length; i++) {
        if (string[i] == characterToLookFor) return 0;
    }
    return 1;
}

int slice(char *newString, char *string, char characterToLookFor, int length) {
    int k = -1;
    for (int i = 0; i < length; i++) {
        if (string[i] == characterToLookFor) { k = i; break; }
    }
    if (k == -1) {
        // Sem separador: copia a string inteira
        strncpy(newString, string, length);
        newString[length] = '\0';
        return 0;
    }
    k = k + 1;
    int j = 0;
    for (j = 0; k + j < length; j++) {
        newString[j] = string[k + j];
    }
    newString[j] = '\0';
    return 0;
}

int removeCharacters(char *fileCopyPath, char characterToRemove) {
    FILE *copy = fopen(fileCopyPath, "rb");
    if (!copy) return 1;

    FILE *tempFile = fopen("./temp/removeCharactersTempFile.txt", "wb");
    if (!tempFile) {
        fclose(copy);
        return 1;
    }

    int c;
    int inComment = 0;

    while ((c = fgetc(copy)) != EOF) {
        if (c == characterToRemove) {
            inComment = 1;
        }
        if (inComment) {
            if (c == '\n') {
                inComment = 0;
                fputc(c, tempFile); // preserva a quebra de linha
            }
            // pula o caractere de comentário e tudo até \n
        } else {
            fputc(c, tempFile);
        }
    }

    fclose(copy);
    fclose(tempFile);

    if (remove(fileCopyPath) != 0) {
        fprintf(stderr, "Erro ao remover arquivo original\n");
    }
    if (rename("./temp/removeCharactersTempFile.txt", fileCopyPath) != 0) {
        fprintf(stderr, "Erro ao renomear arquivo temporário\n");
    }

    return 0;
}

Token *getNextToken(FILE *copy) {

    FILE *commentariesIdentified  = fopen(FILEPATH_COMMENTARIES_DEDICATED_FILE, "a");
    FILE *identifiersIdentified   = fopen(FILEPATH_IDENTIFIERS_DEDICATED_FILE,  "a");
    FILE *operatorsIdentified     = fopen(FILEPATH_OPERATORS_DEDICATED_FILE,     "a");
    FILE *delimitersIdentified    = fopen(FILEPATH_DELIMITERS_DEDICATED_FILE,    "a");
    FILE *tokensIdentified        = fopen(TOKENS_DEDICATED_FILE,                 "a");
    FILE *lexycalErrorsIdentified = fopen(LEXYCAL_ERRORS_DEDICATED_FILE,         "a");

    if (!copy || !commentariesIdentified || !identifiersIdentified ||
        !operatorsIdentified || !delimitersIdentified ||
        !tokensIdentified || !lexycalErrorsIdentified) {
        return NULL;
    }

    // FIX: declarar todas as variáveis ANTES de qualquer goto
    Token *token = (Token *)malloc(sizeof(Token));
    token->id    = 0;
    token->type  = NULL;
    token->value = NULL;

    char commentariesBuffer[256];
    char identifiersBuffer[256];
    char operatorsBuffer[256];
    char delimitersBuffer[256];
    char numberBuffer[256];

    commentariesBuffer[0] = '\0';
    identifiersBuffer[0]  = '\0';
    operatorsBuffer[0]    = '\0';
    delimitersBuffer[0]   = '\0';
    numberBuffer[0]       = '\0';

    // FIX: variáveis que antes ficavam dentro de blocos após goto
    int hasDot  = 0;
    int hasExp  = 0;
    int next    = 0;

    int c = fgetc(copy);
    if (c == EOF) goto end;

    // Pula espaços/tabs/newlines
    while (c == ' ' || c == '\n' || c == '\t') {
        if (c == '\n') id++;
        c = fgetc(copy);
        if (c == EOF) goto end;
    }

    if (c == EOF) goto end;

    if (c == '\n') id++;

    // ── Números e Identificadores ──────────────────────────────────────────
    if (isdigit(c) || isalpha(c) || c == '_') {

        if (isdigit(c)) {
            char buffer[256];
            int idx = 0;
            buffer[idx++] = c;
            int nx = fgetc(copy);
            int hasLetter = 0;

            while (nx != EOF && (isalnum(nx) || nx == '_')) {
                if (isalpha(nx) || nx == '_') hasLetter = 1;
                buffer[idx++] = nx;
                nx = fgetc(copy);
            }
            buffer[idx] = '\0';

            if (nx != EOF) ungetc(nx, copy);

            if (hasLetter) {
                char lineError[256];
                snprintf(lineError, sizeof(lineError),
                    "Erro léxico na linha %d: identificador inválido '%s'\n", id, buffer);
                fputs(lineError, lexycalErrorsIdentified);
                printf("[ ERRO ] %s\n", lineError);
                goto end;
            }

            // É um número puro: devolve todos os chars exceto o primeiro e reprocessa
            // de forma simples: copia para numberBuffer e cria token
            strncpy(numberBuffer, buffer, sizeof(numberBuffer) - 1);
            numberBuffer[sizeof(numberBuffer) - 1] = '\0';

            token->id    = id;
            token->type  = "NUMBER";
            token->value = strdup(numberBuffer);

            fprintf(tokensIdentified, "<%s>\n", token->value);
            printf("[ SUCESSO ] Número aceito na linha %d!\n", id);
            goto end;

        } else {
            token->type = "IDENTIFIER"; // será ajustado após ler o buffer
            goto identifiers;
        }
    }

    // ── Operadores ────────────────────────────────────────────────────────
    if (c == '+' || c == '-' || c == '*' || c == '/' ||
        c == '=' || c == '>' || c == '<' || c == '!' ||
        c == '~' || c == '%') {

        if (c == '/') {
            int nx2 = fgetc(copy);
            if (nx2 == '*') {
                // Comentário de bloco inválido para esta linguagem
                char buffer[256];
                int idx = 0;
                buffer[idx++] = '/';
                buffer[idx++] = '*';
                int ch;
                while ((ch = fgetc(copy)) != EOF && idx < 254) {
                    buffer[idx++] = ch;
                    if (ch == '*') {
                        int nx3 = fgetc(copy);
                        if (nx3 == '/') { buffer[idx++] = nx3; break; }
                        ungetc(nx3, copy);
                    }
                }
                buffer[idx] = '\0';
                char lineError[512];
                snprintf(lineError, sizeof(lineError),
                    "Erro léxico na linha %d: comentário inválido '%s'\n", id, buffer);
                fputs(lineError, lexycalErrorsIdentified);
                printf("[ ERRO ] %s", lineError);
                goto end;
            }
            ungetc(nx2, copy);
        }

        token->type = "OPERATOR";
        goto operators;
    }

    // ── Delimitadores ─────────────────────────────────────────────────────
    if (c == '(' || c == ')' || c == '{' || c == '}' ||
        c == '[' || c == ']' || c == ',' || c == ':' ||
        c == '.' || c == ';' || c == '\"') {

        token->type = "DELIMITER";
        goto delimiters;
    }

    // ── Comentários ───────────────────────────────────────────────────────
    if (c == '#') {
        token->type = "COMMENTARY";
        goto comments;
    }

    // ── Caractere inválido ────────────────────────────────────────────────
    if (!isValidChar(c)) {
        char lineError[256];
        snprintf(lineError, sizeof(lineError),
            "Erro léxico na linha %d: caractere inválido %c\n", id, c);
        fputs(lineError, lexycalErrorsIdentified);
    }
    goto end;

    // ══════════════════════════════════════════════════════════════════════
    numbers:
    {
        hasDot = 0;
        hasExp = 0;
        m = 0;

        NUMBERS_Q0:
        if (c != EOF && isdigit(c) && m < (int)sizeof(numberBuffer) - 1) {
            numberBuffer[m++] = c;
            c = fgetc(copy);
            goto NUMBERS_Q1;
        } else {
            goto numbers_end;
        }

        NUMBERS_Q1:
        if (c != EOF && isdigit(c) && m < (int)sizeof(numberBuffer) - 1) {
            numberBuffer[m++] = c;
            c = fgetc(copy);
            goto NUMBERS_Q1;
        } else if (c != EOF && c == '.' && m < (int)sizeof(numberBuffer) - 1) {
            if (hasDot) {
                numberBuffer[m++] = c;
                int ch2;
                while ((ch2 = fgetc(copy)) != EOF && (isdigit(ch2) || ch2 == '.')) {
                    if (m < (int)sizeof(numberBuffer) - 1) numberBuffer[m++] = ch2;
                }
                numberBuffer[m] = '\0';
                char lineError[256];
                snprintf(lineError, sizeof(lineError),
                    "Erro léxico na linha %d: número inválido %s\n", id, numberBuffer);
                fputs(lineError, lexycalErrorsIdentified);
                printf("[ ERRO ] %s", lineError);
                goto numbers_end;
            }
            hasDot = 1;
            numberBuffer[m++] = c;
            c = fgetc(copy);
            goto NUMBERS_Q1;
        } else if (c != EOF && (c == 'e' || c == 'E') && m < (int)sizeof(numberBuffer) - 1) {
            if (hasExp) {
                numberBuffer[m] = '\0';
                char lineError[256];
                snprintf(lineError, sizeof(lineError),
                    "Erro léxico na linha %d: número inválido %s\n", id, numberBuffer);
                fputs(lineError, lexycalErrorsIdentified);
                printf("[ ERRO ] %s", lineError);
                goto numbers_end;
            }
            hasExp = 1;
            numberBuffer[m++] = c;
            c = fgetc(copy);
            goto NUMBERS_Q1;
        } else {
            if (c != EOF) ungetc(c, copy);
            numberBuffer[m] = '\0';

            token->id    = id;
            token->type  = "NUMBER";
            token->value = strdup(numberBuffer);

            fprintf(tokensIdentified, "<%s>\n", token->value);
            printf("[ SUCESSO ] Número aceito na linha %d!\n", id);
            goto numbers_end;
        }

        numbers_end:
        m = 0;
        goto end;
    }

    // ══════════════════════════════════════════════════════════════════════
    comments:
    {
        i = 0;

        COMMENTARY_Q0:
        if (c != EOF && c == '#' && i < (int)sizeof(commentariesBuffer) - 1) {
            commentariesBuffer[i++] = c;
            c = fgetc(copy);
            goto COMMENTARY_Q1;
        } else {
            goto comment_end;
        }

        COMMENTARY_Q1:
        if (c != EOF && c != '\n' && i < (int)sizeof(commentariesBuffer) - 1) {
            commentariesBuffer[i++] = c;
            c = fgetc(copy);
            goto COMMENTARY_Q1;
        } else if (c == '\n') {
            id++;
            commentariesBuffer[i++] = '\n';
            commentariesBuffer[i]   = '\0';

            token->id    = id;
            token->type  = "COMMENTARY";
            token->value = strdup(commentariesBuffer);

            char *nl = strchr(token->value, '\n');
            if (nl) *nl = '\0';

            fprintf(tokensIdentified, "<%s>\n", token->value);

            char lineComment[256];
            snprintf(lineComment, sizeof(lineComment),
                "Identificado na linha %d: %s", id, commentariesBuffer);
            fputs(lineComment, commentariesIdentified);

            printf("[ SUCESSO ] Comentário aceito na linha %d!\n", id);
            goto comment_end;
        } else {
            printf("[ ERRO ] Comentário não aceito!");
            goto comment_end;
        }

        comment_end:
        i = 0;
        goto end;
    }

    // ══════════════════════════════════════════════════════════════════════
    identifiers:
    {
        j = 0;

        IDENTIFIER_Q0:
        if (c != EOF &&
            ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') &&
            j < (int)sizeof(identifiersBuffer) - 1) {
            identifiersBuffer[j++] = c;
            c = fgetc(copy);
            goto IDENTIFIER_Q1;
        } else {
            goto identifier_end;
        }

        IDENTIFIER_Q1:
        if (c != EOF &&
            ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
             (c >= '0' && c <= '9') || c == '_') &&
            j < (int)sizeof(identifiersBuffer) - 1) {
            identifiersBuffer[j++] = c;
            c = fgetc(copy);
            goto IDENTIFIER_Q1;
        } else {
            identifiersBuffer[j] = '\0';
            if (c != EOF) ungetc(c, copy);

            // FIX: checagem de keyword APÓS preencher o buffer
            if (strcmp(identifiersBuffer, "not") == 0 ||
                strcmp(identifiersBuffer, "and") == 0 ||
                strcmp(identifiersBuffer, "or")  == 0) {
                token->type = "KEYWORD";
            } else {
                token->type = "IDENTIFIER";
            }

            token->id    = id;
            token->value = strdup(identifiersBuffer);

            fprintf(tokensIdentified, "<%d, %s>\n", token->id, token->value);

            char lineIdentifier[256];
            snprintf(lineIdentifier, sizeof(lineIdentifier),
                "Identificado na linha %d: %s", id, identifiersBuffer);
            fputs(lineIdentifier, identifiersIdentified);

            printf("[ SUCESSO ] Identificador aceito na linha %d!\n", id);
            goto identifier_end;
        }

        identifier_end:
        j = 0;
        goto end;
    }

    // ══════════════════════════════════════════════════════════════════════
    operators:
    {
        next = 0;
        k    = 0;

        OPERATORS_Q0:
        if (c != EOF && k < (int)sizeof(operatorsBuffer) - 1 &&
           (c == '+' || c == '-' || c == '/' || c == '%' || c == '~')) {

            operatorsBuffer[k++] = c;
            operatorsBuffer[k]   = '\0';

            token->id    = id;
            token->type  = "OPERATOR";
            token->value = strdup(operatorsBuffer);

            fprintf(tokensIdentified, "<%s>\n", token->value);

            char lineOperator[256];
            snprintf(lineOperator, sizeof(lineOperator),
                "Identificado na linha %d: %s", id, operatorsBuffer);
            fputs(lineOperator, operatorsIdentified);

            printf("[ SUCESSO ] Operador aceito na linha %d!\n", id);
            goto operators_end;

        } else if (c == '>' || c == '<' || c == '=' || c == '!') {
            next = fgetc(copy);
            if (next == '=') {
                operatorsBuffer[0] = c;
                goto OPERATORS_Q2;
            } else {
                operatorsBuffer[0] = c;
                operatorsBuffer[1] = '\0';
                ungetc(next, copy);

                token->id    = id;
                token->type  = "OPERATOR";
                token->value = strdup(operatorsBuffer);

                fprintf(tokensIdentified, "<%s>\n", token->value);

                char lineOperator[256];
                snprintf(lineOperator, sizeof(lineOperator),
                    "Identificado na linha %d: %s", id, operatorsBuffer);
                fputs(lineOperator, operatorsIdentified);

                printf("[ SUCESSO ] Operador aceito na linha %d!\n", id);
                goto operators_end;
            }

        } else if (c == '*') {
            next = fgetc(copy);
            if (next == '*') {
                operatorsBuffer[0] = c;
                goto OPERATORS_Q2;
            } else {
                operatorsBuffer[0] = c;
                operatorsBuffer[1] = '\0';
                ungetc(next, copy);

                token->id    = id;
                token->type  = "OPERATOR";
                token->value = strdup(operatorsBuffer);

                fprintf(tokensIdentified, "<%s>\n", token->value);

                char lineOperator[256];
                snprintf(lineOperator, sizeof(lineOperator),
                    "Identificado na linha %d: %s", id, operatorsBuffer);
                fputs(lineOperator, operatorsIdentified);

                printf("[ SUCESSO ] Operador aceito na linha %d!\n", id);
                goto operators_end;
            }
        }
        goto operators_end;

        OPERATORS_Q2:
        operatorsBuffer[1] = next;
        operatorsBuffer[2] = '\0';

        token->id    = id;
        token->type  = "OPERATOR";
        token->value = strdup(operatorsBuffer);

        fprintf(tokensIdentified, "<%s>\n", token->value);

        {
            char lineOperator[256];
            snprintf(lineOperator, sizeof(lineOperator),
                "Identificado na linha %d: %s", id, operatorsBuffer);
            fputs(lineOperator, operatorsIdentified);
        }

        printf("[ SUCESSO ] Operador aceito na linha %d!\n", id);

        operators_end:
        k = 0;
        goto end;
    }

    // ══════════════════════════════════════════════════════════════════════
    delimiters:
    {
        l = 0;

        DELIMITERS_Q0:
        if (c != EOF && l < (int)sizeof(delimitersBuffer) - 1 &&
           (c == '(' || c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == ',' || c == ':' ||
            c == '.' || c == ';' || c == '\"')) {
            goto DELIMITERS_Q1;
        } else {
            goto delimiters_end;
        }

        DELIMITERS_Q1:
        delimitersBuffer[l++] = c;
        delimitersBuffer[l]   = '\0';

        token->id    = id;
        token->type  = "DELIMITER";
        token->value = strdup(delimitersBuffer);

        fprintf(tokensIdentified, "<%s>\n", token->value);

        {
            char lineDelimiter[256];
            snprintf(lineDelimiter, sizeof(lineDelimiter),
                "Identificado na linha %d: %s", id, delimitersBuffer);
            fputs(lineDelimiter, delimitersIdentified);
        }

        printf("[ SUCESSO ] Delimitador aceito na linha %d!\n", id);

        delimiters_end:
        l = 0;
        goto end;
    }

    // ══════════════════════════════════════════════════════════════════════
    end:
    i = 0; j = 0; k = 0; l = 0;

    fclose(commentariesIdentified);
    fclose(identifiersIdentified);
    fclose(operatorsIdentified);
    fclose(delimitersIdentified);
    fclose(tokensIdentified);
    fclose(lexycalErrorsIdentified);

    // FIX: retorna NULL se o token não foi preenchido, evitando acesso a value==NULL no parser
    if (token->value == NULL) {
        free(token);
        return NULL;
    }

    return token;
}

// ══════════════════════════════════════════════════════════════════════════════
// Tabela de Símbolos
// ══════════════════════════════════════════════════════════════════════════════

int insertTokenIntoSymbolTable(Token token, SymbolTable *symbolTable) {
    // FIX: checar value e type antes de strdup
    if (token.value == NULL || token.type == NULL) return -1;

    for (int i = 0; i < 256; i++) {
        if (symbolTable->id[i] == token.id) return 0;
    }

    for (int i = 0; i < 256; i++) {
        if (symbolTable->id[i] == 0) {
            symbolTable->id[i]   = token.id;
            symbolTable->name[i] = strdup(token.value);
            symbolTable->type[i] = strdup(token.type);
            return 1;
        }
    }

    return -1;
}

int fillSymbolTable(SymbolTable *symbolTable) {
    FILE *tokensIdentified = fopen(TOKENS_DEDICATED_FILE, "r");
    if (!tokensIdentified) return 1;

    char line[256];
    while (fgets(line, sizeof(line), tokensIdentified)) {
        char *tokenValue = strtok(line, "<>\n");
        if (tokenValue) {
            Token token;
            token.id    = id++;
            token.value = strdup(tokenValue);
            token.type  = "IDENTIFIER";
            insertTokenIntoSymbolTable(token, symbolTable);
        }
    }

    fclose(tokensIdentified);
    return 0;
}

int printSymbolTable(SymbolTable *symbolTable) {
    int maxName = strlen("Nome");
    int maxType = strlen("Tipo");

    for (int i = 0; i < 256; i++) {
        if (symbolTable->id[i] != 0) {
            int nameLen = strlen(symbolTable->name[i]);
            int typeLen = strlen(symbolTable->type[i]);
            if (nameLen > maxName) maxName = nameLen;
            if (typeLen > maxType) maxType = typeLen;
        }
    }

    maxName += 2;
    maxType += 2;

    printf("Tabela de Símbolos:\n");
    printf("%-10s %-*s %-*s\n", "ID", maxName, "Nome", maxType, "Tipo");

    int totalWidth = 10 + maxName + maxType + 2;
    for (int i = 0; i < totalWidth; i++) printf("=");
    printf("\n");

    for (int i = 0; i < 256; i++) {
        if (symbolTable->id[i] != 0) {
            printf("%-10d %-*s %-*s\n",
                symbolTable->id[i],
                maxName, symbolTable->name[i],
                maxType, symbolTable->type[i]);
        }
    }

    return 0;
}

// ══════════════════════════════════════════════════════════════════════════════
// AST
// ══════════════════════════════════════════════════════════════════════════════

ASTNode *createASTNode(Token token, ASTNode *left, ASTNode *right) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->token = token;
    node->left  = left;
    node->right = right;
    return node;
}

int isRelOp(Token token) {
    if (token.value == NULL) return 0;
    return strcmp(token.value, "<")  == 0 ||
           strcmp(token.value, ">")  == 0 ||
           strcmp(token.value, "==") == 0 ||
           strcmp(token.value, "!=") == 0 ||
           strcmp(token.value, "<=") == 0 ||
           strcmp(token.value, ">=") == 0 ||
           strcmp(token.value, "<>") == 0 ||
           strcmp(token.value, "in") == 0 ||
           strcmp(token.value, "is") == 0;
}

// ══════════════════════════════════════════════════════════════════════════════
// Parser (análise sintática)
// ══════════════════════════════════════════════════════════════════════════════

void sync(FILE *file) {
    while (CurrentToken != NULL &&
           CurrentToken->value != NULL &&
           strcmp(CurrentToken->value, ";")  != 0 &&
           strcmp(CurrentToken->value, "\n") != 0) {
        CurrentToken = getNextToken(file);
    }
    if (CurrentToken != NULL)
        CurrentToken = getNextToken(file);
}

ASTNode *parseExpression(FILE *file); // forward declaration

ASTNode *parseFactor(FILE *file) {
    // FIX: checar value antes de qualquer strcmp
    if (CurrentToken == NULL || CurrentToken->value == NULL)
        return NULL;

    Token token = *CurrentToken;

    if (strcmp(token.type, "NUMBER")     == 0 ||
        strcmp(token.type, "IDENTIFIER") == 0) {
        CurrentToken = getNextToken(file);
        return createASTNode(token, NULL, NULL);
    }

    if (strcmp(token.value, "(") == 0) {
        CurrentToken = getNextToken(file);

        if (CurrentToken == NULL || CurrentToken->value == NULL) {
            fprintf(sintaticErrorsIdentified,
                "Erro sintático na linha %d: expressão esperada após '('\n",
                token.id);
            sync(file);
            return NULL;
        }

        ASTNode *node = parseExpression(file);

        if (CurrentToken == NULL || CurrentToken->value == NULL ||
            strcmp(CurrentToken->value, ")") != 0) {
            fprintf(sintaticErrorsIdentified,
                "Erro sintático na linha %d: esperado ')'\n",
                token.id);
            sync(file);
            return NULL;
        }

        CurrentToken = getNextToken(file);
        return node;
    }

    if (strcmp(token.value, "[") == 0) {
        CurrentToken = getNextToken(file);

        if (CurrentToken != NULL && CurrentToken->value != NULL &&
            strcmp(CurrentToken->value, "]") == 0) {
            CurrentToken = getNextToken(file);
            return createASTNode(token, NULL, NULL);
        }

        ASTNode *first    = parseExpression(file);
        ASTNode *listNode = first;

        while (CurrentToken != NULL && CurrentToken->value != NULL &&
               strcmp(CurrentToken->value, ",") == 0) {
            CurrentToken = getNextToken(file);
            ASTNode *nx = parseExpression(file);
            listNode = createASTNode(token, listNode, nx);
        }

        if (CurrentToken == NULL || CurrentToken->value == NULL ||
            strcmp(CurrentToken->value, "]") != 0) {
            fprintf(sintaticErrorsIdentified,
                "Erro sintático na linha %d: esperado ']'\n",
                token.id);
            sync(file);
            return NULL;
        }

        CurrentToken = getNextToken(file);
        return listNode;
    }

    char lineError[256];
    snprintf(lineError, sizeof(lineError),
        "Erro sintático na linha %d: token inesperado '%s'\n",
        token.id, token.value);
    fputs(lineError, sintaticErrorsIdentified);

    CurrentToken = getNextToken(file);
    sync(file);
    return NULL;
}

ASTNode *parseUnary(FILE *file) {
    // FIX: checar value antes de strcmp
    if (CurrentToken == NULL || CurrentToken->value == NULL)
        return NULL;

    Token token = *CurrentToken;

    if (strcmp(token.value, "+") == 0 ||
        strcmp(token.value, "-") == 0 ||
        strcmp(token.value, "~") == 0) {
        CurrentToken = getNextToken(file);
        ASTNode *child = parseUnary(file);
        return createASTNode(token, NULL, child);
    }

    return parseFactor(file);
}

ASTNode *parsePower(FILE *file) {
    ASTNode *left = parseUnary(file);
    if (left == NULL) return NULL;

    if (CurrentToken != NULL && CurrentToken->value != NULL &&
        strcmp(CurrentToken->value, "**") == 0) {
        Token op = *CurrentToken;
        CurrentToken = getNextToken(file);
        ASTNode *right = parsePower(file);
        return createASTNode(op, left, right);
    }

    return left;
}

ASTNode *parseTerm(FILE *file) {
    ASTNode *node = parsePower(file);

    while (CurrentToken != NULL && CurrentToken->value != NULL &&
          (strcmp(CurrentToken->value, "*") == 0 ||
           strcmp(CurrentToken->value, "/") == 0 ||
           strcmp(CurrentToken->value, "%") == 0)) {
        Token op = *CurrentToken;
        CurrentToken = getNextToken(file);
        ASTNode *right = parsePower(file);
        node = createASTNode(op, node, right);
    }

    return node;
}

ASTNode *parseArithExpr(FILE *file) {
    ASTNode *node = parseTerm(file);

    while (CurrentToken != NULL && CurrentToken->value != NULL &&
          (strcmp(CurrentToken->value, "+") == 0 ||
           strcmp(CurrentToken->value, "-") == 0)) {
        Token op = *CurrentToken;
        CurrentToken = getNextToken(file);
        ASTNode *right = parseTerm(file);
        node = createASTNode(op, node, right);
    }

    return node;
}

ASTNode *parseRelExpr(FILE *file) {
    ASTNode *node = parseArithExpr(file);

    while (CurrentToken != NULL && CurrentToken->value != NULL &&
           isRelOp(*CurrentToken)) {
        Token op = *CurrentToken;
        CurrentToken = getNextToken(file);
        ASTNode *right = parseArithExpr(file);
        node = createASTNode(op, node, right);
    }

    return node;
}

ASTNode *parseNotExpr(FILE *file) {
    if (CurrentToken != NULL && CurrentToken->value != NULL &&
        strcmp(CurrentToken->value, "not") == 0) {
        Token op = *CurrentToken;
        CurrentToken = getNextToken(file);
        ASTNode *child = parseNotExpr(file);
        return createASTNode(op, NULL, child);
    }

    return parseRelExpr(file);
}

ASTNode *parseAndExpr(FILE *file) {
    ASTNode *node = parseNotExpr(file);

    while (CurrentToken != NULL && CurrentToken->value != NULL &&
           strcmp(CurrentToken->value, "and") == 0) {
        Token op = *CurrentToken;
        CurrentToken = getNextToken(file);
        ASTNode *right = parseNotExpr(file);
        node = createASTNode(op, node, right);
    }

    return node;
}

ASTNode *parseOrExpr(FILE *file) {
    ASTNode *node = parseAndExpr(file);

    while (CurrentToken != NULL && CurrentToken->value != NULL &&
           strcmp(CurrentToken->value, "or") == 0) {
        Token op = *CurrentToken;
        CurrentToken = getNextToken(file);
        ASTNode *right = parseAndExpr(file);
        node = createASTNode(op, node, right);
    }

    return node;
}

ASTNode *parseExpression(FILE *file) {
    return parseOrExpr(file);
}

ASTNode *parseStatement(FILE *file) {
    if (CurrentToken == NULL || CurrentToken->value == NULL) return NULL;

    if (strcmp(CurrentToken->type, "IDENTIFIER") == 0) {
        Token ident = *CurrentToken;
        CurrentToken = getNextToken(file);

        if (CurrentToken != NULL && CurrentToken->value != NULL &&
            strcmp(CurrentToken->value, "=") == 0) {
            Token assign = *CurrentToken;
            CurrentToken = getNextToken(file);
            ASTNode *expr = parseExpression(file);
            return createASTNode(assign,
                createASTNode(ident, NULL, NULL),
                expr);
        }

        return createASTNode(ident, NULL, NULL);
    }

    return parseExpression(file);
}

ASTNode *parseProgram(FILE *file) {
    ASTNode *root = NULL;

    while (CurrentToken != NULL && CurrentToken->value != NULL) {
        ASTNode *stmt = parseStatement(file);
        if (stmt == NULL) {
            sync(file);
        }
    }

    return root;
}