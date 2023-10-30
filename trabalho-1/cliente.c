#include "helper.h"

void printColoridoCliente(int id)
{
    char *cores[6] = {"\033[0;31m", "\033[0;32m", "\033[0;33m", "\033[0;34m", "\033[0;35m", "\033[0;36m"};

    printf("%s", cores[id % 6]);
}

void sleepRandom(int max)
{
    sleep((rand() % (max + 1)) / 1000);
}

void conversaComAmigos(int id, int maxTempoConversa)
{
    printColoridoCliente(id);
    printf("Cliente %d conversando com amigos\n", id);

    sleepRandom(maxTempoConversa);

    printColoridoCliente(id);
    printf("Cliente %d terminou de conversar com amigos\n", id);
}

void consomePedido(int id, int maxTempoConsumindoBebida)
{
    printColoridoCliente(id);
    printf("Cliente %d consumindo bebida\n", id);

    sleepRandom(maxTempoConsumindoBebida);

    printColoridoCliente(id);
    printf("Cliente %d terminou de consumir bebida\n", id);
}

int fazPedido(cliente_t *clienteDados)
{
    int garcom_id = -1;

    for (size_t i = 0; i < clienteDados->qntDeGarcons; i++)
    {
        // Procura um garcom que tenha capacidade para atender, se tiver sem_trywait retorna 0, caso não retorna -1
        if (sem_trywait(clienteDados->queue[i]->sem_atender_cliente) == 0)
        {
            garcom_id = i;

            printColoridoCliente(clienteDados->id);
            printf("Cliente %d: Garcom %d me atendeu\n", clienteDados->id, garcom_id);

            // Usa mutex para evitar que outro cliente faça o mesmo
            pthread_mutex_lock(clienteDados->queue[garcom_id]->mtx_editar_queue);
            // salva o id na lista de pedidos do garcom
            clienteDados->queue[garcom_id]->clientes[clienteDados->queue[garcom_id]->cliente_index] = clienteDados->id;
            // incrementa o index para o próximo cliente
            clienteDados->queue[garcom_id]->cliente_index += 1;
            // libera o mutex
            pthread_mutex_unlock(clienteDados->queue[garcom_id]->mtx_editar_queue);
            break;
        }
    }

    return garcom_id;
}

void esperaPedido(cliente_t *clienteDados, int garcom_id)
{
    sem_wait(clienteDados->queue[garcom_id]->sem_entregar_pedido);

    printColoridoCliente(clienteDados->id);
    printf("Cliente %d recebeu pedido do garcom %d\n", clienteDados->id, garcom_id);
}

void *threadCliente(void *arg)
{
    cliente_t *clienteDados = (cliente_t *)arg;

    while (!*(clienteDados->fechouBar))
    {
        conversaComAmigos(clienteDados->id, clienteDados->maxTempoAntesDeNovoPedido);
        int garcom_id = fazPedido(clienteDados);
        if (garcom_id == -1)
        {
            printColoridoCliente(clienteDados->id);
            printf("Cliente %d: Não conseguiu fazer pedido\n", clienteDados->id);

            break;
        }
        esperaPedido(clienteDados, garcom_id);
        consomePedido(clienteDados->id, clienteDados->maxTempoConsumindoBebida);
    }

    printColoridoCliente(clienteDados->id);
    printf("Cliente %d: Fechando a conta\n", clienteDados->id);

    pthread_exit(NULL);
}
