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
    }

    // Número de palavras por processo
    int words_per_proc = W / num_procs;

    // Variáveis usadas para o Scatter e o Gather
    int *counts = NULL;
    int *displs = NULL;

    // Calcula a região que deve ser utilizada por cada processo.
    int start = my_rank * words_per_proc;
    int end;
    if(my_rank == num_procs - 1)
        end = W;
    else
        end = start + words_per_proc;

    // Aloca os vetores necessários para fazer o Scather e o Gather
    if(my_rank == 0) {
        counts = (int*) malloc (num_procs * sizeof (int));
        displs = (int*) malloc (num_procs * sizeof (int));
    }

    // Calcula as posições para o Scatter
    if(my_rank == 0) {       

        for(int rank = 0; rank < num_procs; rank++) {

            int l_start = rank * words_per_proc;
            int l_end;
            if(rank == num_procs - 1)
                l_end = W;
            else
                l_end = l_start + words_per_proc;

            counts[rank] = (l_end - l_start) * (C+1);
            displs[rank] = (l_start) * (C+1);

            //printf("rank %d + st %d + cn %d\n", rank, displs[rank], counts[rank]);
        }
    }

    char *PalavrasProc = (char*) malloc((end-start) * (C+1) * sizeof(char));

    // Faz o Scatter dos conteúdos da matriz
    MPI_Scatterv(VetPalavras, counts, displs, MPI_CHAR, PalavrasProc, (end - start) * (C+1), MPI_CHAR, 0, MPI_COMM_WORLD);   

    /*for(int i = 0; i < end - start; i++) {
        printf("(%d) %s\n", my_rank, PalavrasProc + (i * (C+1)));
        fflush(0);
    }*/

    if(my_rank == 0) {
        // fecha o arquivo de entrada
        fclose(inputfile);
    }

    //printf("(%d) hello (%d to %d)\n", my_rank, start, end);

    // Guardam os resultados para cada processo.
    int *Coluna = (int*) malloc ((end - start) * sizeof (int));
    int *LinhaMatriz = (int*) malloc ((end - start) * sizeof (int));

    // Obtém o maior número de threads disponível e executa com eles
    int max_num_threads = omp_get_max_threads();

    //printf("(%d max: %dt", my_rank, max_num_threads);

    // Divide as palavras entre os threads usando o método padrão do OpenMP
    int lin, col;
    # pragma omp parallel for num_threads(max_num_threads) shared (start, end, PalavrasProc, Matriz, Coluna, LinhaMatriz, L, C, W)
    for(int linha = 0; linha < end - start; linha++)
    {

        //aloca variavel auxiliar Frase, para receber string de uma linha de Matriz
        char *Frase = (char*)malloc((C+1)*sizeof(char));

        //aloca variavel auxiliar Palavra, a ser procurada na Matriz
        char *Palavra = (char*)malloc((C+1)*sizeof(char));

        strcpy(Palavra, PalavrasProc + (linha * (C+1)));
        int TamPalavra = strlen(Palavra);
        
        for(lin = 0; lin < L; lin++)
        {
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

    // Vetores para receber o Gather
    int *Coluna_Root = NULL;
    int *LinhaMatriz_Root = NULL;
    
    // Aloca os vetores para as respsotas
    if(my_rank == 0) {
        Coluna_Root = (int*) malloc (W * sizeof (int));
        LinhaMatriz_Root = (int*) malloc (W * sizeof (int));
    }

    // Calcula as posiçõe spara o Gather
    if(my_rank == 0) {       

        for(int rank = 0; rank < num_procs; rank++) {

            int l_start = rank * words_per_proc;
            int l_end;
            if(rank == num_procs - 1)
                l_end = W;
            else
                l_end = l_start + words_per_proc;

            counts[rank] = l_end - l_start;
            displs[rank] = l_start;

            //printf("rank %d + st %d + cn %d\n", rank, displs[rank], counts[rank]);
        }
    }

    // Faz o Gather
    MPI_Gatherv(Coluna, end - start, MPI_INT, Coluna_Root, counts, displs, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gatherv(LinhaMatriz, end - start, MPI_INT, LinhaMatriz_Root, counts, displs, MPI_INT, 0, MPI_COMM_WORLD);
   
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
        free(counts);
        free(displs);
    }
    
    // Finaliza o MPI
    MPI_Finalize();

    return(0);
    
}
