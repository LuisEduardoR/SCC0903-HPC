/*
   Universidade de Sao Paulo - ICMC
   SSC0903 - Computacao de Alto Desempenho
   Atividade 2
   Grupo:
   João Pedro A. S. Secundino (10692054);
   João Pedro Uchôa Cavalcante (10801169);
   Luís Eduardo Rozante de Freitas Pereira (10734794);
   Sérgio Ricardo G. B. Filho (10408386);
*/

# include <omp.h>
# include <math.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

// Faz a impressão da matriz. 
void printa_matriz(size_t n, size_t m, double **mat) {

    for(size_t i = 0; i < n; i++) {
        for(size_t j = 0; j < m; j++)
            printf("%.1lf ", mat[i][j]);
        printf("\n");
    }

}

// Função que compara dois números.
int fun_comparacao (const void *a, const void *b){
    return ( *(double *)a - *(double *)b );
}

// Média aritmética Somatório de todos os elementos da amostra, divididos pelo tamanho
// da amostra.
double calcula_media_aritmetica(size_t len, double *linha) {

    // Calcula o somatório dos elementos da linha.
    double sum = 0;
    for(size_t i = 0; i < len; i++)
        sum += linha[i];

    // Retorna a média aritmética dos elementos.
    return (sum / len);

}

// Média harmônica: Razão entre o tamanho da amostra e o somatório do inverso das amostras.
double calcula_media_harmonica(size_t len, double *linha) {

    // Calcula o somatório dos inversos dos elementos da linha.
    double inv_sum = 0;
    for(size_t i = 0; i < len; i++)
        inv_sum += 1 / linha[i];

    // Retorna a média harmônica dos elementos.
    return (len / inv_sum);

}

// Mediana: Elemento médio da amostra (elemento médio da coluna ordenada). Para um
// número par de elementos, a mediana é a média entre os elementos do meio
// ((n/2+n/2+1)/2).
double calcula_mediana(size_t len, double *linha_ordenana) {

    // Retorna a média entre os elementos do meio se len é par.
    if (len % 2 == 0) {
        return (linha_ordenana[len / 2 - 1] + linha_ordenana[len / 2]) / 2;
    }

    // Retorna o elemento do meio se len é ímpar.
    return linha_ordenana[len / 2 - 1];

}

// Moda: Elemento mais frequente da amostra (elemento que mais aparece na coluna, se
// houver mais de um, considera-se somente o primeiro. Se não houver, retorna -1).
double calcula_moda(size_t len, double *linha_ordenana) {
    
    // Guarda o valor a ser retornado como moda e sua frequência.
    double moda = -1.0;
    int freq_moda = 1;

    // Guarda o valor que estamos avaliando.
    double atual = linha_ordenana[0];
    int freq_atual = 0;

    // Analisa os elementos e encontra a moda.
    for (size_t i = 0; i < len; i++){
        
        // Começa a avaliar um novo valor.
        if(linha_ordenana[i] != atual) {
            freq_atual = 1;
            atual = linha_ordenana[i];
        } else {
            freq_atual++;
        }

        // Se a frequência do atual é maior que a da moda, atualiza a moda.
        if(freq_atual > freq_moda) {
            freq_moda = freq_atual;
            moda = atual;
        }

    }

    return moda;

}   

// Variância: Soma dos quadrados das diferenças entre o elemento da amostra e a média
// aritmética calculada.
double calcula_variancia(size_t len, double *linha, double media) {

    // Calcula o somatório dos quadrados das diferenças da linha.
    double sum = 0;
    for(size_t i = 0; i < len; i++) {
        double dif = (linha[i] - media);
        sum += (dif * dif);
    }

    // Retorna a variância da amostra.
    return sum / (len - 1);
}

// Desvio padrão: Raiz quadrada da variância.
double calcula_desvio_padrao(double variancia) {
    return sqrt(variancia);
}

// Coeficiente de variação: Razão entre o desvio padrão e a média aritmética. 
double calcula_coeficiente_variacao(double desvio, double media) {
    return desvio / media;
}
