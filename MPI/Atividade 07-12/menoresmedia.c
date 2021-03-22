
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

int main(int argc,char **argv)
{

    // Declara as variáveis do MPI
    int my_rank, num_procs, provided;

    // Guarda a dimensão da matriz
    int dim;

    // Guarda o valor da média
    int media;

    // Declara a matriz MAT e o vetor vet_menor
    int *MAT, *vet_menor, *sums;

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
    MAT = (int *)malloc(dim * dim * sizeof(int));

    // Lê a matriz TRANSPOSTA para MAT
    if(my_rank == 0) {
        for(int i = 0; i < dim; i++) {
            for(int j = 0; j < dim; j++) {
                fscanf(inputfile, "%d ", &(MAT[j * dim + i]));
            }
        }
        // Fecha o arquivo de entrada
        fclose(inputfile);
    }

    // Faz o Broadcast da matriz para todos os processos
    MPI_Bcast(MAT, dim*dim, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Pega a região a ser processada para esse processo
    elems_per_proc = dim * dim / (num_procs);
    start = my_rank * elems_per_proc;
    end;
    if(my_rank == num_procs - 1)
        end = dim * dim;
    else
        end = start + elems_per_proc;

    // printf("(%d) elems_per_proc=%d\n start=%d end=%d\n", my_rank, elems_per_proc, start, end);

    // Faz o cálculo da soma em paralelo para esse processo
    sum_local = 0;
    # pragma omp parallel for num_threads(THREADS) shared(MAT, start, end) reduction(+:sum_local)
    for(int i = start; i < end; i++){
        sum_local += MAT[i];
    }

    //printf("(%d) local sum = %d\n", my_rank, sum_local);

    // Preenche as informações para o Gatherv
    if(my_rank == 0) {

        // Vetor que guardará as somas
        sums = (int *)malloc(num_procs * sizeof(int));
        recvcounts = (int *)malloc(num_procs * sizeof(int));
        displs = (int *)malloc(num_procs * sizeof(int));

        for(int i = 0; i < num_procs; i++) {
            recvcounts[i] = 1;
            displs[i] = i;
        }
    }

    // Faz o Gather.
    MPI_Gatherv(&sum_local, 1, MPI_INT, sums, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

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

        vet_menor = (int *)malloc(dim * sizeof(int));

        for(int i = 0; i < num_procs; i++) {
            media += sums[i];
        }
        media /= (dim*dim);
    }

    // Faz o Broadcast da matriz para todos os processos
    MPI_Bcast(&media, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /*if(my_rank == 0) {
        printf("m: %d\n", media);
    }*/

    // Obtém a coluna inicial e final desse processo.
    coluns_per_proc = dim / num_procs;
    start = my_rank * coluns_per_proc;
    end;
    if(my_rank == num_procs - 1)
        end = dim;
    else
        end = start + coluns_per_proc;

    // ALoca o vetor de menor local e inicializa.
    menor_local = (int *)malloc((end - start) * sizeof(int));
    for(int i = 0; i < end - start; i++)
        menor_local[i] = 0;

    // Verifica quais elementos são menores em cada coluna.
    # pragma omp parallel for num_threads(THREADS) shared(MAT, vet_menor, media, start, end)
    for(int i = start; i < end; i++) {

        // Conta os elementos menores que a média
        int num = 0;
        for(int j = 0; j < dim; j++) {
            if(MAT[i*dim+j] < media)
                num++;
        }

        // printf("(%d) coluna %d valor %d\n", my_rank, i, num);

        // Escreve os resultados no vetor.
        menor_local[i - start] = num;

    }

    // Preenche as informações para o Gatherv
    if(my_rank == 0) {
        for(int rank = 0; rank < num_procs; rank++) {

            int l_start = rank * coluns_per_proc;
            int l_end;
            if(rank == num_procs - 1)
                l_end = dim;
            else
                l_end = l_start + coluns_per_proc;

            recvcounts[rank] = l_end - l_start;
            displs[rank] = l_start;

            //printf("rank %d + st %d + cn %d\n", rank, displs[rank], recvcounts[rank]);
        }
    }

    // Faz o Gather.
    MPI_Gatherv(menor_local, (end - start), MPI_INT, vet_menor, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    // Printa o resultado.
    if(my_rank == 0) {
        for(int i = 0; i < dim; i++) {
            printf("%d ", vet_menor[i]);
        }
        printf("\n");
    }

    // Libera memória
    free(MAT);
    free(menor_local);
    if(my_rank == 0) {
        free(recvcounts);
        free(displs);
        free(sums);
        free(vet_menor);
    }

    MPI_Finalize();
	
	return(0);
}
