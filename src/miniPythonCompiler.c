#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ******************************************************
// ETAPA 0: RECEBIMENTO DO ARQUIVO 
// ******************************************************

#define FILE_OUTPUT_PATH "./output/"

#define FILE_TEMP_PATH "../temp/"

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

    //getNextToken();

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

            if(remove(fileCopyPath) != 0) {
                fprintf(stderr, "Error removing original file\n");
            }
            if(rename("./temp/removeCharactersTempFile.txt", fileCopyPath) != 0) {
                fprintf(stderr, "Error renaming temporary file\n");
            }

        }

    }


    return 0;

}

int getNextToken() {

    FILE *copy = fopen("./output/example-1.txt", "rb");

    FILE *commentariesIdentified = fopen("./output/commentariesIdentified.txt", "wb");

    if (!copy) return 1;

    int i = 0;

    // Comments
    goto comments;

    comments:

        char commentariesBuffer[256];

        int c = fgetc(copy);

        goto Q0;

        Q0:

            if (c != EOF && c == '#') {

                commentariesBuffer[i] = c;

                c = fgetc(copy);

                goto Q1;

            } else {

                printf("[ AVISO ] Comentário não encontrado!\n");

                goto comment_end;

            }
            
        Q1:

            if (c != EOF && c != '\n') {

                commentariesBuffer[i] = c;
                    
                i++;
                
                goto Q1;
    
            } else if (c == '\n') {

                commentariesBuffer[i] = '\0';

                fputs(commentariesBuffer, commentariesIdentified);
    
                printf("[ SUCESSO ] Comentário aceito!\n");

                goto comment_end;
                
            } else {
                
                printf("[ ERRO ] Comentário não aceito!");
                
                goto comment_end;
                
            }
            
    comment_end:
            
    commentariesBuffer[0] = '\0';
    i = 0;

    fclose(copy);
    fclose(commentariesIdentified);

    return 0;
    
}

int fillSymbolTable() {

    getNextToken();

    return 0;

}