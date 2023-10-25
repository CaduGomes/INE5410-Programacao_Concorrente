#include "helper.h"

extern sem_t semaforoAguardandoAtendimento;
extern sem_t semaforoEsperandoPedido;
extern sem_t semaforoReceberAtendimento;
extern sem_t semaforoAguardandoProximaRodada;
extern sem_t semaforoProximaRodada;

extern bool fechouBar;
extern int qntRodadasGratis;
extern int qntDePedidosPorRodadaConst;
extern int qntDePedidosPorRodada;
extern int qntDeGarcons;

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

            printf("quantidade de pedidos por rodada: %d\n", qntDePedidosPorRodada);
            qntDePedidosPorRodada -= garcomDados->capacidadeGarcom;

            if (qntDePedidosPorRodada == 0)
            {
                printf("Garcom %d: Liberando proxima rodada\n", garcomDados->id);
                while (sem_trywait(&semaforoAguardandoProximaRodada) == 0)
                {
                    sem_post(&semaforoProximaRodada);
                }

                qntRodadasGratis--;
            }
            else
            {
                sem_post(&semaforoAguardandoProximaRodada);

                printf("Garcom %d: Aguardando proxima rodada\n", garcomDados->id);

                sem_wait(&semaforoProximaRodada);
            }

            printf("qntRodadasGratis: %d\n", qntRodadasGratis);
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
            printf("Iniciando próxima rodada\n");
        }
        if (receberPedido(garcomDados->id))
        {
            qntPedidosAtuais++;
        }
    }

    printf("Garcom %d: Fechando o bar\n", garcomDados->id);

    pthread_exit(NULL);
}
