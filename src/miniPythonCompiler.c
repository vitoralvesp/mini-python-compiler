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

Token *CurrentToken;

int id = 1;

int i = 0;

int j = 0;

int k = 0;

int l = 0;

int m = 0;

IdentifiedErrors *identifiedErrors;

int findClosureCharacter(char *string, char characterToLookFor, int length);

/** @brief Recebe o arquivo de entrada e procede para validá-lo, retornando 0 se for válido e 1 caso contrário.
 *  @param argc número de parâmetros recebidos na linha de comando.
 *  @param argv vetor com os parâmetros recebidos na linha de comando
 *  @return 0 se o arquivo for válido, 1 caso contrário.
 */
int readFile(int argc, char *argv[]);

/** @brief Cria uma cópia do arquivo de entrada para que as modificações sejam feitas na cópia, retornando 0 se o processo for válido e 1 caso contrário.
 *  @param argc número de parâmetros recebidos na linha de comando.
 *  @param argv vetor com os parâmetros recebidos na linha de comando
 *  @return 0 se o arquivo for válido, 1 caso contrário.
 */
FILE *createFileCopy(int argc, char *argv[]);

/** @brief Recebe um caractere a ser removido de um arquivo e procede com a busca para removêlo, retornando 0 se o processo for bem-sucedido e 1 caso contrário.
 *  @param argc número de parâmetros recebidos na linha de comando.
 *  @param argv vetor com os parâmetros recebidos na linha de comando
 *  @param characterToRemove caractere a ser removido do arquivo.
 *  @return 0 se o arquivo for válido, 1 caso contrário.
 */
int removeCharacters(char *fileCopyPath, char characterToRemove);

int slice(char *newString, char *string, char caracterToLookFor, int length);

Token *getNextToken(FILE *copy);

int insertTokenIntoSymbolTable(Token token, SymbolTable *symbolTable);

int fillSymbolTable();

int clearFile(const char *filePath);

int isValidChar(char c);

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

    CurrentToken = (Token *)malloc(sizeof(Token));

    copy = fopen(fileCopyPath, "rb");

    clearFile(FILEPATH_COMMENTARIES_DEDICATED_FILE);

    clearFile(FILEPATH_IDENTIFIERS_DEDICATED_FILE);

    clearFile(FILEPATH_OPERATORS_DEDICATED_FILE);

    clearFile(FILEPATH_DELIMITERS_DEDICATED_FILE);

    clearFile(TOKENS_DEDICATED_FILE);

    clearFile(LEXYCAL_ERRORS_DEDICATED_FILE);

    while (1) {
        
        Token *t = getNextToken(copy);
        
        if (t == NULL) break;
    
    }

    fclose(copy);

    int didCommentsRemovalProcessSucceed = removeCharacters(fileCopyPath, '#');
    
    int didTabsRemovalProcessSucceed = removeCharacters(fileCopyPath, '\t');
    
    int didLineBreaksRemovalProcessSucceed = removeCharacters(fileCopyPath, '\n');

    int didWhiteSpacesRemovalProcessSucceed = removeCharacters(fileCopyPath, ' ');

    if (didCommentsRemovalProcessSucceed != 0) {

        puts("[ ERRO ] Não foi possível remover os comentários do arquivo copiado.");

    } else {

        puts("[ SUCESSO ] Todos os comentários foram removidos do arquivo copiado com sucesso!");

    }
    
    if (didWhiteSpacesRemovalProcessSucceed != 0) {

        puts("[ ERRO ] Não foi possível remover os espaços do arquivo copiado.");

    } else {

        puts("[ SUCESSO ] Todos os espaços foram removidos do arquivo copiado com sucesso!");

    }

    if (didTabsRemovalProcessSucceed != 0) {

        puts("[ ERRO ] Não foi possível remover as tabulações do arquivo copiado.");

    } else {

        puts("[ SUCESSO ] Todas as tabulações foram removidas do arquivo copiado com sucesso!");

    }

    if (didLineBreaksRemovalProcessSucceed != 0) {

        puts("[ ERRO ] Não foi possível remover os caracteres de quebra de linha do arquivo copiado.");

    } else {

        puts("[ SUCESSO ] Todos os caracteres de quebra de linha foram removidos do arquivo copiado com sucesso!");

    }

    if (didCommentsRemovalProcessSucceed != 0 && didWhiteSpacesRemovalProcessSucceed != 0 && didTabsRemovalProcessSucceed != 0 && didLineBreaksRemovalProcessSucceed != 0) {

        puts("[ ERRO ] O processo de remoção de caracteres do arquivo copiado não foi completamente bem-sucedido. Verifique os erros acima para mais detalhes.");

        return 1;

    } else {

        puts("[ SUCESSO ] Prosseguindo para a análise léxica!");
    
    }

    return 0;
}


