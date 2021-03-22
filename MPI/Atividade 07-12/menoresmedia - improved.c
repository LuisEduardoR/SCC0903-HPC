
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>
#include <mpi.h>

# define THREADS 4

// --------------------------------------------------------------------------------------------
// Funções Úteis ------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

// Função para calcular qual parte dos dados um determinado processo deve receber/executar
void calc_proc_range(int my_rank,       // rank do processo atual
                     int num_procs,     // número total de processos
                     int count,         // número total de elementos
                     int *out_start,    // o inicio será escrito aqui
                     int *out_end) {    // o final será escrito aqui

    // Cálcula o número de elementos para cada processo
    int count_per_proc  = count / num_procs;

    // Cálcula o ínicio desse processo
    (*out_start) = my_rank * count_per_proc;
    // Cálcula o final desse processo
    (*out_end) = (my_rank == num_procs - 1) ? count : ((*out_start) + count_per_proc);

}

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

// Wraper para calcular o que deve ser enbiado a cada processo e usar o MPI_Scatterv da maneira correta
void do_scatterv(void *send_buf,         // buffer que será enviado
                int send_count,         // número de elementos a serem enviados ao todo
                int send_elem_size,     // tamanho dos elementos a serem enviados (usado para arrays e strings, se não deixe como 1)
                MPI_Datatype datatype,  // tipo de dados para ser usado no MPI
                int datatype_size,      // tamanho do tipo de dados que está sendo enviado
                int root,               // rank do processo que está enviando
                int my_rank,            // rank do processo atual
                int num_procs,          // número total de processos
                MPI_Comm comm,          // grupo de comunicação do MPI
                void **out_recv_buf,    // buffer que receberá os dados, será realocado para o tamanho necessário
                int *out_recv_count) {  // número de elementos que foram colocados em out_send_buf

    // Parâmetros para o MPI_Scatterv
    int *sendcounts = NULL;
    int *displs = NULL;

    // Cálcula o número de elementos enviados por processo
    int send_count_per_proc = send_count / num_procs;

    int proc_start = -1;
    int proc_end = -1;

    // Faz os calculos e preparos necessários para o
    // MPI_Scatterv no root
    if(my_rank == root) {       

        // Aloca a memória necessária para as arrays de parâmetros
        sendcounts = (int*)malloc(num_procs * sizeof(int));
        displs = (int*)malloc(num_procs * sizeof(int));

        // Preenche sendcounts e displs
        for(int rank = 0; rank < num_procs; rank++) {
            calc_proc_range(rank, num_procs, send_count, &proc_start, &proc_end);
            sendcounts[rank] = (proc_end - proc_start) * send_elem_size;
            displs[rank] = (proc_start) * send_elem_size;
        }
    }

    // Cálcula o início e o final da região para o processo atual
    calc_proc_range(my_rank, num_procs, send_count, &proc_start, &proc_end);

    // Salva o número de elementos que será colocado em out_recv_buf
    *out_recv_count = (proc_end - proc_start);

    // Aloca o espaço necessário em out_recv_buf
    *out_recv_buf = realloc(*out_recv_buf, (*out_recv_count) * send_elem_size * datatype_size);

    // Faz o Scatterv pelo MPI
    MPI_Scatterv(send_buf, 
                sendcounts, 
                displs, 
                datatype, 
                *out_recv_buf, 
                (*out_recv_count) * send_elem_size, 
                datatype, 
                root, 
                comm);

    // Libera a memória usada pelo root
    if(my_rank == root) { 
        free(sendcounts);
        free(displs);
    }

}

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

