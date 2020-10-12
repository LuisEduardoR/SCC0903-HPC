# include <omp.h>
# include <math.h>
# include <stdio.h>
# include <stdlib.h>

# define THREADS 8

void printa_matriz(size_t n, size_t m, double **mat) {

    for(size_t i = 0; i < n; i++) {
        for(size_t j = 0; j < m; j++)
            printf(" %lf\t", mat[i][j]);
        printf("\n");
    }

}

void calcula_media_aritmetica(size_t n, int i, double *linha, double **resposta) {
    resposta[0][i] = i * 10000 + 0; // TODO: substituir pelo resultado
}

void calcula_media_harmonica(size_t n, int i, double *linha, double **resposta) {
    resposta[1][i] = i * 10000 + 1; // TODO: substituir pelo resultado
}

void calcula_mediana(size_t n, int i, double *linha, double **resposta) {
    resposta[2][i] = i * 10000 + 2; // TODO: substituir pelo resultado
}

void calcula_moda(size_t n, int i, double *linha, double **resposta) {
    resposta[3][i] = i * 10000 + 3; // TODO: substituir pelo resultado
}   

void calcula_variancia(size_t n, int i, double *linha, double **resposta) {
    // media_aritmetica: resposta[0][]
    resposta[4][i] = i * 10000 + 4; // TODO: substituir pelo resultado
}

void calcula_desvio_padrao(size_t n, int i, double *linha, double **resposta) {
    // media_aritmetica: resposta[4][]
    resposta[5][i] = i * 10000 + 5; // TODO: substituir pelo resultado
}

void calcula_coeficiente_variacao(size_t n, int i, double *linha, double **resposta) {
    // media_aritmetica: resposta[5][]
    resposta[6][i] = i * 10000 + 6; // TODO: substituir pelo resultado
}

int main() {

    // Escaneia o tamanho da matriz.
    size_t n, m;
    scanf(" %d %d", &n, &m);

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

    # pragma omp parallel num_threads(THREADS) shared(m, n, matriz, matriz_resposta)
    {

        # pragma omp single
        {

            // Cria as tasks para cada linha da matriz transposta.
            for(size_t i = 0; i < m; i++) {

                // Cria a task da média aritmética.
                # pragma omp task
                {
                    // Essas tarefas são executas sequêncialmente pois necessitam do resultado das anteriores, 
                    // tentar paraleliza-las provavelmente causaria uma perda de tempo muito grande com a 
                    // sincronização.
                    calcula_media_aritmetica(n, i, matriz[i], matriz_resposta);
                    calcula_variancia(n, i, matriz[i], matriz_resposta);
                    calcula_desvio_padrao(n, i, matriz[i], matriz_resposta);
                    calcula_coeficiente_variacao(n, i, matriz[i], matriz_resposta);
                }

                // Cria a task da média harmônica.
                # pragma omp task
                {
                    calcula_media_harmonica(n, i, matriz[i], matriz_resposta);
                }

                // Cria a task da mediana.
                # pragma omp task
                {
                    calcula_mediana(n, i, matriz[i], matriz_resposta);
                }

                // Cria a task da moda.
                # pragma omp task
                {
                    calcula_moda(n, i, matriz[i], matriz_resposta);
                }

            }

        }

    }

    // Printa a matriz
    printa_matriz(7, m, matriz_resposta);
    
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