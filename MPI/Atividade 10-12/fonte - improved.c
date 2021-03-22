
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

/*
Dada uma matriz de dimensoes LxC, com digitos decimais (0-9),
procurar por palavras nas linhas da matriz. As palavras a serem 
procuradas tem tamanhos variaveis nao maiores que C caracteres validos 
(sem contar o '\0') e assume-se que elas aparecem apenas uma vez na matriz,
i.e., nao se repetem.
No final, a aplicacao mostra como resultado, para cada palavra, 
a sua localizacao inicial na matriz em termos de linha e coluna, 
(ambas iniciando em zero). Se a palavra nao for encontrada na 
matriz, os endereços de linha e coluna no arquivo de saida devem ser ambos -1.

O arquivo de entrada contem em cada linha:
- valores de L e C para a matriz de caracteres;
- matriz de caracteres com digitos decimais (0-9). Cada linha da matriz 
  esta em uma linha do arquivo de entrada.
- quantidade de W palavras a serem buscadas na matriz.
- palavras a serem buscadas na matriz, sendo, novamente, uma palavra por linha.

O arquivo de saida contem uma linha para cada palavra a ser procurada na matriz, 
com os seguintes resultados: <palavra>,<linha>,<coluna>  (sem espaços entre eles e 
com quebra de linha no final de cada linha) 

Exemplo de arquivo de entrada:
10           << ordem da matriz
0000012345   << digitos da matriz
9876543111
5122222222
3333753193
4445494444
0695555555
6666826666
7777777777
2929292988
1818181818	<< digitos da matriz
12			<< nr de palavras a buscar
12345       << palavra a buscar
9876543		<< palavra a buscar
512
75319		<< palavra a buscar
549
69
826
0101		<< palavra a buscar. Esta não existe.
29292929
1818181818	<< palavra a buscar
3416		<< palavra a buscar
4444

Exemplo de arquivo de saida:
12345,0,5       << palavra a buscar
9876543,1,0		<< palavra a buscar
512,2,0
75319,3,4		<< palavra a buscar
549,4,3
69,5,1
826,6,4
0101,-1,-1		<< palavra a buscar. Esta não existe.
29292929,8,0
1818181818,9,0	<< palavra a buscar
3416,-1,-1		<< palavra a buscar
4444,4,6		<< palavra a buscar

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <mpi.h>

// Based on Author: Paulo S L de Souza

int encontra(char *Frase, char *Palavra, int C, int TamPalavra) {

      int DeslocaPalavra = 0;  // percorre caracteres da palavra de busca (Palavra)
      int DeslocaMatriz = 0;   // percorre caracteres da linha da matriz (Frase)
      int ColIni = -1;         // guarda a posicao inicial da Palavra na Frase (coluna onde inicia)
      
      // enquanto ha deslocamentos na linha da matriz a fazer para buscar a palavra de TamPalavra 
      // E
      // enquanto a palavra a ser buscada ainda nao foi percorrida por completo
//      printf("Frase=%s e Palavra=%s\n", Frase, Palavra);
//      fflush(0);
      
      while( (((C+1) - DeslocaMatriz) >= (TamPalavra - DeslocaPalavra))  &&  (DeslocaPalavra < TamPalavra) )
      {
            if(Frase[DeslocaMatriz] == Palavra[DeslocaPalavra]) // se caracteres sao iguais
            {
                if(DeslocaPalavra == 0) // eh o primeiro a ser igual?
                {
                    ColIni = DeslocaMatriz; // marca o inicio da palavra na Matriz. Vai que serah igual!
                }
                DeslocaPalavra++;  // ainda é igual, entao desloca caracter na palavra de busca
            }
            else
            {
                DeslocaPalavra=0; // volta ao comeco da palavra de busca, pois nao eh igual
                ColIni=-1;
            }
            DeslocaMatriz++;  // desloca o caracter na Matriz
/*         // para depuracao apenas 
           printf("ColIni=%d, DeslocaPalavra=%d, DeslocaMatriz=%d, TamPalavra=%d, C+1=%d, %c==%c?.\n ", ColIni, DeslocaPalavra, DeslocaMatriz, TamPalavra, C+1, Frase[DeslocaMatriz], Palavra[DeslocaPalavra]);
            fflush(0);
*/
      } // fim do while

