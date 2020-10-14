/*
  Universidade de Sao Paulo - ICMC
  SSC0903 - Computacao de Alto Desempenho
  Atividade 2
  Grupo:
  João Pedro A. S. Secundino (10692054);
  João Pedro Uchôa Cavalcante (10801169);
  Luís Eduardo Rozante de Freitas Pereira (10734794);
  Sérgio Ricardo G. B. Filho (10408386);
*/

# include <omp.h>
# include <math.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include "atividade2.h"

# define THREADS 8

# define PRINT_TEMPO 1
# define PRINT_MATRIZ 1


int main() 
{

    // Escaneia o tamanho da matriz.
    size_t n, m;
    scanf(" %llu %llu", &n, &m);

    // Aloca a matriz transposta.
    double **matriz = calloc(m, sizeof(double*)); // Aloca as linhas.
    for(size_t i = 0; i < m; i++)
        matriz[i] = calloc(n, sizeof(double)); // Aloca a coluna.

    // Lê a entrada transposta.
    for(size_t i = 0; i < n; i++) {
        for(size_t j = 0; j < m; j++)
            scanf(" %lf", &(matriz[j][i]));
    }

    // Aloca a matriz de respostas.
    double **matriz_resposta = calloc(7, sizeof(double*)); // Aloca as linhas.
    for(size_t i = 0; i < 7; i++)
        matriz_resposta[i] = calloc(m, sizeof(double)); // Aloca a coluna.

    // Começa a contar o tempo.
    double wtime = omp_get_wtime ( );

    // (Não cria as tasks, pois essa é a versão sequêncial) Cria tasks para os dados de cada linha da matriz transposta.
    for(size_t i = 0; i < m; i++) {
        
        // Essas tarefas são executas sequêncialmente pois necessitam do resultado das anteriores, 
        // tentar paraleliza-las provavelmente causaria uma perda de tempo muito grande com a 
        // sincronização.

        double media_a = calcula_media_aritmetica(n, matriz[i]);
        double variancia = calcula_variancia(n, matriz[i], media_a); // Depende da média.
        double desvio = calcula_desvio_padrao(variancia); // Depende da variância(por consequência também depende da média).
        double coef_variacao = calcula_coeficiente_variacao(desvio, media_a); // Depende da media e do desvio padrão (por consequência também depende da variância).

        matriz_resposta[0][i] = media_a;
        matriz_resposta[4][i] = variancia;
        matriz_resposta[5][i] = desvio;
        matriz_resposta[6][i] = coef_variacao;

        matriz_resposta[1][i] = calcula_media_harmonica(n, matriz[i]);
    
        // Essas tarefas são executas juntas e sequencialmente pois necessitam da linha com os dados ordenada. 
        // Isso é feito para evitar ter que ordenar o vetor duas vezes e para facilitar a sincronização, já que 
        // a média e mediana são assintóticamente ingsignificantes comparadas a ordenção.

        // Aloca espaço para guardar a versão ordenada da linha e copia ela.
        double *linha_ord = malloc(n * sizeof(double));
        memcpy(linha_ord, matriz[i], n * sizeof(double));

        // Ordena os elementos da linha.
        qsort(linha_ord, n, sizeof(double), fun_comparacao); // O(n(log(n)))

        matriz_resposta[2][i] = calcula_mediana(n, linha_ord); // O(1)
        matriz_resposta[3][i] = calcula_moda(n, linha_ord); // O(n)

        // Libera a memória usada para o guardar a linha ordenada.
        free(linha_ord);

    }

    // Pega o tempo final e calcula a durção da execução.
    wtime = omp_get_wtime ( ) - wtime;

    // Printa a matriz com os resultados
    # if PRINT_MATRIZ
        printa_matriz(7, m, matriz_resposta);
    # endif
    
    # if PRINT_TEMPO
        printf("\nElapsed wall clock time = %.5f\n", wtime );
    # endif

    // Desaloca a matriz transposta.
    for(size_t i = 0; i < m; i++)
        free(matriz[i]);
    
    free(matriz);

    // Desaloca a matriz de resposta.
    for(size_t i = 0; i < 7; i++)
        free(matriz_resposta[i]);
    
    free(matriz_resposta);

    return 0;
}
