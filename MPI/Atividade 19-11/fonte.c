
/*
  Universidade de Sao Paulo - ICMC
  SSC0903 - Computacao de Alto Desempenho
  Atividade Aula
  Grupo:
  João Pedro A. S. Secundino (10692054);
  João Pedro Uchôa Cavalcante (10801169);
  Luís Eduardo Rozante de Freitas Pereira (10734794);
  Sérgio Ricardo G. B. Filho (10408386);

PCAM

P - Particionamento

Sejam A, B matrizes N x N . Para a multiplicação das matrizes, cada tarefa será 
responsável por multiplicar dois valores, sendo cada um deles proveniente de uma das matrizes 
operando. Por exemplo, para realizar a multiplicação A*B, seriam criadas N^3 tarefas T_{ijk}, 
(i,j,k \in [1,N]), cada uma responsável por multiplicar os valores A[i][j] * B[j][k]. As tarefas 
T_{ijk} relacionadas a um mesmo elemento resultante deverão se comunicar por meio de uma 
redução de forma a calcular o valor acumulado deste elemento R_{AB_{ik}}. Após realizada a 
computação, as tarefas T_{ijk} gerarão N^2 resultados, que deverão ser comunicados à 
tarefa mestre para que a saída do programa seja gerada.

C - Comunicação

Para que a multiplicação de matrizes seja possível, cada tarefa T_{ijk} deverá receber 
como entrada os dois elementos a serem multiplicados provenientes das matrizes de entrada. 
Outra comunicação a ser realizada acontece quando as tarefas relacionadas a um mesmo elemento 
resultante R_{AB_{ik}}  devem acumular os seus resultados. Assim que os seus valores forem 
comunicados à tarefa mestre, a tarefa T_{ijk} deixa de existir. A tarefa mestre recebe estes 
resultados R_{AB_{ik}} e os imprime na tela.

A - Aglomeração

Para reduzir o overhead de comunicação, as tarefas serão aglomeradas de acordo com os 
elementos necessários para que valores C_{ij} sejam calculados.  Na prática, um processo mestre 
será responsável por distribuir, através de um broadcast, as matrizes A e B para P_{d} 
processos em regime de memória distribuída. Então, cada um destes processos escravos será responsável 
por calcular o valor de N^2 / P_{d} valores da matriz C, aglomerando para isto as tarefas que estão 
relacionadas ao cálculo deste valor. Os P_{d} intervalos de tamanho N^2/P_{d} a serem 
calculados serão definidos da seguinte forma: seja C_{v} um vetor proveniente de uma matriz de 
dimensão N x N ao serem concatenadas as suas linhas de forma consecutiva (a linha i+1 é 
concatenada imediatamente ao fim da linha i). O processo P_{d_{i}}, i \in [2, P_{d}], será 
responsável por calcular o i-ésimo intervalo de tamanho N^2/P_{d} deste vetor C_{v}. Por 
exemplo, o processo P_{d_{1}} calculará os itens de C_{v} do intervalo [0, N^2 / P_{d}], 
P_{d_{2}} os do intervalo  [N^2 / P_{d} + 1, 2*N^2 / P_{d}] e assim por diante. No caso de 
sobra dos itens de C_{v}, estes serão distribuídos para os processos seguindo-se uma Round Robin.

Cada processo então realizará outra aglomeração de suas tarefas, porém agora em um ambiente de memória 
compartilhada. Serão definidos P_{c} atividades a serem executadas paralelamente por cada processo, 
sendo cada uma delas responsáveis por executar a computação responsável por um intervalo dos elementos 
de C_{v} recebidos pelo seu processo. Este intervalo será dividido em P_{c} partes, sendo cada 
um processado pela atividade P_{c_{i}}. Ou seja, a tarefa P_{c_{1}} do processo P_{d_{1}} 
será responsável pelo intervalo [0, N^2/(P_{d} * P_{c}], a atividade P_{c_{2}} do mesmo processo 
pelo intervalo [N^2 / (P_{d} * P_{c}, 2*N^2 / (P_{d} * P_{c}] e assim por diante. Para cada elemento 
C_{ij}, a tarefa responsável pelo seu cálculo deverá realizar o produto entre a linha de A e a coluna 
de B relacionada. Ao realizar o cálculo do intervalo recebido, o processo deverá comunicar o seu resultado 
ao processo mestre para que seja realizada a impressão das respostas.

M - Mapeamento

Os P_{d} processos iniciais deverão ser distribuídos à P_{d} nós de um cluster a partir do MPI, 
sendo opcional a passagem de uma lista com os nomes dos P_{d} processadores que devem receber estes 
processos. Já para os P_{c} processos posteriores vale o seguinte: de acordo com Ian Foster 
{FOSTER, Ian. Designing and building parallel programs: concepts and tools for parallel software engineering. 
Addison-Wesley Longman Publishing Co., Inc., 1995.}, o problema do mapeamento não acontece em sistemas que 
possuem memória compartilhada e oferecem escalonamento automático de tarefas. Portanto, nesta atividade, como 
os nós nos quais o programa será executado possuem as características citadas, o problema do mapeamento 
não será abordado durante a implementação do algoritmo. 

*/

