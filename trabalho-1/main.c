#include "helper.h"

sem_t semaforoAguardandoAtendimento;
sem_t semaforoEsperandoPedido;
sem_t semaforoReceberAtendimento;
sem_t semaforoAguardandoProximaRodada;
sem_t semaforoProximaRodada;

bool fechouBar = false;
int qntRodadasGratis = 0;
int qntDePedidosPorRodadaConst = 0;
int qntDePedidosPorRodada = 0;
int qntDeGarcons = 0;

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
    sem_init(&semaforoAguardandoProximaRodada, 0, 0);
    sem_init(&semaforoProximaRodada, 0, 0);

    pthread_t clientes[qntClientes];
    pthread_t garcons[qntGarcons];

    cliente_t *clienteDados[qntClientes];
    garcom_t *garcomDados[qntGarcons];

    qntRodadasGratis = atoi(argv[4]);
    int capMaxGarcom = atoi(argv[3]);
    int capMaxGarcomTotal = capMaxGarcom * qntGarcons;

    if (capMaxGarcomTotal == qntClientes || capMaxGarcomTotal < qntClientes)
    {
        qntDePedidosPorRodada = capMaxGarcomTotal;
    }
    else if (capMaxGarcomTotal > qntClientes)
    {
        qntDePedidosPorRodada = qntClientes;
        while (qntDePedidosPorRodada % capMaxGarcom != 0)
        {
            qntDePedidosPorRodada--;
        }
    }

    printf("Quantidade de pedidos por rodada: %d\n", qntDePedidosPorRodada);

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
