#include "helper.h"

extern sem_t sem_proxima_rodada;

extern pthread_mutex_t mtx_diminuir_rodada;
extern pthread_mutex_t mtx_diminuir_qnt_pedidos;

extern queue_t **queue;

extern bool fechouBar;
extern int qntDeRodadasGratis;
extern int qntDePedidosPorRodada;
extern int qntDePedidosPorRodadaConst;
extern int qntDeClientes;

void entregaPedidos(int id, int capacidadeGarcom)
{
    printf("Garcom %d: Voltando da copa\n", id);

    for (size_t i = 0; i < capacidadeGarcom; i++)
    {
        // entregando os pedidos para os clientes
        sem_post(queue[id]->sem_entregar_pedido);

        // zerando fila do garcom
        queue[id]->queue[i] = -1;
    }

    queue[id]->queue_index = 0;
}

void *threadGarcom(void *arg)
{
    garcom_t *garcomDados = (garcom_t *)arg;

    for (size_t i = 0; i < garcomDados->capacidadeGarcom; i++)
    {
        queue[garcomDados->id]->queue[i] = -1;
        sem_post(queue[garcomDados->id]->sem_atender_cliente);
    }

    queue[garcomDados->id]->queue_index = 0;

    printf("Garcom %d: Iniciando trabalho\n", garcomDados->id);

    while (!fechouBar)
    {
        if (queue[garcomDados->id]->queue_index == garcomDados->capacidadeGarcom)
        {

            printf("Garcom %d: Indo para a copa\n", garcomDados->id);

            entregaPedidos(garcomDados->id, garcomDados->capacidadeGarcom);

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

                break;
            }
            else
            {
                for (size_t i = 0; i < queue[garcomDados->id]->garcom_capacidade; i++)
                {
                    sem_post(queue[garcomDados->id]->sem_atender_cliente);
                }
            }
        }
        // if (receberPedido(garcomDados))
        // {
        //     qntPedidosAtuais++;
        // }
    }

    printf("Garcom %d: Indo embora\n", garcomDados->id);

    pthread_exit(NULL);
}