// Wraper para calcular o que cada processo deve receber e usar o MPI_Gatherv da maneira correta
void do_gatherv( void *send_buf,         // buffer que será enviado pelo processo atual
                int send_count,         // número de elementos a serem enviados pelo processo atual
                int send_elem_size,     // tamanho dos elementos a serem enviados (usado para arrays e strings, se não deixe como 1)
                MPI_Datatype datatype,  // tipo de dados para ser usado no MPI
                int datatype_size,      // tamanho do tipo de dados que está sendo recebido
                int root,               // rank do processo que está recebendo
                int my_rank,            // rank do processo atual
                int num_procs,          // número total de processos
                MPI_Comm comm,          // grupo de comunicação do MPI
                void **out_recv_buf,    // buffer que receberá os dados, será realocado para o tamanho necessário
                int recv_count) {       // número de elementos que seram colocados em out_recv_buf

    // Parâmetros para o MPI_Gatherv
    int *recvcounts = NULL;
    int *displs = NULL;

    // Cálcula o número de elementos enviados por processo
    int send_count_per_proc = recv_count / num_procs;

    int proc_start = -1;
    int proc_end = -1;

    // Faz os calculos e preparos necessários para o
    // MPI_Gatherv no root
    if(my_rank == root) {       

        // Aloca a memória necessária para as arrays de parâmetros
        recvcounts = (int*)malloc(num_procs * sizeof(int));
        displs = (int*)malloc(num_procs * sizeof(int));

        // Preenche recvcounts e displs
        for(int rank = 0; rank < num_procs; rank++) {
            calc_proc_range(rank, num_procs, recv_count, &proc_start, &proc_end);
            recvcounts[rank] = (proc_end - proc_start) * send_elem_size;
            displs[rank] = (proc_start) * send_elem_size;
        }
    }

    // Cálcula o início e o final da região para o processo atual
    calc_proc_range(my_rank, num_procs, recv_count, &proc_start, &proc_end);

    // Aloca o espaço necessário em out_recv_buf
    if(my_rank == root)
        *out_recv_buf = realloc(*out_recv_buf, recv_count * send_elem_size * datatype_size);

    // Faz o Scatterv pelo MPI
    MPI_Gatherv(send_buf, 
                send_count, 
                datatype,
                *out_recv_buf,  
                recvcounts, 
                displs, 
                datatype, 
                root, 
                comm);

    // Libera a memória usada pelo root
    if(my_rank == root) { 
        free(recvcounts);
        free(displs);
    }

}

// --------------------------------------------------------------------------------------------
// Fim de Funções Úteis -----------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

