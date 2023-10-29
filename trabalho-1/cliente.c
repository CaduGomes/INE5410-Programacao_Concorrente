#include "helper.h"

extern bool fechouBar;
extern int qntDeGarcons;

extern queue_t **queue;

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

int fazPedido(int id)
{
    int garcom_id = -1;

    for (size_t i = 0; i < qntDeGarcons; i++)
    {
        // Procura um garcom que tenha capacidade para atender, se tiver sem_trywait retorna 0, caso não retorna -1
        if (sem_trywait(queue[i]->sem_atender_cliente) == 0)
        {
            garcom_id = i;

            printf("Cliente %d: Garcom %d me atendeu\n", id, garcom_id);

            // se coloca na lista de pedidos do garcom utilizando mutex para evitar que outro cliente faça o mesmo
            pthread_mutex_lock(queue[garcom_id]->mtx_editar_queue);
            int index = queue[garcom_id]->queue_index;
            queue[garcom_id]->queue[index] = id;
            queue[garcom_id]->queue_index = index + 1;
            pthread_mutex_unlock(queue[garcom_id]->mtx_editar_queue);
            break;
        }
    }

    return garcom_id;
}

void esperaPedido(int id, int garcom_id)
{
    sem_wait(queue[garcom_id]->sem_entregar_pedido);

    printf("Cliente %d recebeu pedido\n", id);
}

void *threadCliente(void *arg)
{
    cliente_t *clienteDados = (cliente_t *)arg;

    while (!fechouBar)
    {
        conversaComAmigos(clienteDados->id, clienteDados->maxTempoAntesDeNovoPedido);
        int garcom_id = fazPedido(clienteDados->id);
        if (garcom_id == -1)
        {
            printf("Cliente %d: Não conseguiu fazer pedido\n", clienteDados->id);
            break;
        }
        esperaPedido(clienteDados->id, garcom_id);
        consomePedido(clienteDados->id, clienteDados->maxTempoConsumindoBebida);
    }

    printf("Cliente %d: Fechando a conta\n", clienteDados->id);
    pthread_exit(NULL);
}