# include <omp.h>
# include <mpi.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

// Dimensões das matrizes.
# define N 3 
// Número de threads.
# define NUMTHREADS 8

// Faz a multiplicação
void multiply(int n, double *A, double *B, double *C, int start, int end) {
    
    // Faz a computação para as linhas desse, processo dividindo-as entre os processadores
    // A é uma matriz com dimensões n1 x m1 e B é uma matriz com dimensões n2 x m2
    // C é a matriz de resultado com dimensões n1 x m2
    // É necessário que m1 seja igual a n2 para a computação ter sucesso
    # pragma omp parallel num_threads(NUMTHREADS) shared(n, A, B, C, start, end)
    {

        // Multiplica A*B
        // As linhas são divididas em THREADS grupos com
        // tamanhos smiliares sendo o resto da divisão dividido 
        // igualmente entre eles, isso é feito pelo omp for.
        // A divisão é feita em linhas, pois isso otimiza os
        // acessos a cache e evita falso compartilhamento ao
        // escrever para C.
        # pragma omp for
        for(int pos = start; pos < end; pos++) {
            
            int i = pos / n;
            int j = pos % n;

            // Inicializa C[i][j] com a identidade 
            // da soma, para poder fazer a acumulação.
            C[i * n + j] = 0;

            // Acumula para C a multiplicação
            // dos elementos de uma linha i da matriz A para
            // uma coluna j da matriz B.
            for(int k = 0; k < N; k++) {
                C[i * n + j] += A[i * n + k] * B[k * n + j];
            }

        }

    }

}

int main(int argc, char **argv)  {

    int my_rank, num_procs, provided;
    int tag, src, dst, test_flag, i;
    int buffersize, sizemsgsnd, nrmsgs;

    char *buffersnd;
    char msgsnd[30], msgrcv[30], msgsync[30];

    MPI_Status  status;
    MPI_Request mpirequest_mr, mpirequest_mr_ready;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (my_rank == 0)
    {

        // Calcula o número de elementos na matriz.
        int tam = N * N;

        // Aloca espaço para guardar as matrizes.
        double *A = malloc(tam * sizeof(double));
        double *B = malloc(tam * sizeof(double));
        double *C = malloc(tam * sizeof(double));

        // Preenche as matrizes A e B
        for(int i = 0; i < tam; i++) {
            A[i] = i;
            B[i] = i;
        }


        // Faz o broadcast
        MPI_Bcast(A, tam, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(B, tam, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // Calcula valores necessários para determinar onde esse processo acaba e o próximo termina.
        int num_work_procs = num_procs - 1;
        int r = tam % num_work_procs;

        // Faz o gather
        for(int rank = 1; rank < num_procs; rank++) {

            int work_rank = rank - 1;

            // Encontra o começo e o final para esse processo.
            int start = work_rank * (tam / num_work_procs) + work_rank * (work_rank < r) + r * (work_rank >=r );
            int end = start + (tam / num_work_procs) + 1 * (work_rank < r);

            // Guarda o status do recebimento.
            MPI_Status status;

            // Recebe a entrada
            MPI_Recv(C + start, end - start, MPI_DOUBLE, rank, 0, MPI_COMM_WORLD, &status);

        }

        // Printa a resposta.
        for(int i = 0; i < N; i++) {
            for(int j = 0; j < N; j++) {
                printf("%.1lf ", C[i * N + j]);
            }
            printf("\n");
        }

        // Libera a memória alocada.
        free(A);
        free(B);
        free(C);


    } else {

        // Calcula o número de elementos na matriz.
        int tam = N * N;

        // Calcula valores necessários para determinar onde esse processo acaba e o próximo termina.
        int my_work_rank = my_rank - 1;
        int num_work_procs = num_procs - 1;
        int r = tam % num_work_procs;

        // Encontra o começo e o final para esse processo.
        int start = my_work_rank * (tam / num_work_procs) + my_work_rank * (my_work_rank < r) + r * (my_work_rank >= r);
        int end = start + (tam / num_work_procs) + 1 * (my_work_rank < r);

        //printf("Proc %d: from %d to %d\n", my_rank, start, end);

        // Aloca espaço para guardar as matrizes recebidas
        double *A_recv = malloc(tam * sizeof(double));
        double *B_recv = malloc(tam * sizeof(double));

        // Recebe os dados
        MPI_Bcast(A_recv, tam, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(B_recv, tam, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // Aloca espaço para guardar a matriz com as respostas desse processo.
        double *C = malloc(tam * sizeof(double));

        // Calcula os valores
        multiply(N, A_recv, B_recv, C, start, end);

        // Envia os resultados para o processo de rank 0.
        MPI_Send(C + start, end - start, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

        // Libera a memória alocada.
        free(C);

    }

    MPI_Finalize();

    return 0;

}
