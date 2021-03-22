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
# include <stdio.h>
# include <stdlib.h>

int main() {

    // Escaneia o tamanho da matriz.
    int n;
    scanf(" %d ", &n);

    // Aloca a matriz para guardar a entrada.
    char *string = malloc(n * sizeof(char));

    // Lê a entrada.
    for(int i = 0; i < n; i++)
        string[i] = getchar();

    // Array que guarda as frequências de cada carácter.
    int freq[256];
    // Inicia a array
    for(int i = 0; i < 256; i++)
        freq[i] = 0;

    // Começa a contar o tempo.
    double wtime = omp_get_wtime ();

    omp_lock_t mylock;
    omp_init_lock(&mylock);

    # pragma omp parallel for num_threads(THREADS) shared(n, string) reduction(+: freq)
    for(int i = 0; i < n; i++)
    {

        // Contas os caracteres desse pedaço da string.            
        freq[(int)string[i]]++;    

    }

    // Agora encontramos o caractér com a maior frequência sequencialmente.
    char max_char;
    int max_freq = -1;
    for(int i = 0; i < 256; i++) {
        if(freq[i] > max_freq) {
            max_freq = freq[i];
            max_char = (char)i;
        }
    }

    // Pega o tempo final e calcula a durção da execução.
    wtime = omp_get_wtime ( ) - wtime;

    // Printa o resultado.
    # if PRINT_SAIDA_DETALHADA
    printf("Maior frequencia: %c (ASCII=%d)\n", max_char, (int)max_char);
    # else
    printf("%c\n", max_char);
    # endif

    // Printa o vetor com as frequências.
    # if PRINT_FREQ
        for(int i = 0; i < 256; i++) {
            printf("%c : %d \t", (char)i, freq[i]);
            if(i != 0 && i % 8 == 0)
                putchar('\n');
        }
    # endif
    
    # if PRINT_TEMPO
        printf("\nElapsed wall clock time = %.5f\n", wtime );
    # endif

    // Libera a memória alocada pela entrada.
    free(string);

    return 0;
}
