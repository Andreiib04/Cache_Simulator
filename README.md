# Cache Simulator Readme

Este é um simulador de cache implementado em C que permite simular o comportamento de uma memória cache com diferentes políticas de substituição (LRU, FIFO e Random). O simulador recebe um arquivo de entrada contendo endereços de memória e gera estatísticas sobre o desempenho da cache com base nas configurações fornecidas.

## Como Usar

Para usar este simulador, siga estas etapas:

1. Compile o código fonte: Você deve compilar o código C antes de executá-lo. Use o compilador GCC ou um ambiente de desenvolvimento C de sua escolha.

   ```bash
   gcc -o cache_simulator cache_simulator.c -lm
   ```

2. Execute o programa: Após a compilação bem-sucedida, você pode executar o programa com os argumentos necessários.

   ```bash
   ./cache_simulator <nsets> <bsize> <assoc> <substituição> <flag_saida> arquivo_de_entrada
   ```

   - `<nsets>`: O número de conjuntos na cache.
   - `<bsize>`: O tamanho do bloco da cache.
   - `<assoc>`: A associatividade da cache (1 para mapeamento direto, >1 para associativa).
   - `<substituição>`: A política de substituição ('r' para Random, 'f' para FIFO, 'l' para LRU).
   - `<flag_saida>`: Uma flag para escolher o formato de saída (0 para formato personalizado, 1 para formato padrão).
   - `arquivo_de_entrada`: O arquivo de entrada contendo os endereços de memória a serem acessados pela cache.

3. Observe a Saída: O programa calculará estatísticas de desempenho da cache e imprimirá os resultados no console.

## Estrutura do Código

- O código é dividido em várias funções para facilitar a leitura e a manutenção.
- A estrutura principal é a `Cache`, que representa a configuração da cache e contém os conjuntos de cache.
- Os principais métodos são:
  - `createCache`: Cria uma instância da estrutura de cache com base nas configurações fornecidas.
  - `access_cache`: Simula o acesso à cache com base no endereço de memória.
  - `destroyCache`: Libera a memória alocada para a estrutura da cache.
  - `printFlagOut`: Imprime informações detalhadas sobre a cache e o desempenho.
- A função `main` lê os argumentos da linha de comando, abre o arquivo de entrada, simula o acesso à cache e imprime os resultados.

## Políticas de Substituição

O simulador suporta três políticas de substituição:

- Random ('r' ou 'R'): Escolhe aleatoriamente uma linha da cache para substituição.
- FIFO ('f' ou 'F'): Substitui a linha mais antiga na cache.
- LRU ('l' ou 'L'): Substitui a linha menos usada (Least Recently Used).

## Saída

O programa pode produzir duas saídas diferentes, dependendo do valor da flag de saída:

1. Formato Padrão (flag = 1):
   - Número total de acessos.
   - Número total de hits.
   - Número total de misses.
   - Taxa de miss compulsório.
   - Taxa de miss de capacidade.
   - Taxa de miss de conflito.

2. Formato Personalizado (flag = 0):
   - Mostra uma visualização detalhada dos conjuntos da cache, incluindo os valores armazenados em cada conjunto.
   - Exibe informações sobre a configuração da cache, associatividade, tamanho do bloco e política de substituição.
   - Apresenta as taxas de acertos e faltas, incluindo faltas compulsórias, de capacidade e de conflito.

A escolha entre os formatos de saída depende do seu objetivo ao utilizar o simulador.

Certifique-se de que os arquivos de entrada e saída estejam configurados corretamente antes de executar o programa.
