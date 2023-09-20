#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Função imprime resultados na correção do exercício -- definida em helper.c
void imprimir_resultados(int n, int **results);

int newCompute(int arg)
{
    int oldRet = 0, oldOldRet = 0, ret = 1;
    for (size_t i = 2; i <= arg; i++)
    {
        oldOldRet = oldRet; // 0 1 1 2 3 5 8 13 21 34 55 89 144 233 377 610 987
        oldRet = ret;       // 1 1 2 3 5 8 13 21 34 55 89 144 233 377 610 987 1597
        ret += oldOldRet;   // 1 2 3 5 8 13 21 34 55 89 144 233 377 610 987 1597 2584
    }
    return ret;
}

// Função wrapper que pode ser usada com pthread_create() para criar uma
// thread que retorna o resultado de compute(arg
void *compute_thread(void *arg)
{
    int *ret = malloc(sizeof(int));
    *ret = newCompute(*((int *)arg));
    return ret;
}

int main(int argc, char **argv)
{
    // Temos n_threads?
    if (argc < 2)
    {
        printf("Uso: %s n_threads x1 x2 ... xn\n", argv[0]);
        return 1;
    }
    // n_threads > 0 e foi dado um x para cada thread?
    int n_threads = atoi(argv[1]);
    if (!n_threads || argc < 2 + n_threads)
    {
        printf("Uso: %s n_threads x1 x2 ... xn\n", argv[0]);
        return 1;
    }

    int args[n_threads];
    int *results[n_threads];
    pthread_t threads[n_threads];
    // Cria threads repassando argv[] correspondente
    for (int i = 0; i < n_threads; ++i)
    {
        args[i] = atoi(argv[2 + i]);
        pthread_create(&threads[i], NULL, compute_thread, &args[i]);
    }
    // Faz join em todas as threads e salva resultados
    for (int i = 0; i < n_threads; ++i)
        pthread_join(threads[i], (void **)&results[i]);

    // Imprime resultados na tela
    // Importante: deve ser chamada para que a correção funcione
    imprimir_resultados(n_threads, results);

    // Faz o free para os resultados criados nas threads
    for (int i = 0; i < n_threads; ++i)
        free(results[i]);

    return 0;
}
