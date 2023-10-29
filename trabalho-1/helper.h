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
    int cliente_index;
    int *clientes;
    pthread_mutex_t *mtx_editar_queue;
    sem_t *sem_entregar_pedido;
    sem_t *sem_atender_cliente;
} queue_t;

typedef struct
{
    bool *fechouBar;
    int id;
    int capacidadeGarcom;
    int qntDePedidosPorRodadaConst;
    int *qntDePedidosPorRodada;
    int *qntDeRodadasGratis;
    int *atendimentosPorRodada;
    pthread_mutex_t *mtx_atendimentos_por_rodada;
    pthread_mutex_t *mtx_rodada_gratis;
    pthread_mutex_t *mtx_qnt_pedidos_rodada;
    sem_t *sem_proxima_rodada;
    queue_t *queue;
} garcom_t;

typedef struct
{
    int id;
    bool *fechouBar;
    int maxTempoAntesDeNovoPedido;
    int maxTempoConsumindoBebida;
    int qntDeGarcons;
    queue_t **queue;
} cliente_t;

void *threadGarcom(void *arg);

void *threadCliente(void *arg);

#endif
