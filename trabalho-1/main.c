#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

sem_t semaforoPedindo;
sem_t semaforoEsperandoPedido;

bool fechouBar = false;

typedef struct
{
    int id;
    int capacidadeGarcom;
    int qntRodadasGratis;
    bool fechouBar;
} garcom_t;

typedef struct
{
    int id;
    int maxTempoAntesDeNovoPedido;
    int maxTempoConsumindoBebida;
    bool fechouBar;

} cliente_t;

void conversaComAmigos(int id, int maxTempoConversa)
{
    printf("Cliente %d conversando com amigos\n", id);

    sleep(maxTempoConversa / 1000); // TODO: randomizar

    printf("Cliente %d terminou de conversar com amigos\n", id);
}

void consomePedido(int id, int maxTempoConsumindoBebida)
{
    printf("Cliente %d consumindo bebida\n", id);

    sleep(maxTempoConsumindoBebida / 1000); // TODO: randomizar

    printf("Cliente %d terminou de consumir bebida\n", id);
}

bool fazPedido(int id)
{
    printf("Cliente %d aguardando atendimento\n", id);

    sem_wait(&semaforoPedindo);

    if (fechouBar == true)
    {
        printf("Cliente %d nao conseguiu fazer o pedido\n", id);
        return false;
    }

    printf("Cliente %d fez o pedido\n", id);
    return true;
}

bool esperaPedido(int id)
{
    printf("Cliente %d esperando pedido\n", id);

    sem_wait(&semaforoEsperandoPedido);

    printf("Cliente %d recebeu pedido\n", id);
    return true;
}

void *threadCliente(void *arg)
{
    cliente_t *clienteDados = (cliente_t *)arg;

    while (!fechouBar)
    {
        conversaComAmigos(clienteDados->id, clienteDados->maxTempoAntesDeNovoPedido);
        if (fechouBar == true)
            break;

        if (!fazPedido(clienteDados->id))
            break;
        if (esperaPedido(clienteDados->id))
        {
            consomePedido(clienteDados->id, clienteDados->maxTempoConsumindoBebida);
        }
    }

    pthread_exit(NULL);
}

void entregaPedidos(int id)
{
    sem_post(&semaforoEsperandoPedido);

    printf("Garcom %d: entregou o pedido\n", id);
}

void *threadGarcom(void *arg)
{
    garcom_t *garcomDados = (garcom_t *)arg;

    int qntPedidosAtuais = 0;

    while (!fechouBar)
    {
        while (sem_post(&semaforoPedindo) == 0)
        {
            printf("Garcom %d: recebeu o pedido\n", garcomDados->id);
            qntPedidosAtuais++;
            if (qntPedidosAtuais == garcomDados->capacidadeGarcom)
            {
                garcomDados->qntRodadasGratis--;
                qntPedidosAtuais = 0;
                printf("Garcom %d: Indo para a copa\n", garcomDados->id);
                printf("Garcom %d: Voltando da copa\n", garcomDados->id);

                for (size_t i = 0; i < garcomDados->capacidadeGarcom; i++)
                {
                    entregaPedidos(garcomDados->id);
                }

                if (garcomDados->qntRodadasGratis == 0)
                {
                    fechouBar = true;

                    printf("Garcom %d: Fechando a copa\n", garcomDados->id);
                    while (sem_trywait(&semaforoPedindo) == 0)
                    {
                        printf("Garcom %d: Cancelando pedido\n", garcomDados->id);
                    }
                    while (sem_trywait(&semaforoEsperandoPedido) == 0)
                    {
                        printf("Garcom %d: Cancelando pedido\n", garcomDados->id);
                    }
                    pthread_exit(NULL);
                }
            }
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    if (argc < 6)
    {
        printf("Indique a quantidade correta de paramentos na linha de comando: <clientes> <garcons> <clientes/garcon> <rodadas> <max.conversa> <max.consumo>");
        return 1;
    }

    int qntClientes = atoi(argv[1]);
    int qntGarcons = atoi(argv[2]);

    sem_init(&semaforoPedindo, 0, 0);
    sem_init(&semaforoEsperandoPedido, 0, 0);

    pthread_t clientes[qntClientes];
    pthread_t garcons[qntGarcons];

    cliente_t *clienteDados[qntClientes];
    garcom_t *garcomDados[qntGarcons];

    for (int i = 0; i < qntClientes; i++)
    {
        clienteDados[i] = malloc(sizeof(cliente_t));
        clienteDados[i]->id = i;
        clienteDados[i]->maxTempoAntesDeNovoPedido = atoi(argv[5]);
        clienteDados[i]->maxTempoConsumindoBebida = atoi(argv[6]);
        pthread_create(&clientes[i], NULL, threadCliente, (void *)clienteDados[i]);
    }

    for (int i = 0; i < qntGarcons; i++)
    {
        garcomDados[i] = malloc(sizeof(garcom_t));
        garcomDados[i]->id = i;
        garcomDados[i]->capacidadeGarcom = atoi(argv[3]);
        garcomDados[i]->qntRodadasGratis = atoi(argv[4]);
        pthread_create(&garcons[i], NULL, threadGarcom, (void *)garcomDados[i]);
    }

    for (int i = 0; i < qntClientes; i++)
    {
        pthread_join(clientes[i], NULL);
        free(clienteDados[i]);
    }

    for (int i = 0; i < qntGarcons; i++)
    {
        pthread_join(garcons[i], NULL);
        free(garcomDados[i]);
    }

    printf("Fechou o bar\n");

    return 0;
}
