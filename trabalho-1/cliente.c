#include "helper.h"

extern sem_t semaforoAguardandoAtendimento;
extern sem_t semaforoEsperandoPedido;
extern sem_t semaforoReceberAtendimento;
extern sem_t semaforoAguardandoProximaRodada;
extern sem_t semaforoProximaRodada;

extern sem_t sem_entregar_pedido;
extern sem_t sem_aguardar_proximo_pedido;

extern sem_t sem_aguardando_atendimento;
extern sem_t sem_anotar_pedido;

extern bool fechouBar;
extern int qntDeRodadasGratis;
extern int qntDePedidosPorRodadaConst;
extern int qntDePedidosPorRodada;
extern int qntDeGarcons;

extern int **queue;
extern int clienteAtualPedido;
extern int clienteAtualReceber;

void sleepRandom(int max)
{
    sleep((rand() % (max + 1)) / 1000);
}

void conversaComAmigos(int id, int maxTempoConversa)
{
    printf("Cliente %d conversando com amigos\n", id);

    sleepRandom(maxTempoConversa);

    printf("Cliente %d terminou de conversar com amigos\n", id);
}

void consomePedido(int id, int maxTempoConsumindoBebida)
{
    printf("Cliente %d consumindo bebida\n", id);

    sleepRandom(maxTempoConsumindoBebida);

    printf("Cliente %d terminou de consumir bebida\n", id);
}

bool fazPedido(int id)
{
    printf("Cliente %d aguardando atendimento\n", id);
    sem_wait(&sem_aguardando_atendimento);
    clienteAtualPedido = id;

    sem_post(&sem_anotar_pedido);

    // printf("Cliente %d fez o pedido\n", id);
    return true;
}

bool esperaPedido(int id)
{
    sem_wait(&sem_entregar_pedido);
    if (id == clienteAtualReceber)
    {
        sem_post(&semaforoEsperandoPedido);
        sem_post(&sem_aguardar_proximo_pedido);

        printf("Cliente %d recebeu pedido\n", id);
        return true;
    }
    else
    {
        sem_post(&sem_entregar_pedido);
        // sem_wait(&sem_aguardar_proximo_pedido);
    }

    return esperaPedido(id);
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
