#ifndef HELPER_FILE_H
#define HELPER_FILE_H

#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct
{
    int id;
    int capacidadeGarcom;
} garcom_t;

typedef struct
{
    int id;
    int maxTempoAntesDeNovoPedido;
    int maxTempoConsumindoBebida;
} cliente_t;

typedef struct
{
    int garcom_id;
    int garcom_capacidade;
    int queue_index;
    int *queue;
    bool garcom_entregando;
    pthread_mutex_t *mtx_editar_queue;
    sem_t *sem_entregar_pedido;
    sem_t *sem_atender_cliente;
} queue_t;

void *threadGarcom(void *arg);

void *threadCliente(void *arg);

#endif
