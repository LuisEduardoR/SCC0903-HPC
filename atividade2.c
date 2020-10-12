# include <omp.h>
# include <math.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# define THREADS 8

void printa_matriz(size_t n, size_t m, double **mat) {

    for(size_t i = 0; i < n; i++) {
        for(size_t j = 0; j < m; j++)
            printf(" %.1lf\t", mat[i][j]);
        printf("\n");
    }

}

int cmpValues (const void *a, const void *b){
    return ( *(double *)a - *(double *)b );
}

// Média aritmética Somatório de todos os elementos da amostra, divididos pelo tamanho
// da amostra.
double calcula_media_aritmetica(size_t len, double *linha) {

    // Calcula o somatório dos elementos da linha.
    double sum = 0;
    for(size_t i = 0; i < len; i++)
        sum += linha[i];

    // Retorna a média aritmética dos elementos.
    return (sum / len);

}

// Média harmônica: Razão entre o tamanho da amostra e o somatório do inverso das amostras.
double calcula_media_harmonica(size_t len, double *linha) {

    // Calcula o somatório dos inversos dos elementos da linha.
    double inv_sum = 0;
    for(size_t i = 0; i < len; i++)
        inv_sum += 1 / linha[i];

    // Retorna a média harmônica dos elementos.
    return (len / inv_sum);

}

// Mediana: Elemento médio da amostra (elemento médio da coluna ordenada). Para um
// número par de elementos, a mediana é a média entre os elementos do meio
// ((n/2+n/2+1)/2).
double calcula_mediana(size_t len, double *linha) {
    
    // Copiando vetor
    double *copied_vec = malloc( len * sizeof(double) );
    memcpy(copied_vec, linha, len * sizeof(double));
    
    // Ordenando a copia -> bubble :)
    /*for(int i=0; i<len; i++){
        for(int j=i+1; j<len; j++){
            if(copied_vec[i] > copied_vec[j]){
                double aux = copied_vec[i];
                copied_vec[i] = copied_vec[j];
                copied_vec[j] = aux;
            }
        }
    }
    */
    // ordena com quicksort ;)
    qsort(copied_vec, len, sizeof(double), cmpValues);

    double mediana;
    if (len % 2 == 0)
        // media entre os elementos do meio
        mediana = (copied_vec[len/2 -1] +copied_vec[len/2])/2;
    else
        mediana = copied_vec[len/2 -1];

    free(copied_vec);
    return mediana; 
}

// Moda: Elemento mais frequente da amostra (elemento que mais aparece na coluna, se
// houver mais de um, considera-se somente o primeiro. Se não houver, retorna -1).
double calcula_moda(size_t len, double *linha) {
    
    // id na pos 0 e ocorrencia na pos 1
    int id_ocorrencias[2] = {0, 1};
    
    for (int i=0; i<len; i++){
        // inicia considerando primeira ocorrencia
        int ocorrencias = 1;
        for (int j=i+1; j<len; j++){
            // acumula novas ocorrencias
            if(linha[i] == linha[j])
                ocorrencias += 1; 
        }
        
        // troca moda se ocorreu mais vezes
        if (ocorrencias > id_ocorrencias[1]){
            id_ocorrencias[0] = i;
            id_ocorrencias[1] = ocorrencias;
        }
    }

    return id_ocorrencias[1] > 1? linha[id_ocorrencias[0]] : -1.0; 
}   

// Variância: Soma dos quadrados das diferenças entre o elemento da amostra e a média
// aritmética calculada.
double calcula_variancia(size_t len, double *linha, double media) {

    // Calcula o somatório dos quadrados das diferenças da linha.
    double sum = 0;
    for(size_t i = 0; i < len; i++) {
        double dif = (linha[i] - media);
        sum += (dif * dif);
    }

    // Retorna a variância da amostra.
    return sum / (len - 1);
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
                    matriz_resposta[4][i] = variancia;
                    matriz_resposta[5][i] = desvio;
                    matriz_resposta[6][i] = coef_variacao;

                }

                // Cria a task da média harmônica.
                # pragma omp task
                {
                    matriz_resposta[1][i] = calcula_media_harmonica(n, matriz[i]);
                }

                // Cria a task da mediana.
                # pragma omp task
                {
                    matriz_resposta[2][i] = calcula_mediana(n, matriz[i]);
                }

                // Cria a task da moda.
                # pragma omp task
                {
                    matriz_resposta[3][i] = calcula_moda(n, matriz[i]);
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