int readFile(int argc, char *argv[]) {

    FILE *file = fopen(argv[1], "r");

    int result = 0;

    if (!file) 
        result = 1;
    
    else 
        result = 0;

    fclose(file);
    
    return result;
}

int isValidChar(char c) {
    return iswalpha(c) || 
           iswdigit(c) ||
           c == '_' ||
           c == '#' || 
           c == '+' || 
           c == '-' || 
           c == '*' || 
           c == '/' || 
           c == '~' || 
           c == '%' || 
           c == '=' || 
           c == '<' || 
           c == '>' || 
           c == '!' ||
           c == '(' || 
           c == ')' || 
           c == '{' || 
           c == '}' ||
           c == '[' || 
           c == ']' ||
           c == ',' || 
           c == ';' || 
           c == ':' || 
           c == '.' ||
           c == '\"'||
           c == '\''||
           c == ' ' ||
           c == '\t'||
           c == '\n'||
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

    const char *base = strrchr(argv[1], '/');
    
    base = base ? base + 1 : argv[1];

    char outputFilePath[256];
    
    snprintf(outputFilePath, sizeof(outputFilePath), "%s%s", FILE_OUTPUT_PATH, argv[1] + strlen("input/"));

    FILE *file = fopen(argv[1], "rb");

    FILE *copy = fopen(outputFilePath, "wb");

    if (!file || !copy) {
        if (file) fclose(file);
        if (copy) fclose(copy);

        return NULL;
    }
    
    else {
        
        int c;

        while ((c = fgetc(file)) != EOF) {

                fputc(c, copy);
        
        }

    }

    fclose(file);

    fclose(copy);
    
    return copy;
}

int findClosureCharacter(char *string, char characterToLookFor, int length) {

    for (int i = 0; i < length; i++) {

        if (string[i] == characterToLookFor) {
            return 0;
        }

    }

    return 1;

}

int slice(char *newString, char *string, char characterToLookFor, int length) {

    int k = -1;

    for (int i = 0; i < length; i++) {

        if (string[i] == characterToLookFor) {
            k = i;
            break;
        }

    }

    if (k == -1) return 0;

    k = k + 1;

    int j = 0;

    for (j = 0; j < length; j++) {

        newString[j] = string[k + j];

    }

    newString[j] = '\0';

    return 0;

}

int removeCharacters(char *fileCopyPath, char characterToRemove) {

    FILE *copy = fopen(fileCopyPath, "rb");

    if (!copy) {
        
        return 1;

    } else {
        
        FILE *tempFile = fopen("./temp/removeCharactersTempFile.txt", "wb");

        if (!tempFile) {

            fclose(copy);
            
            return 1;
        
        } else {

            int c;

            while ((c = fgetc(copy)) != EOF) {

                if (c != characterToRemove) {

                    fputc(c, tempFile);

                }
        
            }

            fclose(copy);

            fclose(tempFile);

            if (remove(fileCopyPath) != 0) {
            
                fprintf(stderr, "Error removing original file\n");
            
            }
            
            if (rename("./temp/removeCharactersTempFile.txt", fileCopyPath) != 0) {
            
                fprintf(stderr, "Error renaming temporary file\n");
            
            }
 
        }

    }

    return 0;

}

