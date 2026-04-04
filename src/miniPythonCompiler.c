#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ******************************************************
// ETAPA 0: RECEBIMENTO DO ARQUIVO 
// ******************************************************

#define FILE_OUTPUT_PATH "./output/"

#define FILE_TEMP_PATH "../temp/"

#define FILEPATH_COMMENTARIES_DEDICATED_FILE "./output/commentariesIdentified.txt"

#define FILEPATH_IDENTIFIERS_DEDICATED_FILE "./output/identifiersIdentified.txt"

#define FILEPATH_OPERATORS_DEDICATED_FILE "./output/operatorsIdentified.txt"


typedef struct Token {
    char *type;
    char *value;
} Token;

typedef struct SymbolTable {
    int id[256];
    char *name[256];
    char *type[256];

} SymbolTable;

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

int getNextToken();

int insertTokenIntoSymbolTable(Token token, SymbolTable *symbolTable);

int fillSymbolTable();

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

    int result = slice(fileName, argv[1], '/', strlen(argv[1]));

    snprintf(fileCopyPath, sizeof(fileCopyPath), "%s%s", FILE_OUTPUT_PATH, fileName);

    if (result == -1) {
    
        puts("[ ERRO ] Não foi possível extrair o nome do arquivo de entrada para o processo de remoção de caracteres.");
    
        return -1;
    
    }

    printf("Caminho do arquivo copiado: %s\n", fileCopyPath);

    int didFillSymbolTableSucceed = fillSymbolTable();

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


FILE *createFileCopy(int argc, char *argv[]) {

    const char *base = strrchr(argv[1], '/');
    
    base = base ? base + 1 : argv[1];

    char outputFilePath[256];
    
    snprintf(outputFilePath, sizeof(outputFilePath), "%s%s", FILE_OUTPUT_PATH, base);

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

int getNextToken() {

    FILE *copy = fopen("./output/example-1.txt", "rb");

    FILE *commentariesIdentified = fopen(FILEPATH_COMMENTARIES_DEDICATED_FILE, "w");

    FILE *identifiersIdentified = fopen(FILEPATH_IDENTIFIERS_DEDICATED_FILE, "w");

    FILE *operatorsIdentified = fopen(FILEPATH_OPERATORS_DEDICATED_FILE, "w");

    if (!copy || !commentariesIdentified || !identifiersIdentified || !operatorsIdentified) return 1;

    int id = 0;

    int i = 0;

    int j = 0;

    int k = 0;

    int c;

    int operator_c;

    char commentariesBuffer[256];

    char identifiersBuffer[256];

    char operatorsBuffer[256];

    char lookahead[3] = {'\0'};

    while ((c = fgetc(copy)) != EOF) {

        if (c == '\n') id++;

        // Comentários
        if (c == '#') goto comments;

        // Identificadores
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') goto identifiers;

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
            c == '%') goto operators;

        comment_end:
            commentariesBuffer[0] = '\0';
            i = 0;
        
        identifier_end:
            identifiersBuffer[0] = '\0';
            j = 0;

        operators_end:
            operatorsBuffer[0] = '\0';
            k = 0;

    }

    goto end;

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
                c == '+' ||
                c == '-' ||
                c == '/' ||
                c == '%' ||
                c == '~') {

                    operatorsBuffer[k] = c;

                    k++;

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

                    k++;

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

        k++;

        char lineOperator[256];

        snprintf(lineOperator, sizeof(lineOperator), "Identificado na linha %d: %s\n", id, operatorsBuffer);

        fputs(lineOperator, operatorsIdentified);

        printf("[ SUCESSO ] Operador aceito na linha %d!\n", id);

        goto operators_end;
        
    
    end:
    i = 0;
    j = 0;

    fclose(copy);
    fclose(commentariesIdentified);

    return 0;
    
}

int fillSymbolTable() {

    getNextToken();

    return 0;

}