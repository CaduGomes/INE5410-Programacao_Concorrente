#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdlib.h>

// Semáforos usados para controlar o acesso as licenças de software
sem_t controleA;
sem_t controleB;

FILE* out;

void *thread_a(void *args) {
    for (int i = 0; i < *(int*)args; ++i) {
	//      +---> arquivo (FILE*) destino
	//      |    +---> string a ser impressa
	//      v    v
        sem_wait(&controleA);
        fprintf(out, "A");
        // Importante para que vocês vejam o progresso do programa
        // mesmo que o programa trave em um sem_wait().
        fflush(stdout);
        sem_post(&controleB);
    }
    return NULL;
}

void *thread_b(void *args) {
    for (int i = 0; i < *(int*)args; ++i) {
        sem_wait(&controleB);
        fprintf(out, "B");
        fflush(stdout);
        sem_post(&controleA);
    }
    return NULL;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Uso: %s [ITERAÇÕES]\n", argv[0]);
        return 1;
    }
    int iters = atoi(argv[1]);
    srand(time(NULL));
    out = fopen("result.txt", "w");

    pthread_t ta, tb;


    // Inicializa semáforo sem permissões
    sem_init(&controleA, 0, 0);
    sem_init(&controleB, 0, 0);

    // Cria threads
    pthread_create(&ta, NULL, thread_a, &iters);
    pthread_create(&tb, NULL, thread_b, &iters);


    sem_post(&controleA);




    // Espera pelas threads
    pthread_join(ta, NULL);
    pthread_join(tb, NULL);



  // Destrói semáforos
    sem_destroy(&controleA);
    sem_destroy(&controleB);


    //Imprime quebra de linha e fecha arquivo
    fprintf(out, "\n");
    fclose(out);
  
    return 0;
}
