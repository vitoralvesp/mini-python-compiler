#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ******************************************************
// ETAPA 0: RECEBIMENTO DO ARQUIVO 
// ******************************************************

#define FILE_OUTPUT_PATH "../output/"

#define FILE_TEMP_PATH "../temp/"

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
int createFileCopy(int argc, char *argv[]);

/** @brief Recebe um caractere a ser removido de um arquivo e procede com a busca para removêlo, retornando 0 se o processo for bem-sucedido e 1 caso contrário.
 *  @param argc número de parâmetros recebidos na linha de comando.
 *  @param argv vetor com os parâmetros recebidos na linha de comando
 *  @param characterToRemove caractere a ser removido do arquivo.
 *  @return 0 se o arquivo for válido, 1 caso contrário.
 */
int removeCharacters(int argc, char *argv[], char *characterToRemove);


int main(int argc, char *argv[]) {

    if (readFile(argc, argv) != 0) {

        puts("[ ERRO ] Não foi possível abrir/ler o arquivo de entrada.");
        
        return 1;
    
    } else {

        puts("[ SUCESSO ] Arquivo lido com sucesso!");

    }

    if (createFileCopy(argc, argv) != 0) {

        puts("[ ERRO ] Não foi possível criar uma cópia do arquivo de entrada.");
        
        return 1;

    } else {

        puts("[ SUCESSO ] Arquivo copiado em '../output' com sucesso!");

    }

    int didCommentsRemovalProcessSucceed = removeCharacters(argc, argv, "#");
    
    int didWhiteSpacesRemovalProcessSucceed = removeCharacters(argc, argv, " ");
    
    int didTabsRemovalProcessSucceed = removeCharacters(argc, argv, "\t");

    int didLineBreaksRemovalProcessSucceed = removeCharacters(argc, argv, "\n");

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


int createFileCopy(int argc, char *argv[]) {

    const char *base = strrchr(argv[1], '/');
    
    base = base ? base + 1 : argv[1];

    char outputFilePath[256];
    
    snprintf(outputFilePath, sizeof(outputFilePath), "%s%s", FILE_OUTPUT_PATH, base);

    FILE *file = fopen(argv[1], "rb");

    FILE *copy = fopen(outputFilePath, "wb");

    int result = 0;

    if (!file || !copy) 
        result = 1;
    
    else {
        
        int c;

        while ((c = fgetc(file)) != EOF) {

                fputc(c, copy);
        
        }
        
        result = 0;
    }

    fclose(file);

    fclose(copy);
    
    return result;
}


int removeCharacters(int argc, char *argv[], char *characterToRemove) {

    const char *base = strrchr(argv[1], '/');
    
    base = base ? base + 1 : argv[1];

    char outputFilePath[256];

    char temporaryFilePath[256];
    
    snprintf(outputFilePath, sizeof(outputFilePath), "%s%s", FILE_OUTPUT_PATH, base);

    snprintf(temporaryFilePath, sizeof(temporaryFilePath), "%s%s", FILE_TEMP_PATH, base);

    FILE *file = fopen(outputFilePath, "rb");
    
    FILE *temporaryFile = fopen(temporaryFilePath, "wb");

    if (!file || !temporaryFile) 
        return 1;

    else {

        int c;

        if (strcmp(characterToRemove, "#") == 0) {

            int insideComment = 0;

            while ((c = fgetc(file)) != EOF) {

                if (c == '#') 
                    insideComment = 1;
                
                else if (c == '\n') 
                    insideComment = 0;

                if (!insideComment) 
                    fputc(c, temporaryFile);

            }

        } else {
            
            while ((c = fgetc(file)) != EOF) {
    
                if (c != (unsigned char)characterToRemove[0]) 
                    fputc(c, temporaryFile);
    
            }
            
        }

        fclose(file);

        fclose(temporaryFile);

        remove(outputFilePath);

        rename(temporaryFilePath, outputFilePath);

    }

    return 0;

}