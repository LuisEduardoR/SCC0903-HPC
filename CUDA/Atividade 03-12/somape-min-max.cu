
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

__global__ void sum_prod_scalar(int *A, int *B, int dim, int *result){

    __shared__ int row_sum;
    row_sum = *result;
    
    __syncthreads();

    int id = blockDim.x * blockIdx.x + threadIdx.x;
    
    atomicAdd(&row_sum, A[id] * B[id]);
    
    __syncthreads();

    if(id == blockDim.x * blockIdx.x){
        atomicAdd(result, row_sum);
    }

}

// <<LENGTH, WIDTH>>
__global__ void find_min(int *A, int *B, int dim, int *result){

    __shared__ int row_min;
    row_min = *result;
    __syncthreads();

    int id = blockIdx.x * blockDim.x + threadIdx.x;

    if(id < dim){
        atomicMin(&row_min, A[id]);
        atomicMin(&row_min, B[id]);
    }  

    __syncthreads();

    // se for primeira thread do bloco
    if(id == blockDim.x * blockIdx.x){
        atomicMin(result, row_min);
    }


}   

// <<LENGTH, WIDTH>>
__global__ void find_max(int *A, int *B, int dim, int *result){

    __shared__ int row_max;
    row_max = *result;
    __syncthreads();

    int id = blockIdx.x * blockDim.x + threadIdx.x;

    if(id < dim){
        atomicMax(&row_max, A[id]);
        atomicMax(&row_max, B[id]);
    }  

    __syncthreads();

    // se for primeira thread do bloco
    if(id == blockDim.x * blockIdx.x){
        atomicMax(result, row_max);
    }


}   

int main(int argc, char **argv) {

    // Declara a dimensão da matriz
    int dim;

    // Declara as matrizes
    int *A,*B, *A_device, *B_device;

    // Cria as streams
    cudaStream_t stream1_soma, stream2_min, stream3_max;  
    cudaStreamCreate(&stream1_soma);    // Stream que realiza a soma
    cudaStreamCreate(&stream2_min);     // Stream que realiza o minímo
    cudaStreamCreate(&stream3_max);     // Stream que realiza o máximo

    // Declara o acumulador para o produto escalar global o maximo e o minimo
    int somape = 0,         *somape_device; 
    int minimo = 10000,     *minimo_device;
    int maximo = -10000,    *maximo_device;

    // Aloca os resultados no device
    cudaMalloc((void**)&somape_device, sizeof(int)); 
    cudaMalloc((void**)&minimo_device, sizeof(int));
    cudaMalloc((void**)&maximo_device, sizeof(int));

    // Copia para o device
    cudaMemcpyAsync(somape_device, &somape, sizeof(int), cudaMemcpyHostToDevice, stream1_soma);
    cudaMemcpyAsync(minimo_device, &minimo, sizeof(int), cudaMemcpyHostToDevice, stream2_min);
    cudaMemcpyAsync(maximo_device, &maximo, sizeof(int), cudaMemcpyHostToDevice, stream3_max);
    
    // Lê a dimensão das matrizes
    scanf(" %d", &dim);
    

    // Número de elementos da matriz.
    int tam = dim * dim;

    // Aloca as matrizes no host 
    cudaMallocHost((void**)&A, tam * (sizeof(int))); 
    cudaMallocHost((void**)&B, tam * (sizeof(int)));

    // Aloca as matrizes no device para cada stream
    cudaMalloc((void**)&A_device, tam * (sizeof(int))); 
    cudaMalloc((void**)&B_device, tam * (sizeof(int)));

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
    cudaMemcpyAsync(A_device, A, tam * (sizeof(int)), cudaMemcpyHostToDevice);
    cudaMemcpyAsync(B_device, B, tam * (sizeof(int)), cudaMemcpyHostToDevice);

    sum_prod_scalar <<<dim, dim, 0, stream1_soma>>> (A_device, B_device, tam, somape_device);
    find_min <<<dim, dim, 0, stream2_min>>> (A_device, B_device, tam, minimo_device);
    find_max <<<dim, dim, 0, stream3_max>>> (A_device, B_device, tam, maximo_device);

    cudaMemcpyAsync(&somape, somape_device, sizeof(int), cudaMemcpyDeviceToHost, stream1_soma);
    cudaMemcpyAsync(&minimo, minimo_device, sizeof(int), cudaMemcpyDeviceToHost, stream2_min);
    cudaMemcpyAsync(&maximo, maximo_device, sizeof(int), cudaMemcpyDeviceToHost, stream3_max);
    
    // Sincroniza as streams
    cudaStreamSynchronize(stream1_soma);
    cudaStreamSynchronize(stream2_min);
    cudaStreamSynchronize(stream3_max);

    // Imprime o resultado
    printf("%d %d %d\n",somape, minimo, maximo); 
    
    // Desaloca as matrizes
    cudaFreeHost(A);
    cudaFreeHost(B);
    cudaFree(A_device);
    cudaFree(B_device);
    cudaFree(somape_device);
    cudaFree(minimo_device);
    cudaFree(maximo_device);
    
    // Destroi as streams
    cudaStreamDestroy(stream1_soma);
    cudaStreamDestroy(stream2_min);
    cudaStreamDestroy(stream3_max);

}