/*  
    // para depuracao apenas
    printf("ColIni=%d, DeslocaPalavra=%d, DeslocaMatriz=%d, TamPalavra=%d.\n ", ColIni, DeslocaPalavra, DeslocaMatriz, TamPalavra);
	fflush(0);
*/

      if(DeslocaPalavra != TamPalavra)
      {
            ColIni = -1;
      }

// 	ColIni tem a posicao inicial da palavra na frase ou -1 para indicar que nao encontrou
//	printf("Fim da funcao Encontra(): String %s na coluna %d.\n", Palavra, ColIni);
 
      return(ColIni);
      
} // fim encontra

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

// Wraper para calcular o que deve ser enviado a cada processo e usar o MPI_Scatterv da maneira correta
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

int main(int argc, char **argv)
{
    
    FILE *inputfile = NULL;     // handler para o arquivo de entrada
    char inputfilename[256];

    // Declara as variáveis do MPI
    int my_rank = -1;
    int num_procs = -1;
    int provided = -1;

    // Inicializa o MPI
    MPI_Status status;
    MPI_Request mpirequest_mr, mpirequest_mr_ready;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    // Tamanho da matriz
    int L = -1;
    int C = -1;

    if(my_rank == 0) {
    
        if (argc != 2) {
            printf("Nr errado de argumentos. Execute passando <arq_entrada> como argumento.\n");
            return(0);
        }

        strcpy(inputfilename,argv[1]);
        inputfile = fopen(inputfilename, "r");
        if(!inputfile) {
            printf("Mestre: Erro ao abrir arquivo de entrada %s. Saindo. \n", argv[1]);
            fflush(0);
            return(0);
        }
    
        // Leitura dos dados de entrada:
        // carrega nr de linhas e colunas da matriz de caracteres em Ordem
        fscanf(inputfile, "%d %d\n", &L, &C);

    }

    // Faz o broadcast das dimensões da matriz.
    MPI_Bcast(&L, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&C, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //printf("(%d) L:%d C:%d\n", my_rank, L, C);

    // Aloca um vetor de tamanho para guardar as linhas uma depois das outras, sendo usado como matriz
    char *Matriz = (char*) malloc (L * (C+1) * sizeof (char));

    //Leitura da matriz de caracteres em Matriz.
    //Leitura ocorre por linha, ate encontrar o final da linha ou espaco em branco/tab/...
    if(my_rank == 0) {        
    
        //Leitura da matriz de caracteres em Matriz.
        //Leitura ocorre por linha, ate encontrar o final da linha ou espaco em branco/tab/...
        for(int linha = 0; linha < L; linha++) {
            fscanf(inputfile, "%s", Matriz + (linha * (C+1)));    
        }

    }

    // Faz o broadcast dos conteúdos da matriz
    MPI_Bcast(Matriz, L * (C+1), MPI_CHAR, 0, MPI_COMM_WORLD);   

    // Quantidade de palavras a serem buscadas
    int W;

    // Leitura da quantidade de palavras a buscar na matriz
    // carrega qtde de palavras a serem buscadas em QtdePalavras
    if(my_rank == 0) {
        fscanf(inputfile, "%d", &W);
    }

    // Faz o broadcast da quantidade de palavras
    MPI_Bcast(&W, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //printf("(%d) W:%d\n", my_rank, W);

    // Vetor de tamanho W de para guardar as palavras uma na frente da outro
    char *VetPalavras = NULL;
    
    // Leitura das palavras a serem procuradas em VetPalavras
    if(my_rank == 0) {
        // Aloca um vetor para guardar as palavras a serem buscadas uma depois das outras, sendo usado como matriz
        VetPalavras = (char*) malloc (W * (C+1) * sizeof (char));
        for(int i = 0; i < W; i++) {
            fscanf(inputfile, "%s", VetPalavras + (i * (C+1)));
        }
        // fecha o arquivo de entrada
        fclose(inputfile);
    }

    // --------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------

    // Guardam os valores que esse processo deve usar depois do scatter
    char *PalavrasProc = NULL;
    int PalavrasProc_count = -1;

    // Faz o Scatter das palavras a serem buscadas
    do_scatterv(VetPalavras, W, C+1, MPI_CHAR, sizeof(char), 0, my_rank, num_procs, MPI_COMM_WORLD, (void**)(&PalavrasProc), &PalavrasProc_count);   

    /*for(int i = 0; i < end - start; i++) {
        printf("(%d) %s\n", my_rank, PalavrasProc + (i * (C+1)));
        fflush(0);
    }*/

    // --------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------

    // Guardam os resultados para cada processo.
    int *Coluna = (int*) malloc (PalavrasProc_count * sizeof (int));
    int *LinhaMatriz = (int*) malloc (PalavrasProc_count * sizeof (int));

    // Obtém o maior número de threads disponível e executa com eles
    int max_num_threads = omp_get_max_threads();

    //printf("(%d max: %dt", my_rank, max_num_threads);

    // Divide as palavras entre os threads usando o método padrão do OpenMP
    int lin, col;
    # pragma omp parallel for num_threads(max_num_threads) shared (PalavrasProc_count, PalavrasProc, Matriz, Coluna, LinhaMatriz, L, C, W)
    for(int linha = 0; linha < PalavrasProc_count; linha++)
    {

        //aloca variavel auxiliar Frase, para receber string de uma linha de Matriz
        char *Frase = (char*)malloc((C+1)*sizeof(char));

        //aloca variavel auxiliar Palavra, a ser procurada na Matriz
        char *Palavra = (char*)malloc((C+1)*sizeof(char));

        strcpy(Palavra, PalavrasProc + (linha * (C+1)));
        int TamPalavra = strlen(Palavra);
        
        for(lin = 0; lin < L; lin++) {
            strcpy(Frase, Matriz + (lin * (C+1)));
            col = encontra(Frase, Palavra, C, TamPalavra);
            if(col != -1)
                break;
        }

        // se a coluna for -1 não encontrou a string. Coloca a linha como -1 tambem.
        if(col == -1)
            lin = -1;

        Coluna[linha] = col;
        LinhaMatriz[linha] = lin;

        //printf("%s,%d,%d (rank: %d word: %d)\n", Palavra, LinhaMatriz[linha - start], Coluna[linha - start], my_rank, linha - start);

        free(Frase);
        free(Palavra);

    }

    // --------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------

    // Vetores para receber o Gather
    int *Coluna_Root = NULL;
    int *LinhaMatriz_Root = NULL;

    // Faz o Gather dos resultados
    do_gatherv(Coluna, PalavrasProc_count, 1, MPI_INT, sizeof(int), 0, my_rank, num_procs, MPI_COMM_WORLD, (void**)(&Coluna_Root), W);  
    do_gatherv(LinhaMatriz, PalavrasProc_count, 1, MPI_INT, sizeof(int), 0, my_rank, num_procs, MPI_COMM_WORLD, (void**)(&LinhaMatriz_Root), W);

    // --------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------  
   
    // Faz a impressão
    if(my_rank == 0) {
        for(int i = 0; i < W; i++) {
            printf("%s,%d,%d\n", VetPalavras + (i * (C+1)), LinhaMatriz_Root[i], Coluna_Root[i]);
        }
    }

    // Libera memória
    free(Matriz);
    free(PalavrasProc);
    free(Coluna);
    free(LinhaMatriz);
    if(my_rank == 0) {
        free(VetPalavras);
        free(Coluna_Root);
        free(LinhaMatriz_Root);
    }
    
    // Finaliza o MPI
    MPI_Finalize();

    return(0);
    
}
