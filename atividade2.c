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

double calcula_media_aritmetica(size_t len, double *linha) {
    
    double soma_acc = 0;
    for (int i=0; i < len; i++)
        soma_acc += linha[i];

    return soma_acc/len; // TODO: substituir pelo resultado
}

double calcula_media_harmonica(size_t len, double *linha) {
    return 1; // TODO: substituir pelo resultado
}

double calcula_mediana(size_t len, double *linha) {
    return 2; // TODO: substituir pelo resultado
}

double calcula_moda(size_t len, double *linha) {
    return 3; // TODO: substituir pelo resultado
}   

double calcula_variancia(size_t len, double *linha, double media) {
    // media_aritmetica: resposta[0][]
    return 4; // TODO: substituir pelo resultado
}

// Desvio padrão: Raiz quadrada da variância.
double calcula_desvio_padrao(double variancia) {
    return sqrt(variancia);
}

// Coeficiente de variação: Razão entre o desvio padrão e a média aritmética. 
double calcula_coeficiente_variacao(double desvio, double media) {
    return desvio / media;
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
                    double media_a = calcula_media_aritmetica(n, matriz[i]);
                    double variancia = calcula_variancia(n, matriz[i], media_a); // Depende da média.
                    double desvio = calcula_desvio_padrao(variancia); // Depende da variância(por consequência também depende da média).
                    double coef_variacao = calcula_coeficiente_variacao(desvio, media_a); // Depende da media e do desvio padrão (por consequência também depende da variância).

                    matriz_resposta[0][i] = media_a;
                    matriz_resposta[4][i] = i * 10000 + variancia;
                    matriz_resposta[5][i] = i * 10000 + desvio;
                    matriz_resposta[6][i] = i * 10000 + coef_variacao;
                }

                // Cria a task da média harmônica.
                # pragma omp task
                {
                    matriz_resposta[1][i] = i * 10000 + calcula_media_harmonica(n, matriz[i]);
                }

                // Cria a task da mediana.
                # pragma omp task
                {
                    matriz_resposta[2][i] = i * 10000 + calcula_mediana(n, matriz[i]);
                }

                // Cria a task da moda.
                # pragma omp task
                {
                    matriz_resposta[3][i] = i * 10000 + calcula_moda(n, matriz[i]);
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
