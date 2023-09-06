#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <limits.h>

typedef struct
{
    uint32_t tag, value; // Campos para a tag e valor da linha da cache.
    bool valid;          // Indica se a linha da cache é válida.
    int countLru;        // Contador para política de substituição LRU.
    int countFifo;       // Contador para política de substituição FIFO.
} CacheLine;

typedef struct
{
    CacheLine *block; // Array de linhas da cache para um conjunto.
} CacheSet;

typedef struct
{
    int countAcess;  // Contador de acessos à cache.
    char *subst;     // Política de substituição ('r' para random, 'f' para FIFO, 'l' para LRU).
    int nsets;       // Número de conjuntos na cache.
    int bsize;       // Tamanho do bloco da cache.
    int assoc;       // Associatividade da cache.
    int block_count; // Contador de blocos na cache.
    CacheSet *sets;  // Conjuntos da cache.
} Cache;

uint32_t ReverseBytes(uint32_t bytes); // Função para reverter os bytes de um número.

Cache *createCache(int nsets, int bsize, int assoc, char *subst); // Função para criar uma estrutura de cache.
void print_number(uint32_t address);                              // Função para imprimir um número em binário.
void destroyCache(Cache *cache);                                  // Função para destruir a estrutura da cache.
void access_cache(Cache *cache, uint32_t end, float *hits, float *misses,
                  float *miss_compulsorio, float *miss_capacidade,
                  float *miss_conflito); // Função para acessar a cache.
void printFlagOut(Cache *cache, float *hits, float *misses,
                  float *miss_compulsorio, float *miss_capacidade,
                  float *miss_conflito, int numadreesss); // Função para imprimir informações sobre a cache.

int main(int argc, char *argv[])
{
    SetConsoleOutputCP(65001); // Configura a codificação de saída para UTF-8.

    if (argc != 7)
    {
        printf("./cache_simulator <nsets> <bsize> <assoc> <substituição> "
               "<flag_saida> arquivo_de_entrada\n");
        exit(EXIT_FAILURE); // Sai do programa com código de erro.
    }

    // Extrai argumentos da linha de comando.
    int nsets = atoi(argv[1]);
    int bsize = atoi(argv[2]);
    int assoc = atoi(argv[3]);
    char *subst = argv[4];
    int flagOut = atoi(argv[5]);
    char *arquivoEntrada = argv[6];

    // Cria uma estrutura de cache com base nos argumentos.
    Cache *cache = createCache(nsets, bsize, assoc, subst);

    float hits = 0, misses = 0, miss_compulsorio = 0, miss_capacidade = 0,
          miss_conflito = 0;

    // Abre o arquivo de entrada.
    FILE *fp = fopen(arquivoEntrada, "rb");
    if (fp == NULL)
    {
        printf("Erro ao abrir o arquivo.\n");
        exit(EXIT_FAILURE); // Sai do programa com código de erro.
    }

    uint32_t end;
    int numadreesss = 0;

    // Lê endereços do arquivo de entrada e acessa a cache.
    while (fread(&end, sizeof(uint32_t), 1, fp) == 1)
    {
        numadreesss++;
        end = ReverseBytes(end);
        access_cache(cache, end, &hits, &misses, &miss_compulsorio,
                     &miss_capacidade, &miss_conflito);
    }

    fclose(fp); // Fecha o arquivo de entrada.

    if (flagOut == 1)
    {
        printf("\n%d %.4f %.4f %.2f %.2f %.2f", numadreesss, hits / numadreesss,
               misses / numadreesss, miss_compulsorio / misses,
               miss_capacidade / misses, miss_conflito / misses);
    }
    else
    {
        printFlagOut(cache, &hits, &misses, &miss_compulsorio, &miss_capacidade,
                     &miss_conflito, numadreesss);
    }

    destroyCache(cache); // Libera a memória alocada para a cache.

    return 0;
}

uint32_t ReverseBytes(uint32_t bytes)
{
    uint32_t aux = 0;
    uint8_t byte;

    for (int i = 0; i < 32; i += 8)
    {
        byte = (bytes >> i) & 0xff;
        aux |= byte << (32 - 8 - i);
    }

    return (aux);
}