Token *getNextToken(FILE *copy) {

    FILE *commentariesIdentified = fopen(FILEPATH_COMMENTARIES_DEDICATED_FILE, "a");

    FILE *identifiersIdentified = fopen(FILEPATH_IDENTIFIERS_DEDICATED_FILE, "a");

    FILE *operatorsIdentified = fopen(FILEPATH_OPERATORS_DEDICATED_FILE, "a");

    FILE *delimitersIdentified = fopen(FILEPATH_DELIMITERS_DEDICATED_FILE, "a");

    FILE *tokensIdentified = fopen(TOKENS_DEDICATED_FILE, "a");

    FILE *lexycalErrorsIdentified = fopen(LEXYCAL_ERRORS_DEDICATED_FILE, "a");

    if (!copy || !commentariesIdentified || !identifiersIdentified || !operatorsIdentified || !delimitersIdentified || !tokensIdentified || !lexycalErrorsIdentified) return NULL;

    Token *token = (Token *)malloc(sizeof(Token));

    int operator_c;

    char commentariesBuffer[256];

    char identifiersBuffer[256];

    char operatorsBuffer[256];

    char delimitersBuffer[256];

    char numberBuffer[256];

    int c = fgetc(copy);

    if (c == EOF) return NULL;

    while (c == ' ' || c == '\n' || c == '\t') {

        if (c == '\n') id++;
        
        c = fgetc(copy);
        
        if (c == EOF) return NULL;
    
    } 

    if (c != EOF) {

        if (c == '\n') id++;

        if (isdigit(c)) {

            goto numbers;

        }
        
        // Identificadores
        if (isdigit(c) || isalpha(c) || c == '_') {

            char buffer[256];
            int idx = 0;

            buffer[idx++] = c;

            int next = fgetc(copy);

            int isInvalid = 0;

            while (next != EOF) {
                if (isalnum(next) || next == '_') {
                    buffer[idx++] = next;
                } else if (next == '#') {
                    buffer[idx++] = next;
                    isInvalid = 1;
                } else {
                    break;
                }
                next = fgetc(copy);
            }

            buffer[idx] = '\0';

            if (next != EOF) ungetc(next, copy);

            int hasLetter = 0;

            if (isInvalid) {
                
                char lineError[256];

                snprintf(lineError, sizeof(lineError), "Erro léxico na linha %d: identificador inválido '%s'\n", id, buffer);

                fputs(lineError, lexycalErrorsIdentified);
                printf("[ ERRO ] %s", lineError);
            }

            //return token;

            goto identifiers;

        }
        
        // Operadores
        if (c == '+' || 
            c == '-' ||
            c == '*' || 
            c == '/' || 
            c == '=' || 
            c == '>' || 
            c == '<' || 
            c == '!' || 
            c == '~' || 
            c == '%') {

                if (c == '/') {
                    int next = fgetc(copy);

                    if (next == '*') {

                        char buffer[256];
                        int idx = 0;

                        buffer[idx++] = '/';
                        buffer[idx++] = '*';

                        while ((c = fgetc(copy)) != EOF) {
                            buffer[idx++] = c;

                            if (c == '*') {
                                int next2 = fgetc(copy);
                                if (next2 == '/') {
                                    buffer[idx++] = next2;
                                    break;
                                }
                                ungetc(next2, copy);
                            }
                        }

                        buffer[idx] = '\0';

                        char lineError[512];

                        snprintf(lineError, sizeof(lineError),
                            "Erro léxico na linha %d: comentário inválido '%s'\n",
                            id, buffer);

                        fputs(lineError, lexycalErrorsIdentified);
                        printf("[ ERRO ] %s", lineError);

                        goto operators_end;
                    }

                    ungetc(next, copy);

                }
            
                token->type = "OPERATOR";

                goto operators;
            
            }

            // Delimitadores
            if (c == '(' ||
                c == ')' ||
                c == '{' ||
                c == '}' ||
                c == '[' ||
                c == ']' ||
                c == ',' ||
                c == ':' ||
                c == '.' ||
                c == ';' ||
                c == '\"') {

                    int isClosureValid = 0;

                    if (c == '(') {

                        isClosureValid = findClosureCharacter(delimitersBuffer, ')', sizeof(delimitersBuffer));

                        if (!isClosureValid) {

                            char lineError[256];

                            snprintf(lineError, sizeof(lineError), "Erro léxico na linha %d: delimitador de fechamento ] esperado para o delimitador de abertura [\n", id);

                            fputs(lineError, lexycalErrorsIdentified);

                            //printf("[ ERRO ] %s", lineError);   

                        }

                    }

                    if (c == '{') {

                        isClosureValid = findClosureCharacter(delimitersBuffer, '}', sizeof(delimitersBuffer));

                        if (!isClosureValid) {

                            char lineError[256];

                            snprintf(lineError, sizeof(lineError), "Erro léxico na linha %d: delimitador de fechamento } esperado para o delimitador de abertura {\n", id);

                            fputs(lineError, lexycalErrorsIdentified);

                            //printf("[ ERRO ] %s", lineError);   

                        }

                    }

                    if (c == '\'') {

                        isClosureValid = findClosureCharacter(delimitersBuffer, '\'', sizeof(delimitersBuffer));

                        if (!isClosureValid) {

                            char lineError[256];

                            snprintf(lineError, sizeof(lineError), "Erro léxico na linha %d: delimitador de fechamento \' esperado para o delimitador de abertura \'\n", id);

                            fputs(lineError, lexycalErrorsIdentified);

                            //printf("[ ERRO ] %s", lineError);   

                        }

                    }

                    if (c == '\"') {

                        isClosureValid = findClosureCharacter(delimitersBuffer, '\"', sizeof(delimitersBuffer));

                        if (!isClosureValid) {

                            char lineError[256];

                            snprintf(lineError, sizeof(lineError), "Erro léxico na linha %d: delimitador de fechamento \" esperado para o delimitador de abertura \"\n", id);

                            fputs(lineError, lexycalErrorsIdentified);

                            //printf("[ ERRO ] %s", lineError);   

                        }

                    }
                    
                    token->type = "DELIMITER";

                    goto delimiters;
                
                }

            }

            // Comentários
            if (c == '#') {
                
                token->type = "COMMENTARY";
                
                goto comments;
                
            }

            if (!isValidChar(c)) {

                    char lineError[256];

                    snprintf(lineError, sizeof(lineError), "Erro léxico na linha %d: caractere inválido %c\n", id, c);

                    fputs(lineError, lexycalErrorsIdentified);

                    //printf("[ ERRO ] %s", lineError);   
                
            }

            numbers_end:
            numberBuffer[0] = '\0';
            m = 0;

            comment_end:
            commentariesBuffer[0] = '\0';
            i = 0;
                
            identifier_end:
            identifiersBuffer[0] = '\0';
            j = 0;
                
            operators_end:
            operatorsBuffer[0] = '\0';
            k = 0;
                
            delimiters_end:
            delimitersBuffer[0] = '\0';
            l = 0;
            
    
    goto end;

    numbers:

        int hasDot = 0;
        
        int hasExp = 0;
    
        goto NUMBERS_Q0;

        NUMBERS_Q0:

            if (c != EOF && isdigit(c) && m < sizeof(numberBuffer) - 1) {

                numberBuffer[m] = c;
                
                c = fgetc(copy);

                m++;

                goto NUMBERS_Q1;

            } else {

                goto numbers_end;

            }
            
        NUMBERS_Q1:

            if (c != EOF && isdigit(c) && m < sizeof(numberBuffer) - 1) {

                numberBuffer[m] = c;

                c = fgetc(copy);
                
                m++;
                
                goto NUMBERS_Q1;
                
            } else if (c != EOF && c == '.' && m < sizeof(numberBuffer) - 1) {
    
                if (hasDot) {

                    numberBuffer[m++] = c;

                    while ((c = fgetc(copy)) != EOF && (isdigit(c) || c == '.')) {
                        numberBuffer[m++] = c;
                    }

                    numberBuffer[m] = '\0';

                    char lineError[256];

                    snprintf(lineError, sizeof(lineError),
                        "Erro léxico na linha %d: número inválido '%s'\n", id, numberBuffer);

                    fputs(lineError, lexycalErrorsIdentified);
                    printf("[ ERRO ] %s", lineError);

                    goto numbers_end;
                }

                hasDot = 1;

                numberBuffer[m++] = c;
                
                c = fgetc(copy);
                
                goto NUMBERS_Q1;
            
            
            } else if (c != EOF && (c == 'e' || c == 'E') && m < sizeof(numberBuffer) - 1) {
    
                if (hasExp) {

                    numberBuffer[m] = '\0';
                    
                    char lineError[256];
                    
                    snprintf(lineError, sizeof(lineError),
                        "Erro léxico na linha %d: número inválido '%s'\n", id, numberBuffer);

                    
                    fputs(lineError, lexycalErrorsIdentified);
                    
                    printf("[ ERRO ] %s", lineError);

                    goto numbers_end;
                }

                hasExp = 1;
            
                numberBuffer[m++] = c;
            
                c = fgetc(copy);
            
                goto NUMBERS_Q1;
            
            } else {

                numberBuffer[m++] = '\n';
                
                numberBuffer[m++] = '\0';
                
                token->id = id;

                token->value = strdup(numberBuffer);

                char *newline = strchr(token->value, '\n');
                
                if (newline) *newline = '\0';

                fprintf(tokensIdentified, "<%s>\n", token->value);

                char lineNumber[256];

                snprintf(lineNumber, sizeof(lineNumber), "Identificado na linha %d: %s\n", id, numberBuffer);
                
                fputs(lineNumber, tokensIdentified);

                printf("[ SUCESSO ] Número aceito na linha %d!\n", id);

                goto numbers_end;
                
            }

    comments:

        goto COMMENTARY_Q0;

        COMMENTARY_Q0:

            if (c != EOF && c == '#' && i < sizeof(commentariesBuffer) - 1) {

                commentariesBuffer[i] = c;
                
                c = fgetc(copy);

                i++;

                goto COMMENTARY_Q1;

            } else {

                goto comment_end;

            }
            
        COMMENTARY_Q1:

            if (c != EOF && c != '\n' && i < sizeof(commentariesBuffer) - 1) {

                commentariesBuffer[i] = c;

                c = fgetc(copy);
                
                i++;
                
                goto COMMENTARY_Q1;
                
            } else if (c == '\n') {
                
                id++;
                
                commentariesBuffer[i++] = '\n';
                
                commentariesBuffer[i++] = '\0';
                
                token->id = id;

                token->value = strdup(commentariesBuffer);

                char *newline = strchr(token->value, '\n');
                
                if (newline) *newline = '\0';

                fprintf(tokensIdentified, "<%s>\n", token->value);

                char lineComment[256];

                snprintf(lineComment, sizeof(lineComment), "Identificado na linha %d: %s", id, commentariesBuffer);
                
                fputs(lineComment, commentariesIdentified);

                printf("[ SUCESSO ] Comentário aceito na linha %d!\n", id);

                goto comment_end;
                
            } else {
                
                printf("[ ERRO ] Comentário não aceito!");
                
                goto comment_end;
                
            }

    identifiers:

            goto IDENTIFIER_Q0;

            IDENTIFIER_Q0:

               if (c != EOF && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') && j < sizeof(identifiersBuffer) - 1) {

                    identifiersBuffer[j] = c;
                    
                    c = fgetc(copy);

                    j++;

                    goto IDENTIFIER_Q1;

                } else {

                    goto identifier_end;

                }
            
            IDENTIFIER_Q1:

                if (c != EOF && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') && j < sizeof(identifiersBuffer) - 1) {

                    identifiersBuffer[j] = c;

                    c = fgetc(copy);
                    
                    j++;
                    
                    goto IDENTIFIER_Q1;
                    
                } else {
                    
                    identifiersBuffer[j] = '\0';

                    if (c != EOF) ungetc(c, copy);

                    token->id = id;

                    token->value = strdup(identifiersBuffer);

                    fprintf(tokensIdentified, "<%d, %s>\n", token->id, token->value);

                    char lineIdentifier[256];

                    snprintf(lineIdentifier, sizeof(lineIdentifier), "Identificado na linha %d: %s\n", id, identifiersBuffer);
                    
                    fputs(lineIdentifier, identifiersIdentified);

                    printf("[ SUCESSO ] Identificador aceito na linha %d!\n", id);

                    goto identifier_end;
                    
                }

    operators:

        int next = 0;

        goto OPERATORS_Q0;

        OPERATORS_Q0:
                
            if (c != EOF && k < sizeof(operatorsBuffer) - 1 &&
               (c == '+' ||
                c == '-' ||
                c == '/' ||
                c == '%' ||
                c == '~')) {

                    operatorsBuffer[k] = c;

                    k++;

                    operatorsBuffer[k] = '\0';

                    token->id = id;

                    token->value = strdup(operatorsBuffer);

                    fprintf(tokensIdentified, "<%s>\n", token->value);

                    char lineOperator[256];

                    snprintf(lineOperator, sizeof(lineOperator), "Identificado na linha %d: %s\n", id, operatorsBuffer);

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

                    operatorsBuffer[k] = c;

                    token->id = id;

                    token->value = strdup(operatorsBuffer);

                    fprintf(tokensIdentified, "<%s>\n", token->value);

                    k++;

                    char lineOperator[256];

                    snprintf(lineOperator, sizeof(lineOperator), "Identificado na linha %d: %s\n", id, operatorsBuffer);

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

                    operatorsBuffer[k] = c;

                    token->id = id;

                    token->value = strdup(operatorsBuffer);

                    fprintf(tokensIdentified, "<%s>\n", token->value);

                    k++;

                    operatorsBuffer[k] = '\0';

                    char lineOperator[256];

                    snprintf(lineOperator, sizeof(lineOperator), "Identificado na linha %d: %s\n", id, operatorsBuffer);

                    fputs(lineOperator, operatorsIdentified);

                    printf("[ SUCESSO ] Operador aceito na linha %d!\n", id);

                    goto operators_end;

                }

            }

    OPERATORS_Q2:

        operatorsBuffer[1] = next;

        operatorsBuffer[2] = '\0';

        ungetc(next, copy);

        operatorsBuffer[k] = c;

        token->id = id;

        token->value = strdup(operatorsBuffer);

        fprintf(tokensIdentified, "<%s>\n", token->value);

        k++;

        char lineOperator[256];

        snprintf(lineOperator, sizeof(lineOperator), "Identificado na linha %d: %s\n", id, operatorsBuffer);

        fputs(lineOperator, operatorsIdentified);

        printf("[ SUCESSO ] Operador aceito na linha %d!\n", id);

        goto operators_end;
        
    
    delimiters:

        goto DELIMITERS_Q0;

        DELIMITERS_Q0:

            if (c != EOF && l < sizeof(delimitersBuffer) - 1 &&
               (c == '(' ||
                c == ')' ||
                c == '{' ||
                c == '}' ||
                c == '[' ||
                c == ']' ||
                c == ',' ||
                c == ':' ||
                c == '.' ||
                c == ';' ||
                c == '\"')) {

                    goto DELIMITERS_Q1;

            } else {

                goto delimiters_end;

            }
        
        DELIMITERS_Q1:

            delimitersBuffer[l] = c;

            l++;

            delimitersBuffer[l] = '\0';

            token->id = id;

            token->value = strdup(delimitersBuffer);

            fprintf(tokensIdentified, "<%s>\n", token->value);

            char lineDelimiter[256];

            snprintf(lineDelimiter, sizeof(lineDelimiter), "Identificado na linha %d: %s\n", id, delimitersBuffer);

            fputs(lineDelimiter, delimitersIdentified);

            printf("[ SUCESSO ] Delimitador aceito na linha %d!\n", id);

            goto delimiters_end;

    end:
    i = 0;
    j = 0;
    k = 0;
    l = 0;

    fclose(commentariesIdentified);
    fclose(identifiersIdentified);
    fclose(operatorsIdentified);
    fclose(delimitersIdentified);
    fclose(tokensIdentified);
    fclose(lexycalErrorsIdentified);

    return token;
    
}

int fillSymbolTable() {

    //getNextToken();

    return 0;

}