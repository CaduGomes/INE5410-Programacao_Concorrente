#include "helper.h"

sem_t sem_aguardando_proxima_rodada;

sem_t sem_proxima_rodada;

pthread_mutex_t mtx_diminuir_qnt_pedidos;
pthread_mutex_t mtx_diminuir_rodada;
pthread_mutex_t mtx_atendimentos_por_rodada;

bool fechouBar = false;
int qntDeRodadasGratis = -1;
int qntDePedidosPorRodadaConst = -1;
int qntDePedidosPorRodada = -1;
int capacidadeGarcom = -1;
int qntDeGarcons = -1;
int qntDeClientes = -1;

int atendimentosPorRodada = -1;

queue_t **queue;

int getQntDePedidosPorRodada(int capacidadeGarcom, int qntGarcom, int qntDeClientes)
{
    int result;
    int capacidadeTodosGarcons = capacidadeGarcom * qntGarcom;
    if (capacidadeTodosGarcons == qntDeClientes || capacidadeTodosGarcons < qntDeClientes)
    {
        result = capacidadeTodosGarcons;
    }
    else if (capacidadeTodosGarcons > qntDeClientes)
    {
        result = qntDeClientes;
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
        printf("Indique a quantidade correta de paramentos na linha de comando: <clientes> <garcons> <clientes/garcon> <rodadas> <max.conversa> <max.consumo> \n");
        return 1;
    }

    qntDeClientes = atoi(argv[1]);
    qntDeGarcons = atoi(argv[2]);
    capacidadeGarcom = atoi(argv[3]);

    qntDePedidosPorRodada = getQntDePedidosPorRodada(capacidadeGarcom, qntDeGarcons, qntDeClientes);
    qntDePedidosPorRodadaConst = qntDePedidosPorRodada;
    atendimentosPorRodada = qntDePedidosPorRodadaConst;
    qntDeRodadasGratis = atoi(argv[4]);

    pthread_mutex_init(&mtx_diminuir_rodada, NULL);
    pthread_mutex_init(&mtx_diminuir_qnt_pedidos, NULL);
    pthread_mutex_init(&mtx_atendimentos_por_rodada, NULL);

    sem_init(&sem_aguardando_proxima_rodada, 0, 0);

    sem_init(&sem_proxima_rodada, 0, 0);

    pthread_t clientes[qntDeClientes];
    pthread_t garcons[qntDeGarcons];

    cliente_t *clienteDados[qntDeClientes];
    garcom_t *garcomDados[qntDeGarcons];

    queue = malloc(sizeof(queue_t *) * qntDeGarcons);

    for (int i = 0; i < qntDeGarcons; i++)
    {
        queue[i] = malloc(sizeof(queue_t));

        queue[i]->queue = malloc(sizeof(int) * capacidadeGarcom);

        queue[i]->garcom_id = i;
        queue[i]->garcom_capacidade = capacidadeGarcom;

        queue[i]->mtx_editar_queue = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(queue[i]->mtx_editar_queue, NULL);

        queue[i]->sem_entregar_pedido = (sem_t *)malloc(sizeof(sem_t));
        sem_init(queue[i]->sem_entregar_pedido, 0, 0);

        queue[i]->sem_atender_cliente = (sem_t *)malloc(sizeof(sem_t));
        sem_init(queue[i]->sem_atender_cliente, 0, 0);

        garcomDados[i] = malloc(sizeof(garcom_t));
        garcomDados[i]->id = i;
        garcomDados[i]->capacidadeGarcom = capacidadeGarcom;

        pthread_create(&garcons[i], NULL, threadGarcom, (void *)garcomDados[i]);
    }

    for (int i = 0; i < qntDeClientes; i++)
    {
        clienteDados[i] = malloc(sizeof(cliente_t));
        clienteDados[i]->id = i;
        clienteDados[i]->maxTempoAntesDeNovoPedido = atoi(argv[5]);
        clienteDados[i]->maxTempoConsumindoBebida = atoi(argv[6]);
        pthread_create(&clientes[i], NULL, threadCliente, (void *)clienteDados[i]);
    }

    for (int i = 0; i < qntDeClientes; i++)
    {
        pthread_join(clientes[i], NULL);
        free(clienteDados[i]);
    }

    for (int i = 0; i < qntDeGarcons; i++)
    {
        pthread_join(garcons[i], NULL);
        free(garcomDados[i]);
        pthread_mutex_destroy(queue[i]->mtx_editar_queue);
        free(queue[i]->mtx_editar_queue);
        sem_destroy(queue[i]->sem_entregar_pedido);
        free(queue[i]->sem_entregar_pedido);
        sem_destroy(queue[i]->sem_atender_cliente);
        free(queue[i]->sem_atender_cliente);
        free(queue[i]->queue);
        free(queue[i]);
    }

    pthread_mutex_destroy(&mtx_diminuir_rodada);
    pthread_mutex_destroy(&mtx_diminuir_qnt_pedidos);
    pthread_mutex_destroy(&mtx_atendimentos_por_rodada);

    sem_destroy(&sem_aguardando_proxima_rodada);
    sem_destroy(&sem_proxima_rodada);

    free(queue);

    printf("Fechou o bar\n");

    return 0;
}