Cache *createCache(int nsets, int bsize, int assoc, char *subst)
{
    Cache *cache = malloc(sizeof(Cache)); // Aloca memória para a estrutura de cache.
    cache->nsets = nsets;
    cache->bsize = bsize;
    cache->assoc = assoc;
    cache->subst = subst;
    cache->block_count = 0;
    cache->countAcess = 0;

    cache->sets = malloc(nsets * sizeof(CacheSet)); // Aloca memória para os conjuntos da cache.

    for (int i = 0; i < nsets; i++)
    {
        cache->sets[i].block = malloc(assoc * sizeof(CacheLine)); // Aloca memória para as linhas de cache em cada conjunto.
        for (int j = 0; j < assoc; j++)
        {
            cache->sets[i].block[j].value = -1;     // Inicializa o valor da linha da cache.
            cache->sets[i].block[j].valid = false;  // Inicializa a validade da linha da cache.
            cache->sets[i].block[j].countLru = -1;  // Inicializa o contador LRU.
            cache->sets[i].block[j].countFifo = -1; // Inicializa o contador FIFO.
        }
    }

    return cache;
}

void access_cache(Cache *cache, uint32_t end, float *hits, float *misses,
                  float *miss_compulsorio, float *miss_capacidade,
                  float *miss_conflito)
{
    int bitsOffset = (int)log2(cache->bsize); // Calcula o número de bits de deslocamento.
    int bitsIndex = (int)log2(cache->nsets);  // Calcula o número de bits de índice.
    uint32_t tag, index;
    tag = end >> (bitsOffset + bitsIndex);      // Calcula a tag do endereço.
    index = (end >> bitsOffset) % cache->nsets; // Calcula o índice do conjunto.
    cache->countAcess++;                        // Incrementa o contador de acessos à cache.

    bool hit = false;
    for (int i = 0; i < cache->assoc; i++)
    {
        if (cache->sets[index].block[i].valid &&
            cache->sets[index].block[i].tag == tag)
        {
            hit = true;
            (*hits)++;                                                 // Incrementa o contador de acertos.
            cache->sets[index].block[i].countLru = cache->countAcess;  // Atualiza o contador LRU.
            cache->sets[index].block[i].countFifo = cache->countAcess; // Atualiza o contador FIFO.
            break;
        }
    }

    if (!hit)
    {
        int block_to_replace = -1;
        (*misses)++; // Incrementa o contador de faltas.
        int empty_block = -1;
        for (int i = 0; i < cache->assoc; i++)
        {
            if (!cache->sets[index].block[i].valid)
            {
                empty_block = i;
                break;
            }
        }

        if (empty_block != -1)
        {
            cache->sets[index].block[empty_block].valid = true;
            cache->sets[index].block[empty_block].tag = tag;
            (*miss_compulsorio)++; // Incrementa o contador de faltas compulsórias.
            cache->sets[index].block[empty_block].value = end;
            cache->block_count++;

            cache->sets[index].block[empty_block].countLru = cache->countAcess;  // Atualiza o contador LRU.
            cache->sets[index].block[empty_block].countFifo = cache->countAcess; // Atualiza o contador FIFO.
            return;
        }
        else
        {
            if (strcmp(cache->subst, "r") == 0 ||
                strcmp(cache->subst, "R") == 0)
            {
                block_to_replace = rand() % cache->assoc; // Escolhe aleatoriamente uma linha para substituição.
            }
            else if (strcmp(cache->subst, "f") == 0 ||
                     strcmp(cache->subst, "F") == 0)
            {
                int menor = cache->sets[index].block[0].countFifo;
                for (int i = 1; i < cache->assoc; i++)
                {
                    if (cache->sets[index].block[i].countFifo < menor)
                    {
                        menor = cache->sets[index].block[i].countFifo;
                    }
                }

                for (int i = 0; i < cache->assoc; i++)
                {
                    if (menor == cache->sets[index].block[i].countFifo)
                    {
                        block_to_replace = i; // Escolhe a linha com o menor contador FIFO para substituição.
                    }
                }
            }
            else if (strcmp(cache->subst, "l") == 0 ||
                     strcmp(cache->subst, "L") == 0)
            {
                int menor = cache->sets[index].block[0].countLru;
                for (int i = 1; i < cache->assoc; i++)
                {
                    if (cache->sets[index].block[i].countLru < menor)
                    {
                        menor = cache->sets[index].block[i].countLru;
                    }
                }

                for (int i = 0; i < cache->assoc; i++)
                {
                    if (menor == cache->sets[index].block[i].countLru)
                    {
                        block_to_replace = i; // Escolhe a linha com o menor contador LRU para substituição.
                    }
                }
            }
            else
            {
                printf("Política de substituição inválida: %s\n", cache->subst);
                exit(1); // Sai do programa com código de erro.
            }
            cache->sets[index].block[block_to_replace].tag = tag;
            cache->sets[index].block[block_to_replace].valid = true;
            cache->sets[index].block[block_to_replace].value = end;
            cache->sets[index].block[block_to_replace].countLru = cache->countAcess;  // Atualiza o contador LRU.
            cache->sets[index].block[block_to_replace].countFifo = cache->countAcess; // Atualiza o contador FIFO.
        }

        if (cache->block_count == cache->nsets * cache->assoc)
        {
            (*miss_capacidade)++; // Incrementa o contador de faltas de capacidade.
        }
        else
        {
            (*miss_conflito)++; // Incrementa o contador de faltas de conflito.
        }
    }
}

