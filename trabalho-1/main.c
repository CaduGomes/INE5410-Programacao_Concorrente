#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

sem_t semaforoAguardandoAtendimento;
sem_t semaforoEsperandoPedido;
sem_t semaforoReceberAtendimento;
sem_t semaforoProximaRodada;

bool fechouBar = false;
int qntRodadasGratis = 0;

typedef struct
{
    int id;
    int capacidadeGarcom;
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

    sleep(r / 1000);

    printf("Cliente %d terminou de conversar com amigos\n", id);
}

void consomePedido(int id, int maxTempoConsumindoBebida)
{
    printf("Cliente %d consumindo bebida\n", id);

    int r = rand() % (maxTempoConsumindoBebida + 1);

    sleep(r / 1000);

    printf("Cliente %d terminou de consumir bebida\n", id);
}

bool fazPedido(int id)
{
    sem_post(&semaforoAguardandoAtendimento);

    printf("Cliente %d aguardando atendimento\n", id);

    sem_wait(&semaforoReceberAtendimento);

    if (fechouBar == true)
    {
        printf("Cliente %d nao conseguiu fazer o pedido, bar fechado\n", id);
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

    printf("Cliente %d: Fechando a conta\n", clienteDados->id);
    pthread_exit(NULL);
}

bool receberPedido(int id)
{
    sem_wait(&semaforoAguardandoAtendimento);

    if (fechouBar == true)
    {
        printf("Garcom %d: Bar fechado\n", id);
        return false;
    }

    sem_post(&semaforoReceberAtendimento);

    sleep(1);
    printf("Garcom %d: recebeu o pedido\n", id);

    return true;
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
        if (qntPedidosAtuais == garcomDados->capacidadeGarcom)
        {
            qntPedidosAtuais = 0;
            printf("Garcom %d: Indo para a copa\n", garcomDados->id);
            printf("Garcom %d: Voltando da copa\n", garcomDados->id);

            for (size_t i = 0; i < garcomDados->capacidadeGarcom; i++)
            {
                entregaPedidos(garcomDados->id);
            }
            // sem_wait(&semaforoProximaRodada);

            if (qntRodadasGratis == 0)
            {
                fechouBar = true;

                while (sem_trywait(&semaforoAguardandoAtendimento) == 0)
                {
                    sem_post(&semaforoReceberAtendimento);
                    printf("Garcom %d: Cancelando pedido\n", garcomDados->id);
                }
                break;
            }
        }
        if (receberPedido(garcomDados->id))
        {
            qntPedidosAtuais++;
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

    sem_init(&semaforoAguardandoAtendimento, 0, 0);
    sem_init(&semaforoReceberAtendimento, 0, 0);
    sem_init(&semaforoEsperandoPedido, 0, 0);
    sem_init(&semaforoProximaRodada, 0, 0);

    pthread_t clientes[qntClientes];
    pthread_t garcons[qntGarcons];

    cliente_t *clienteDados[qntClientes];
    garcom_t *garcomDados[qntGarcons];

    qntRodadasGratis = atoi(argv[4]);

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
