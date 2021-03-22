/*
  Universidade de Sao Paulo - ICMC
  SSC0903 - Computacao de Alto Desempenho
  Atividade Aula
  Grupo:
  João Pedro A. S. Secundino (10692054);
  João Pedro Uchôa Cavalcante (10801169);
  Luís Eduardo Rozante de Freitas Pereira (10734794);
  Sérgio Ricardo G. B. Filho (10408386);
*/

# define THREADS 8
# define PRINT_TIME 0

# include <omp.h>
# include <math.h>
# include <stdio.h>
# include <stdlib.h>

int main() {

    // Escaneia o tamanho das matrizes
    int n;
    scanf(" %d", &n);

    // Aloca espaço para guardar as matrizes
    // - A
    double **A = malloc(n * sizeof(double*));
    for(int i = 0; i < n; i++)
        A[i] = malloc(n * sizeof(double));
    // - B
    double **B = malloc(n * sizeof(double*));
    for(int i = 0; i < n; i++)
        B[i] = malloc(n * sizeof(double));
    // - C
    double **C = malloc(n * sizeof(double*));
    for(int i = 0; i < n; i++)
        C[i] = malloc(n * sizeof(double));
    // - D
    double **D = malloc(n * sizeof(double*));
    for(int i = 0; i < n; i++)
        D[i] = malloc(n * sizeof(double));
    // - Intermediario 1
    double **I1 = malloc(n * sizeof(double*));
    for(int i = 0; i < n; i++)
        I1[i] = malloc(n * sizeof(double));
    // - Intermediario 2
    double **I2 = malloc(n * sizeof(double*));
    for(int i = 0; i < n; i++)
        I2[i] = malloc(n * sizeof(double));
    // - Resposta
    double **R = malloc(n * sizeof(double*));
    for(int i = 0; i < n; i++)
        R[i] = malloc(n * sizeof(double));

    // Recebe as matrizes =============================================
    // - A
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            double temp;
            scanf(" %lf", &temp);
            A[i][j] = temp;
        }
    }

    // - B
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            double temp;
            scanf(" %lf", &temp);
            B[i][j] = temp;
        }
    }

    // - C
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            double temp;
            scanf(" %lf", &temp);
            C[i][j] = temp;
        }
    }

    // - D
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            double temp;
            scanf(" %lf", &temp);
            D[i][j] = temp;
        }
    }

    // Começa a contar o tempo.
    double wtime = omp_get_wtime ();

    // Faz a computação ===============================================
    # pragma omp parallel num_threads(THREADS) shared(n, A, B, C, D, I1, I2, R)
    {

        // Multiplica A*B e B*C
        // As linhas são divididas em THREADS grupos com
        // tamanhos sendo o resto dividido igualmente entre
        // eles, isso é feito pelo omp for.
        // A divisão é feita em linhas, pois isso otimiza os
        // acessos a cache e evita falso compartilhamento ao
        // escrever para as matrizes intermediárias I1 e I2.
        # pragma omp for
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < n; j++) {
                
                // Inicializa I1[i][j] e I2[i][j] com a identidade 
                // da soma, para poder fazer a acumulação.
                I1[i][j] = 0;
                I2[i][j] = 0;

                // Acumula para I1[i][j] e I2[i][j] a multiplicação
                // dos elementos de uma linha i da matriz A/C para
                // uma coluna j da matriz B/D.
                for(int k = 0; k < n; k++) {
                    I1[i][j] += A[i][k] * B[k][i];
                    I2[i][j] += C[i][k] * D[k][i];
                }

            }
        }

        // Faz a sincronização antes da soma, pois essa
        // depende dos resultados intermediários produzidos
        // pela multiplicação.
        # pragma omp barrier

        // Soma (A*B) + (B*C)
        // As linhas são divididas em THREADS grupos com
        // tamanhos sendo o resto dividido igualmente entre
        // eles, isso é feito pelo omp for.
        // A divisão é feita em linhas, pois isso otimiza os
        // acessos a cache e evita falso compartilhamento ao
        // escrever para a matriz de resposta R.
        # pragma omp for
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < n; j++) {
                R[i][j] = I1[i][j] + I2[i][j];
            }
        }

    }

    // Pega o tempo final e calcula a durção da execução.
    wtime = omp_get_wtime() - wtime;

    // Printa a resposta. =============================================
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            printf("%.1lf ", R[i][j]);
        }
        printf("\n");
    }

    # if PRINT_TIME
        // Printa o tempo.
        printf("Tempo: %.8f\n", wtime);
    # endif

    // Libera a memória alocada. ======================================
    // - A
    for(int i = 0; i < n; i++)
        free(A[i]);
    free(A);
    // - B
    for(int i = 0; i < n; i++)
        free(B[i]);
    free(B);
    // - C
    for(int i = 0; i < n; i++)
        free(C[i]);
    free(C);
    // - D
    for(int i = 0; i < n; i++)
        free(D[i]);
    free(D);
    // - Intermediario 1
    for(int i = 0; i < n; i++)
        free(I1[i]);
    free(I1);
    // - Intermediario 2
    for(int i = 0; i < n; i++)
        free(I2[i]);
    free(I2);
    // - Resposta
    for(int i = 0; i < n; i++)
        free(R[i]);
    free(R);

    return 0;

}