int main(int argc,char **argv)
{

    // Declara as variáveis do MPI
    int my_rank, num_procs, provided;

    // Guarda a dimensão da matriz
    int dim;

    // Guarda o valor da média
    int media;

    // Declara a matriz MAT e o vetor vet_menor
    int *MAT = NULL;
    int *Proc_Lines = NULL;
    int *vet_menor = NULL;
    int *sums = NULL;

    // Usados para guardar o inicio e o final das áreas de cada processo
    int start, end;

    // Guardam os tamanhos das regiões que seram processadas por cada processo.
    int elems_per_proc, coluns_per_proc;

    // Guarda a soma dos elementos do processo local.
    int sum_local;

    // Guarda o número de elemenos menores que a média para cada coluna do processo local.
    int *menor_local;

    // Informações usadas para o Gatherv
    int *recvcounts, *displs;

    FILE *inputfile;     // handler para o arquivo de entrada
    char *inputfilename;  // nome do arquivo de entrada

    // Inicializa o MPI
    MPI_Status  status;
    MPI_Request mpirequest_mr, mpirequest_mr_ready;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // Processo de rank 0, recebe a entrada, faz a comunicação entre os outros processos e printa a saída.
    if (my_rank == 0) {      

        // Abre o arquivo e recebe a entrada.
        if (argc < 2)
        {
            printf("Nr errado de argumentos. Execute passando <arq_entrada> como argumento. \n");
            exit(-1);
        }

        inputfilename = (char*) malloc (256*sizeof(char));
        strcpy(inputfilename,argv[1]);

        if ((inputfile=fopen(inputfilename,"r")) == 0)
        {
            printf("Mestre: Erro ao abrir arquivo de entrada %s. Saindo. \n", inputfilename);
            exit(-1);
        }
        
        fscanf(inputfile, "%d\n", &dim); //Lê a dimensão de MAT
    }

    // Faz o broadcast da dimensão.
    MPI_Bcast(&dim, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Aloca a matriz.

    // Lê a matriz TRANSPOSTA para MAT
    if(my_rank == 0) {
        MAT = (int *)malloc(dim * dim * sizeof(int));
        for(int i = 0; i < dim; i++) {
            for(int j = 0; j < dim; j++) {
                fscanf(inputfile, "%d ", &(MAT[j * dim + i]));
            }
        }
        // Fecha o arquivo de entrada
        fclose(inputfile);
    }
    
    // Guarda o numéro de linhas (colunas transpostas) desse processo
    int Lines_count = -1;

    // Faz o Scatter da matriz para os processos
    do_scatterv(MAT, dim, dim, MPI_INT, sizeof(int), 0, my_rank, num_procs, MPI_COMM_WORLD, (void**)(&Proc_Lines), &Lines_count);

    // Faz o Broadcast da matriz para todos os processos
    //MPI_Bcast(MAT, dim*dim, MPI_INT, 0, MPI_COMM_WORLD);

    // printf("(%d) elems_per_proc=%d\n start=%d end=%d\n", my_rank, elems_per_proc, start, end);

    // Faz o cálculo da soma em paralelo para esse processo
    sum_local = 0;
    # pragma omp parallel for num_threads(THREADS) shared(Proc_Lines, Lines_count) reduction(+:sum_local)
    for(int i = 0; i < dim * Lines_count; i++){
        sum_local += Proc_Lines[i];
    }

    //printf("(%d) local sum = %d\n", my_rank, sum_local);

    // Faz o gather
    do_gatherv(&sum_local, 1, 1, MPI_INT, sizeof(int), 0, my_rank, num_procs, MPI_COMM_WORLD, (void**)(&sums), num_procs);  

    /*
    if(my_rank == 0) {
        for(int i = 0; i < num_procs; i++) {
            printf("(%d) sum = %d\n", i, sums[i]);
        }
    }
    */

    // Calcula a média
    media = 0;
    if(my_rank == 0) {
        for(int i = 0; i < num_procs; i++) {
            media += sums[i];
        }
        media /= (dim*dim);
    }

    // Faz o Broadcast da média para todos os processos
    MPI_Bcast(&media, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /*
    if(my_rank == 0) {
        printf("m: %d\n", media);
        fflush(0);
    }
    */

    // Aloca o vetor de menor local e inicializa.
    menor_local = (int *)malloc(Lines_count * sizeof(int));
    for(int i = 0; i < Lines_count; i++)
        menor_local[i] = 0;

    // Verifica quais elementos são menores em cada coluna.
    # pragma omp parallel for num_threads(THREADS) shared(Proc_Lines, Lines_count, menor_local, media)
    for(int i = 0; i < Lines_count; i++) {

        // Conta os elementos menores que a média
        int num = 0;
        for(int j = 0; j < dim; j++) {
            if(Proc_Lines[i*dim+j] < media)
                num++;
        }

        // printf("(%d) coluna %d valor %d\n", my_rank, i, num);

        // Escreve os resultados no vetor.
        menor_local[i] = num;

    }

    // Faz o gather
    do_gatherv(menor_local, Lines_count, 1, MPI_INT, sizeof(int), 0, my_rank, num_procs, MPI_COMM_WORLD, (void**)(&vet_menor), dim);

    // Printa o resultado.
    if(my_rank == 0) {
        for(int i = 0; i < dim; i++) {
            printf("%d ", vet_menor[i]);
        }
        printf("\n");
    }

    // Libera memória
    free(menor_local);
    free(Proc_Lines);
    if(my_rank == 0) {
        free(MAT);
        free(recvcounts);
        free(displs);
        free(sums);
        free(vet_menor);
    }

    MPI_Finalize();
	
	return(0);
}
