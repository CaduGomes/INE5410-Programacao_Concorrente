#include "helper.h"

extern sem_t sem_proxima_rodada;

extern pthread_mutex_t mtx_diminuir_rodada;
extern pthread_mutex_t mtx_diminuir_qnt_pedidos;
extern pthread_mutex_t mtx_atendimentos_por_rodada;

extern queue_t **queue;

extern bool fechouBar;
extern int qntDeRodadasGratis;
extern int qntDePedidosPorRodada;
extern int qntDePedidosPorRodadaConst;
extern int qntDeClientes;

extern int atendimentosPorRodada;

void liberarOutrosGarcons(int capacidadeGarcom)
{
    for (size_t i = 0; i < (qntDePedidosPorRodadaConst / capacidadeGarcom) - 1; i++)
    {
        sem_post(&sem_proxima_rodada);
    }
}

void receberPedidos(int id, int capacidadeGarcom)
{

    pthread_mutex_lock(&mtx_atendimentos_por_rodada);
    printf("Atendimentos por rodada: %d\n", atendimentosPorRodada);
    if (atendimentosPorRodada == 0 || atendimentosPorRodada < capacidadeGarcom)
    {
        pthread_mutex_unlock(&mtx_atendimentos_por_rodada);
        return;
    }

    atendimentosPorRodada -= capacidadeGarcom;

    for (size_t i = 0; i < capacidadeGarcom; i++)
    {
        sem_post(queue[id]->sem_atender_cliente);
    }

    pthread_mutex_unlock(&mtx_atendimentos_por_rodada);
}

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
    }

    receberPedidos(garcomDados->id, garcomDados->capacidadeGarcom);

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
                pthread_mutex_lock(&mtx_diminuir_rodada);
                qntDeRodadasGratis--;
                pthread_mutex_unlock(&mtx_diminuir_rodada);
                printf("Garcom %d: Quantidade de rodadas gratis: %d\n", garcomDados->id, qntDeRodadasGratis);
                if (qntDeRodadasGratis == 0)
                {
                    fechouBar = true;
                    printf("Garcom %d: Fechando bar\n", garcomDados->id);
                    liberarOutrosGarcons(garcomDados->capacidadeGarcom);
                    break;
                }

                pthread_mutex_lock(&mtx_diminuir_qnt_pedidos);
                qntDePedidosPorRodada = qntDePedidosPorRodadaConst;
                pthread_mutex_unlock(&mtx_diminuir_qnt_pedidos);

                printf("Garcom %d: Acabou a rodada\n", garcomDados->id);
                atendimentosPorRodada = qntDePedidosPorRodadaConst;
                liberarOutrosGarcons(garcomDados->capacidadeGarcom);
                receberPedidos(garcomDados->id, garcomDados->capacidadeGarcom);
            }
            else
            {
                printf("Garcom %d: Aguardando proxima rodada\n", garcomDados->id);
                sem_wait(&sem_proxima_rodada);

                if (fechouBar)
                {
                    break;
                }

                receberPedidos(garcomDados->id, garcomDados->capacidadeGarcom);
            }
        }
    }

    printf("Garcom %d: Indo embora\n", garcomDados->id);

    pthread_exit(NULL);
}
