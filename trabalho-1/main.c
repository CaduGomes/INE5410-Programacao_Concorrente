#include "helper.h"

sem_t sem_pedido_entregue;

sem_t sem_esperando_pedido;

sem_t sem_aguardando_proxima_rodada;

sem_t sem_proxima_rodada;

sem_t sem_entregar_pedido;

sem_t sem_aguardando_atendimento;
sem_t sem_anotar_pedido;

pthread_mutex_t mtx_diminuir_rodada;

bool fechouBar = false;
int qntDeRodadasGratis = -1;
int qntDePedidosPorRodadaConst = -1;
int qntDePedidosPorRodada = -1;
int qntDeGarcons = -1;

int **queue;
int clienteAtualPedido = -1;
int clienteAtualReceber = -1;

int getQntDePedidosPorRodada(int capacidadeGarcom, int qntGarcom, int qntClientes)
{
    int result;
    int capacidadeTodosGarcons = capacidadeGarcom * qntGarcom;
    if (capacidadeTodosGarcons == qntClientes || capacidadeTodosGarcons < qntClientes)
    {
        result = capacidadeTodosGarcons;
    }
    else if (capacidadeTodosGarcons > qntClientes)
    {
        result = qntClientes;
        while (result % capacidadeGarcom != 0)
        {
            result--;
        }
    }
    printf("Quantidade de pedidos por rodada: %d\n", result);
    return result;
}

int main(int argc, char **argv)
{
    if (argc < 6)
    {
        printf("Indique a quantidade correta de paramentos na linha de comando: <clientes> <garcons> <clientes/garcon> <rodadas> <max.conversa> <max.consumo>");
        return 1;
    }

    int qntClientes = atoi(argv[1]);
    qntDeGarcons = atoi(argv[2]);
    int capacidadeGarcom = atoi(argv[3]);

    qntDePedidosPorRodada = getQntDePedidosPorRodada(capacidadeGarcom, qntDeGarcons, qntClientes);
    qntDePedidosPorRodadaConst = qntDePedidosPorRodada;
    qntDeRodadasGratis = atoi(argv[4]);

    pthread_mutex_init(&mtx_diminuir_rodada, NULL);

    sem_init(&sem_pedido_entregue, 0, 0);
    sem_init(&sem_esperando_pedido, 0, 0);

    sem_init(&sem_aguardando_proxima_rodada, 0, 0);

    sem_init(&sem_proxima_rodada, 0, 0);
    sem_init(&sem_entregar_pedido, 0, 0);

    sem_init(&sem_aguardando_atendimento, 0, 0);
    sem_init(&sem_anotar_pedido, 0, 0);

    pthread_t clientes[qntClientes];
    pthread_t garcons[qntDeGarcons];

    cliente_t *clienteDados[qntClientes];
    garcom_t *garcomDados[qntDeGarcons];

    for (int i = 0; i < qntClientes; i++)
    {
        clienteDados[i] = malloc(sizeof(cliente_t));
        clienteDados[i]->id = i;
        clienteDados[i]->maxTempoAntesDeNovoPedido = atoi(argv[5]);
        clienteDados[i]->maxTempoConsumindoBebida = atoi(argv[6]);
        pthread_create(&clientes[i], NULL, threadCliente, (void *)clienteDados[i]);
    }

    queue = malloc(sizeof(int *) * qntDeGarcons);

    for (int i = 0; i < qntDeGarcons; i++)
    {
        queue[i] = malloc(sizeof(int) * qntDePedidosPorRodada);
        garcomDados[i] = malloc(sizeof(garcom_t));
        garcomDados[i]->id = i;
        garcomDados[i]->capacidadeGarcom = capacidadeGarcom;
        pthread_create(&garcons[i], NULL, threadGarcom, (void *)garcomDados[i]);
    }

    for (int i = 0; i < qntClientes; i++)
    {
        pthread_join(clientes[i], NULL);
        free(clienteDados[i]);
    }

    for (int i = 0; i < qntDeGarcons; i++)
    {
        pthread_join(garcons[i], NULL);
        free(garcomDados[i]);
    }

    for (int i = 0; i < qntDeGarcons; i++)
    {
        free(queue[i]);
    }

    free(queue);

    printf("Fechou o bar\n");

    return 0;
}
