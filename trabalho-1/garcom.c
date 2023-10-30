#include "helper.h"

void printColoridoGarcom(int id)
{
    char *cores[6] = {"\033[0;31m", "\033[0;32m", "\033[0;33m", "\033[0;34m", "\033[0;35m", "\033[0;36m"};
    printf("%s", cores[id % 6]);
}

void liberarOutrosGarcons(garcom_t *garcomDados)
{
    int quantidadeDeGarconsEsperando = (garcomDados->qntDePedidosPorRodadaConst / garcomDados->capacidadeGarcom) - 1;
    for (size_t i = 0; i < quantidadeDeGarconsEsperando; i++)
    {
        sem_post(garcomDados->sem_proxima_rodada);
    }
}

void receberPedidos(garcom_t *garcomDados)
{
    pthread_mutex_lock(garcomDados->mtx_atendimentos_por_rodada);
    // Verifica se o garcom pode atender nesta rodada
    if (*(garcomDados->atendimentosPorRodada) == 0 || *(garcomDados->atendimentosPorRodada) < garcomDados->capacidadeGarcom)
    {
        pthread_mutex_unlock(garcomDados->mtx_atendimentos_por_rodada);
        return;
    }

    *(garcomDados->atendimentosPorRodada) -= garcomDados->capacidadeGarcom;
    // Libera a quantidade de pedidos que o garcom pode atender
    for (size_t i = 0; i < garcomDados->capacidadeGarcom; i++)
    {
        sem_post(garcomDados->queue->sem_atender_cliente);
    }

    pthread_mutex_unlock(garcomDados->mtx_atendimentos_por_rodada);
}

void entregaPedidos(garcom_t *garcomDados)
{
    printColoridoGarcom(garcomDados->id);
    printf("Garcom %d: Voltando da copa\n", garcomDados->id);

    for (size_t i = 0; i < garcomDados->capacidadeGarcom; i++)
    {
        // entregando os pedidos para os clientes
        sem_post(garcomDados->queue->sem_entregar_pedido);
    }

    garcomDados->queue->cliente_index = 0;
}

void *threadGarcom(void *arg)
{
    garcom_t *garcomDados = (garcom_t *)arg;

    printColoridoGarcom(garcomDados->id);
    printf("Garcom %d: Iniciando trabalho\n", garcomDados->id);

    receberPedidos(garcomDados);

    garcomDados->queue->cliente_index = 0;

    while (!*(garcomDados->fechouBar))
    {
        if (garcomDados->queue->cliente_index == garcomDados->capacidadeGarcom)
        {
            printColoridoGarcom(garcomDados->id);
            printf("Garcom %d: Indo para a copa\n", garcomDados->id);

            entregaPedidos(garcomDados);

            pthread_mutex_lock(garcomDados->mtx_qnt_pedidos_rodada);
            *(garcomDados->qntDePedidosPorRodada) -= garcomDados->capacidadeGarcom;
            pthread_mutex_unlock(garcomDados->mtx_qnt_pedidos_rodada);

            printf("Garcom %d: Quantidade de pedidos por rodada: %d\n", garcomDados->id, *(garcomDados->qntDePedidosPorRodada));

            if (*(garcomDados->qntDePedidosPorRodada) == 0)
            {
                pthread_mutex_lock(garcomDados->mtx_rodada_gratis);
                *(garcomDados->qntDeRodadasGratis) -= 1;
                pthread_mutex_unlock(garcomDados->mtx_rodada_gratis);

                if (*(garcomDados->qntDeRodadasGratis) == 0)
                {
                    *(garcomDados->fechouBar) = true;
                    printColoridoGarcom(garcomDados->id);
                    printf("Garcom %d: Fechando bar\n", garcomDados->id);
                    liberarOutrosGarcons(garcomDados);
                    break;
                }

                pthread_mutex_lock(garcomDados->mtx_qnt_pedidos_rodada);
                *(garcomDados->qntDePedidosPorRodada) = garcomDados->qntDePedidosPorRodadaConst;
                pthread_mutex_unlock(garcomDados->mtx_qnt_pedidos_rodada);

                printColoridoGarcom(garcomDados->id);
                printf("Garcom %d: Acabou a rodada\n", garcomDados->id);
                *(garcomDados->atendimentosPorRodada) = garcomDados->qntDePedidosPorRodadaConst;
                liberarOutrosGarcons(garcomDados);
                receberPedidos(garcomDados);
            }
            else
            {
                printColoridoGarcom(garcomDados->id);
                printf("Garcom %d: Aguardando proxima rodada\n", garcomDados->id);
                sem_wait(garcomDados->sem_proxima_rodada);

                if (*(garcomDados->fechouBar))
                {
                    break;
                }

                receberPedidos(garcomDados);
            }
        }
    }

    printColoridoGarcom(garcomDados->id);
    printf("Garcom %d: Indo embora\n", garcomDados->id);

    pthread_exit(NULL);
}
