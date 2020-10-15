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

# include "atividade2.h"

# define THREADS 8

# define PRINT_TEMPO 1
# define PRINT_MATRIZ 1

int main() {

    // Escaneia o tamanho da matriz.
    size_t n, m;
    scanf(" %lu %lu", &n, &m);

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

    # pragma omp parallel num_threads(THREADS) shared(m, n, matriz, matriz_resposta)
    {

        # pragma omp single
        {

            // Pega o número de threads usadas pelo OMP.
            int n_threads = omp_get_num_threads();

            // Divide as linhas da matriz transposta baseada no número de threads e cria tasks para
            // fazer os cálculos necessários nessas regiões.
            int div_linhas = (m / n_threads);
            int resto = m % n_threads;
            for(size_t i = 0; i < (n_threads < m ? n_threads : m); i++) {

                // Calcula a primeira e última linha que serão atribuídas as tasks nessa iteração.
                int inicio, fim;
                inicio = i * div_linhas + i * (i < resto) + resto * (i >= resto);
                fim = inicio + div_linhas + 1 * (i < resto);

                // Crias as tasks para essa região caso ela não seja vazia.
                cria_task(matriz, matriz_resposta, n, inicio, fim);

            }

        }

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
