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

# define PRINT_SAIDA_DETALHADA 0
# define PRINT_CHAR_TASKS 0
# define PRINT_FREQ 0
# define PRINT_TEMPO 0

# include <omp.h>
# include <math.h>
# include <stdio.h>
# include <stdlib.h>

void calc_perimeter(float base, float height, float *anwser) {
    *anwser = (2 * base + 2 * height);
}

void calc_area(float base, float height, float *anwser) {
    *anwser = (base * height);
}

void calc_diag(float base, float height, float *anwser) {
    *anwser = sqrt(base * base + height * height);
}

int main() {

    // Escaneia o número de retângulos
    int n;
    scanf(" %d", &n);

    // Aloca espaço para guardar as bases e as alturas
    float *bases = malloc(n * sizeof(float));
    float *heights = malloc(n *sizeof(float));

    // Recebe as bases.
    for(int i = 0; i < n; i++) {
        float temp;
        scanf(" %f", &temp);
        bases[i] = temp;
    }

    // Recebe as alturas.
    for(int i = 0; i < n; i++) {
        float temp;
        scanf(" %f", &temp);
        heights[i] = temp;
    }

    // Aloca uma matriz para as respostas.
    float **awnsers = malloc(3 * sizeof(float*)); // Aloca as linhas (uma para cada métrica)
    awnsers[0] = malloc(n * sizeof(float)); // (Aloca espaço para a respsota de cada vetor na métrica "perimetro")
    awnsers[1] = malloc(n * sizeof(float)); // (Aloca espaço para a respsota de cada vetor na métrica "area")
    awnsers[2] = malloc(n * sizeof(float)); // (Aloca espaço para a respsota de cada vetor na métrica "diagonal")

    // Faz os calculos necessários.
    # pragma omp parallel num_threads(THREADS) shared(n, bases, heights, awnsers)
    {

        # pragma omp single 
        {

            // Cria as tasks para cada retângulo.
            for(int i = 0; i < n; i++) {

                # pragma omp task
                {
                    calc_perimeter(bases[i], heights[i], &(awnsers[0][i]));
                }
                # pragma omp task
                {
                    calc_area(bases[i], heights[i], &(awnsers[1][i]));
                }
                # pragma omp task
                {
                    calc_diag(bases[i], heights[i], &(awnsers[2][i]));
                }
                
            }

        }
    }

    // Escreve a saída.
    for(int i = 0; i < n; i++) {
        printf("base[%d]=%.0f, ", i, bases[i]);
        printf("alt[%d]=%.0f, ", i, heights[i]);
        printf("per[%d]=%.0f, ", i, awnsers[0][i]);
        printf("area[%d]=%.0f, ", i, awnsers[1][i]);
        printf("diag[%d]=%.2f\n", i, awnsers[2][i]);
    }

    // Libera a memória alocada.
    free(bases);
    free(heights);
    free(awnsers[0]);
    free(awnsers[1]);
    free(awnsers[2]);
    free(awnsers);

    return 0;

}
