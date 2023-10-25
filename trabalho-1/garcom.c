#include "helper.h"

extern sem_t sem_pedido_entregue;
extern sem_t sem_esperando_pedido;
extern sem_t sem_aguardando_proxima_rodada;
extern sem_t sem_proxima_rodada;

extern sem_t sem_entregar_pedido;

extern sem_t sem_aguardando_atendimento;
extern sem_t sem_anotar_pedido;

extern pthread_mutex_t mtx_diminuir_rodada;

extern bool fechouBar;
extern int qntDeRodadasGratis;
extern int qntDePedidosPorRodada;
extern int qntDePedidosPorRodadaConst;
extern int qntDeGarcons;

extern int **queue;
extern int clienteAtualPedido;
extern int clienteAtualReceber;

bool receberPedido(garcom_t *garcomDados)
{
    sem_post(&sem_aguardando_atendimento);

    sem_wait(&sem_anotar_pedido);

    if (clienteAtualPedido == -1)
    {
        return false;
    }

    if (fechouBar == true)
    {
        return false;
    }
    printf("Cliente %d: fazendo pedido\n", clienteAtualPedido);

    for (size_t i = 0; i < garcomDados->capacidadeGarcom; i++)
    {
        if (queue[garcomDados->id][i] == -1)
        {
            printf("Garcom %d: recebeu o pedido do cliente %d\n", garcomDados->id, clienteAtualPedido);
            queue[garcomDados->id][i] = clienteAtualPedido;
            clienteAtualPedido = -1;
            break;
        }
    }

    return true;
}

void entregaPedidos(int id, int clienteIndex)
{
    sem_post(&sem_entregar_pedido);
    clienteAtualReceber = queue[id][clienteIndex];
    sem_wait(&sem_pedido_entregue);
    printf("Garcom %d: entregou o pedido do cliente %d\n", id, clienteAtualReceber);
    queue[id][clienteIndex] = -1;
}

void *threadGarcom(void *arg)
{
    garcom_t *garcomDados = (garcom_t *)arg;

    int qntPedidosAtuais = 0;

    for (size_t i = 0; i < garcomDados->capacidadeGarcom; i++)
    {
        queue[garcomDados->id][i] = -1;
    }

    printf("Garcom %d: Iniciando trabalho\n", garcomDados->id);

    while (!fechouBar)
    {
        if (qntPedidosAtuais == garcomDados->capacidadeGarcom)
        {
            qntPedidosAtuais = 0;
            printf("Garcom %d: Indo para a copa\n", garcomDados->id);
            printf("Garcom %d: Voltando da copa\n", garcomDados->id);

            for (size_t i = 0; i < garcomDados->capacidadeGarcom; i++)
            {
                entregaPedidos(garcomDados->id, i);
            }

            printf("quantidade de pedidos por rodada: %d\n", qntDePedidosPorRodada);

            pthread_mutex_lock(&mtx_diminuir_rodada);
            qntDePedidosPorRodada -= garcomDados->capacidadeGarcom;
            pthread_mutex_unlock(&mtx_diminuir_rodada);
            printf("quantidade de pedidos por rodada: %d\n", qntDePedidosPorRodada);

            if (qntDePedidosPorRodada == 0)
            {
                pthread_mutex_lock(&mtx_diminuir_rodada);
                qntDePedidosPorRodada = qntDePedidosPorRodadaConst;
                pthread_mutex_unlock(&mtx_diminuir_rodada);
                printf("Garcom %d: Acabou a rodada\n", garcomDados->id);
                while (sem_trywait(&sem_aguardando_proxima_rodada) == 0)
                {
                    sem_post(&sem_proxima_rodada);
                }

                qntDeRodadasGratis--;
            }
            else
            {
                sem_post(&sem_aguardando_proxima_rodada);

                printf("Garcom %d: Aguardando proxima rodada\n", garcomDados->id);

                sem_wait(&sem_proxima_rodada);
            }

            if (qntDeRodadasGratis == 0)
            {
                fechouBar = true;
                printf("Garcom %d: Fechando bar\n", garcomDados->id);
                while (sem_trywait(&sem_esperando_pedido) == 0)
                {
                    sem_post(&sem_entregar_pedido);
                }

                while (sem_trywait(&sem_aguardando_atendimento) == 0)
                {
                    sem_post(&sem_anotar_pedido);
                }

                break;
            }
            printf("Iniciando próxima rodada\n");
        }
        if (receberPedido(garcomDados))
        {
            qntPedidosAtuais++;
        }
    }

    printf("Garcom %d: Indo embora\n", garcomDados->id);

    pthread_exit(NULL);
}
