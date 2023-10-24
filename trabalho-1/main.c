#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

sem_t semaforoPedindo;
sem_t semaforoEsperandoPedido;

sem_t semaforoGarcomLivre;

bool fechouBar = false;
bool garcomLivre = true;

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

    int r = rand() % (maxTempoConversa + 1);

    printf("Conversando: %d \n", r);

    sleep(r / 1000); // TODO: randomizar

    printf("Cliente %d terminou de conversar com amigos\n", id);
}

void consomePedido(int id, int maxTempoConsumindoBebida)
{
    printf("Cliente %d consumindo bebida\n", id);

    int r = rand() % (maxTempoConsumindoBebida + 1);

    printf("Consumindo: %d \n", r);

    sleep(r / 1000); // TODO: randomizar
    printf("Cliente %d terminou de consumir bebida\n", id);
}

void fazPedido(int id)
{
    printf("Cliente %d aguardando atendimento\n", id);

    if (sem_post(&semaforoPedindo) == 0)
    {
        printf("Cliente %d fez o pedido\n", id);
    }
    else
    {
        printf("Cliente %d nao conseguiu fazer o pedido\n", id);
    }
}

void esperaPedido(int id)
{
    printf("Cliente %d esperando pedido\n", id);

    sem_wait(&semaforoEsperandoPedido);

    printf("Cliente %d recebeu pedido\n", id);
}

void *threadCliente(void *arg)
{
    cliente_t *clienteDados = (cliente_t *)arg;

    while (!fechouBar)
    {
        // conversaComAmigos(clienteDados->id, clienteDados->maxTempoAntesDeNovoPedido);
        // if (fechouBar == false)
        // {
        //     fazPedido(clienteDados->id);
        //     esperaPedido(clienteDados->id);
        //     consomePedido(clienteDados->id, clienteDados->maxTempoConsumindoBebida);
        // }
        // else
        // {
        //     pthread_exit(NULL);
        // }

        conversaComAmigos(clienteDados->id, clienteDados->maxTempoAntesDeNovoPedido);

        if (fechouBar)
        {
            pthread_exit(NULL);
        }

        if (garcomLivre)
        {
            fazPedido(clienteDados->id);
            esperaPedido(clienteDados->id);
            consomePedido(clienteDados->id, clienteDados->maxTempoConsumindoBebida);
        }
        else
        {
            sem_wait(&semaforoGarcomLivre);
        }
    }

    pthread_exit(NULL);
}

void receberPedido(int id)
{
    sem_wait(&semaforoPedindo);

    printf("Garcom %d: recebeu o pedido\n", id);
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
        printf("quantidade de pedidos atuais: %d\n", qntPedidosAtuais);
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
                printf("Garcom %d: Fechando a copa\n", garcomDados->id);
                fechouBar = true;
                pthread_exit(NULL);
            }

            garcomLivre = true;
            sem_post(&semaforoGarcomLivre);
        }
        receberPedido(garcomDados->id);
        qntPedidosAtuais++;
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
