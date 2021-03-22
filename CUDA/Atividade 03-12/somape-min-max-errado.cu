
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
#include <cuda.h>

__global__ void sum_prod_scalar(int *a, int *b, int dim,int *result){
    __shared__ int row_sum;
    row_sum = 0;
    
    __syncthreads();

    int id = blockDim.x * blockIdx.x + threadIdx.x;
    
    atomicAdd(&row_sum, a[id] * b[id]);
    
    __syncthreads();

    if(id == blockDim.x * blockIdx.x){
        atomicAdd(result, row_sum);
    }

}

// <<LENGTH, WIDTH>>
__global__ void find_min(int *A, int *B, int dim, int *result){
    __shared__ int row_min;
    row_min = 1000000;
    __syncthreads();

    int id = blockIdx.x * blockDim.x + threadIdx.x;

    if(id < dim){
        if(A[id] < row_min)
            row_min = A[id];
        if(B[id] < row_min)
            row_min = A[id];
    }  

    __syncthreads();

    // se for primeira thread do bloco
    if(id == blockDim.x * blockIdx.x){
        if(row_min < *result) 
            *result = row_min; 
    }


}   

// <<LENGTH, WIDTH>>
__global__ void find_max(int *A, int *B, int dim, int *result){
    __shared__ int row_max;
    row_max = -1000000;
    
    __syncthreads();

    int id = blockIdx.x * blockDim.x + threadIdx.x;
 
    if(id < dim){
        if(A[id] > row_max)
            row_max = A[id];
        if(B[id] > row_max)
            row_max = A[id];
    }  

    __syncthreads();

    // se for primeira thread do bloco
    if(id == blockDim.x * blockIdx.x){
        if(row_max < *result) 
            *result = row_max; 
    }
}

int main(int argc, char **argv) {

    // Declara a dimensão da matriz
    int dim;

    // Declara as matrizes
    int *A,*B, *A_device_s1, *B_device_s1;

    // Declara o acumulador para o produto escalar global o maximo e o minimo
    int somape, minimo = 10000, maximo = -10000;

    // Declara um vetor para os produtos escalares locais
    int *prod_escalar;
    
    // Lê a dimensão das matrizes
    scanf(" %d", &dim);
    
    // Cria as streams
    cudaStream_t stream1_soma, stream2_min, stream3_max;  
    cudaStreamCreate(&stream1_soma);    // Stream que realiza a soma
    cudaStreamCreate(&stream2_min);     // Stream que realiza o minímo
    cudaStreamCreate(&stream3_max);     // Stream que realiza o máximo

    // Número de elementos da matriz.
    int tam = dim * dim;

    // Aloca as matrizes no host 
    cudaMallocHost((void**)&A, tam * (sizeof(int))); 
    cudaMallocHost((void**)&B, tam * (sizeof(int)));

    // Aloca as matrizes no device para cada stream
    cudaMalloc((void**)&A_device_s1, tam * (sizeof(int))); 
    cudaMalloc((void**)&B_device_s1, tam * (sizeof(int)));

    // Aloca um vetor para armazenar os produtos escalares de cada linha
    cudaMalloc((void**)&prod_escalar, dim * (sizeof(int)));

    // Lê a matriz A
    for(int i = 0; i < dim; i++) {
        for(int j = 0; j < dim; j++) {
            scanf(" %d",&(A[i*dim+j]));
        }
    }

    // Lê a matriz B
    for(int i = 0; i < dim; i++) {
        for(int j = 0; j < dim; j++) {
            scanf(" %d",&(B[i*dim+j])); 
        }
    }
    
    // Copia a memória para cada stream do device
    cudaMemcpyAsync(A_device_s1, A, tam * (sizeof(int)), cudaMemcpyHostToDevice);
    cudaMemcpyAsync(B_device_s1, B, tam * (sizeof(int)), cudaMemcpyHostToDevice);

    sum_prod_scalar <<<dim, dim, 0, stream1_soma>>> (A_device_s1, B_device_s1, tam, &somape);
    find_min <<<dim, dim, 0, stream2_min>>> (A_device_s1, B_device_s1, tam, &minimo);
    find_max <<<dim, dim, 0, stream3_max>>> (A_device_s1, B_device_s1, tam, &maximo);
    
    // Sincroniza as streams
    cudaStreamSynchronize(stream1_soma);
    cudaStreamSynchronize(stream2_min);
    cudaStreamSynchronize(stream3_max);

    // Imprime o resultado
    printf("%d %d %d\n",somape, minimo, maximo); 
    
    // Desaloca as matrizes
    cudaFreeHost(A);
    cudaFreeHost(B);
    cudaFree(A_device_s1);
    cudaFree(B_device_s1);
    cudaFree(prod_escalar);
    
    // Destroi as streams
    cudaStreamDestroy(stream1_soma);
    cudaStreamDestroy(stream2_min);
    cudaStreamDestroy(stream3_max);

}