void destroyCache(Cache *cache)
{
    for (int i = 0; i < cache->nsets; i++)
    {
        free(cache->sets[i].block); // Libera a memória das linhas de cache em cada conjunto.
    }

    free(cache->sets); // Libera a memória dos conjuntos da cache.
    free(cache);       // Libera a memória da estrutura de cache.
}
void printFlagOut(Cache *cache, float *hits, float *misses,
                  float *miss_compulsorio, float *miss_capacidade,
                  float *miss_conflito, int numadreesss)
{
    printf("\n\n");
    printf("\t\t\t\t\t\t\t\t\t\tIMPRIMINDO DADOS DA CACHE\n\n\n");
    for (int i = 0; i < cache->nsets; i++)
    {
        printf("\nÍndice %d", i);
        for (int j = 0; j < cache->assoc; j++)
        {
            printf("\t Conjunto %d value: %d", j, cache->sets[i].block[j].value);
        }
    }
    printf("Número de Conjuntos: %d\t", cache->nsets);
    printf("Tamanho do Bloco: %d\t", cache->bsize);
    printf("Associatividade: %d\n", cache->assoc);
    if (cache->assoc == 1)
    {
        printf("Mapeamento direto");
    }
    else if (cache->nsets == 1)
    {
        printf("Totalmente Associativa");
    }
    else
    {
        printf("Associativa por conjuntos");
    }
    printf("\nTipo de Substituição: ");
    if (strcmp(cache->subst, "r") == 0 || strcmp(cache->subst, "R") == 0)
    {
        printf("RANDOM \n");
    }
    else if (strcmp(cache->subst, "f") == 0 || strcmp(cache->subst, "F") == 0)
    {
        printf("FIFO \n");
    }
    else if (strcmp(cache->subst, "l") == 0 || strcmp(cache->subst, "L") == 0)
    {
        printf("LRU \n");
    }
    printf("---------------------------------------------- \n");
    printf("Acessos\t| %d\nhits\t| %.0f (%.2f%%)\n", numadreesss, *hits,
           (*hits / numadreesss) * 100);
    printf("Misses\t| %.0f (%.2f%%)\n", *misses, (*misses / numadreesss) * 100);
    printf("---------------------------------------------- \n");
    printf("Misses Compulsórios\t| %.0f (%.2f%%) \nMisses de Capacidade\t| %.0f "
           "(%.2f%%) \nMisses de Conflito\t| %.0f (%.2f%%) \n,",
           *miss_compulsorio, (*miss_compulsorio / *misses) * 100,
           *miss_capacidade, (*miss_capacidade / *misses) * 100, *miss_conflito,
           (*miss_conflito / *misses) * 100);
    printf("============================================== \n");
}

void print_number(uint32_t address)
{
    for (int i = 31; i >= 0; i--)
    {
        printf("%d", (address >> i) & 1); // Imprime os bits do número em binário.
    }
    printf("\n%u\n", address); // Imprime o número em decimal.
}
