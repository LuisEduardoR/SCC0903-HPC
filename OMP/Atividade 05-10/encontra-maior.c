/*
  Universidade de Sao Paulo - ICMC
  SSC0903 - Computacao de Alto Desempenho
  Atividade 1
  Grupo:
  Joao Pedro A. S. Secundino (10692054);
  Joao Pedro Uchoa Cavalcante (10801169);
  Luis Eduardo Rozante de Freitas Pereira (10734794);
  Sergio Ricardo G. B. Filho (10408386);
*/

#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

// definindo em quantas threads as tarefas serao aglomeradas
#define T 8

int main(int argc,char **argv){
    double wtime;
    int *v, i, r, maior = -1, localmaior, v_size, n_threads = T;


    if ( argc  != 2)
    {
        printf("Wrong arguments. Please use main <amount_of_elements>\n");
        exit(0);
    }

    // alocando vetor com tamanho informado por argumento
    v_size = atoi(argv[1]);
    r = v_size%n_threads; //resto da divisão que será utilzado na determinacao dos intervalos de cada thread
    v = (int *) malloc(sizeof(int) * v_size);

    printf("Amount of vetor=%d\n", v_size);
    fflush(0);

    omp_lock_t mylock;
    omp_init_lock(&mylock);
    wtime = omp_get_wtime ();

    #pragma omp parallel num_threads(n_threads) shared(v, v_size, r, maior, n_threads) private(localmaior)
    {
        
        // determinando o intervalo de tarefas para a thread atual
        int id = omp_get_thread_num();
        int start = id * (v_size / n_threads) + id * (id < r) + r * (id >=r );
        int end = start + (v_size / n_threads) + 1 * (id < r);
        
        // preenchendo vetor com v_size na posicao do meio e 1 nas demais posições
        for (int i = start; i < end; i++)
        {
            v[i] = (i == v_size/2 ? v_size : 1 );
        }

        // sincronizando antes de iniciar busca pelo maior local
        
        #pragma omp barrier
        
        // buscando pelo maior local no intervalo
        localmaior = -1;
        for(int i = start; i < end; i++)
        {
            if(v[i] > localmaior){
                localmaior = v[i];
            }
        }

        // definindo regiao critica para atualizacao do maior global
        
        omp_set_lock(&mylock);
        if (localmaior > maior){
            maior = localmaior;
        }
        omp_unset_lock(&mylock);
    }

   /*
    *************************************************************************************
    Não modifique este trecho de código
    */
    wtime = omp_get_wtime() - wtime;

    printf("OMP SHD LOCK: Tam=%d, Num_Threads=%d, maior=%d, Elapsed wall clock time = %f  \n", v_size, n_threads, maior, wtime); //Imprime o vetor ordenado
    free(v); //Desaloca o vetor lido

    return 0;
} // fim da main
