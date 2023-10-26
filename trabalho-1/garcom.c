#include "helper.h"

extern sem_t sem_pedido_entregue;
extern sem_t sem_aguardando_proxima_rodada;
extern sem_t sem_proxima_rodada;

extern sem_t sem_entregar_pedido;

extern sem_t sem_aguardando_atendimento;
extern sem_t sem_anotar_pedido;

extern pthread_mutex_t mtx_diminuir_rodada;
extern pthread_mutex_t mtx_diminuir_qnt_pedidos;

extern bool fechouBar;
extern int qntDeRodadasGratis;
extern int qntDePedidosPorRodada;
extern int qntDePedidosPorRodadaConst;
extern int qntDeGarcons;
extern int qntDeClientes;

extern int **queue;
extern int clienteAtualReceber;
extern int clienteAtualPedido;
extern int capacidadeGarcom;

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

            pthread_mutex_lock(&mtx_diminuir_qnt_pedidos);
            qntDePedidosPorRodada -= garcomDados->capacidadeGarcom;
            pthread_mutex_unlock(&mtx_diminuir_qnt_pedidos);

            printf("Garcom %d: Quantidade de pedidos por rodada: %d\n", garcomDados->id, qntDePedidosPorRodada);

            if (qntDePedidosPorRodada == 0)
            {
                pthread_mutex_lock(&mtx_diminuir_qnt_pedidos);
                qntDePedidosPorRodada = qntDePedidosPorRodadaConst;
                pthread_mutex_unlock(&mtx_diminuir_qnt_pedidos);
                printf("Garcom %d: Acabou a rodada\n", garcomDados->id);

                for (size_t i = 0; i < (qntDePedidosPorRodadaConst / garcomDados->capacidadeGarcom) - 1; i++)
                {
                    sem_post(&sem_proxima_rodada);
                }

                pthread_mutex_lock(&mtx_diminuir_rodada);
                qntDeRodadasGratis--;
                pthread_mutex_unlock(&mtx_diminuir_rodada);
            }
            else
            {
                printf("Garcom %d: Aguardando proxima rodada\n", garcomDados->id);
                sem_wait(&sem_proxima_rodada);
            }

            printf("Garcom %d: Quantidade de rodadas gratis: %d\n", garcomDados->id, qntDeRodadasGratis);

            if (qntDeRodadasGratis == 0)
            {
                fechouBar = true;
                printf("Garcom %d: Fechando bar\n", garcomDados->id);

                for (size_t i = 0; i < qntDeClientes - qntDePedidosPorRodadaConst; i++)
                {
                    sem_post(&sem_entregar_pedido);
                    sem_post(&sem_aguardando_atendimento);
                }

                break;
            }
        }
        if (receberPedido(garcomDados))
        {
            qntPedidosAtuais++;
        }
    }

    printf("Garcom %d: Indo embora\n", garcomDados->id);

    pthread_exit(NULL);
}
