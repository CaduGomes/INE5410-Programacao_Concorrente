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

void *threadGarcom(void *arg);

void *threadCliente(void *arg);

#endif
