#include "helper.h"

int getQntDePedidosPorRodada(int capacidadeGarcom, int qntGarcom, int qntDeClientes)
{
    // Variável que armazena a quantidade de pedidos por rodada
    int result;

    // Capacidade máxima do bar
    int capacidadeTodosGarcons = capacidadeGarcom * qntGarcom;

    // Caso a capacidade do bar seja igual ou menor que a quantidade de clientes
    if (capacidadeTodosGarcons == qntDeClientes || capacidadeTodosGarcons < qntDeClientes)
    {
        result = capacidadeTodosGarcons;
    }
    // Caso a capacidade do bar seja maior que a quantidade de clientes
    else if (capacidadeTodosGarcons > qntDeClientes)
    {
        result = qntDeClientes;
        // Caso a quantidade de clientes não seja divisível pela capacidade do garçom
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
    // Verifica se a quantidade de argumentos é a correta
    if (argc < 6)
    {
        printf("Indique a quantidade correta de paramentos na linha de comando: <clientes> <garcons> <clientes/garcon> <rodadas> <max.conversa> <max.consumo> \n");
        return 1;
    }

    int qntDeClientes = atoi(argv[1]);
    int qntDeGarcons = atoi(argv[2]);
    int capacidadeGarcom = atoi(argv[3]);

    // Verifica se a quantidade de clientes é maior que a quantidade de garçons
    if (capacidadeGarcom > qntDeClientes)
    {
        capacidadeGarcom = qntDeClientes;
        printf("A capacidade do garçom foi alterada pois a quantidade de clientes era menor que a capacidade do garçom\n");
        printf("Nova capacidade do garçom: %d\n", capacidadeGarcom);
    }

    int qntDePedidosPorRodadaConst = getQntDePedidosPorRodada(capacidadeGarcom, qntDeGarcons, qntDeClientes);

    int qntDePedidosPorRodada = qntDePedidosPorRodadaConst;
    int atendimentosPorRodada = qntDePedidosPorRodadaConst;
    int qntDeRodadasGratis = atoi(argv[4]);

    bool fechouBar = false;

    // Declaração das threads
    pthread_t clientes[qntDeClientes];
    pthread_t garcons[qntDeGarcons];

    // Declaração dos dados das threads
    cliente_t *clienteDados[qntDeClientes];
    garcom_t *garcomDados[qntDeGarcons];
    queue_t *queue[qntDeGarcons];

    // Inicialização dos mutex do garçom
    pthread_mutex_t mtx_atendimentos_por_rodada;
    pthread_mutex_init(&mtx_atendimentos_por_rodada, NULL);

    pthread_mutex_t mtx_rodada_gratis;
    pthread_mutex_init(&mtx_rodada_gratis, NULL);

    pthread_mutex_t mtx_qnt_pedidos_rodada;
    pthread_mutex_init(&mtx_qnt_pedidos_rodada, NULL);

    // Inicialização do semáforo para a próxima rodada
    sem_t sem_proxima_rodada;
    sem_init(&sem_proxima_rodada, 0, 0);

    for (int i = 0; i < qntDeGarcons; i++)
    {
        queue[i] = malloc(sizeof(queue_t));

        queue[i]->clientes = malloc(sizeof(int) * capacidadeGarcom);
        queue[i]->mtx_editar_queue = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(queue[i]->mtx_editar_queue, NULL);

        queue[i]->sem_entregar_pedido = (sem_t *)malloc(sizeof(sem_t));
        sem_init(queue[i]->sem_entregar_pedido, 0, 0);

        queue[i]->sem_atender_cliente = (sem_t *)malloc(sizeof(sem_t));
        sem_init(queue[i]->sem_atender_cliente, 0, 0);

        garcomDados[i] = malloc(sizeof(garcom_t));
        garcomDados[i]->queue = queue[i];

        garcomDados[i]->id = i;
        garcomDados[i]->capacidadeGarcom = capacidadeGarcom;
        garcomDados[i]->qntDePedidosPorRodadaConst = qntDePedidosPorRodadaConst;

        garcomDados[i]->fechouBar = &fechouBar;
        garcomDados[i]->qntDePedidosPorRodada = &qntDePedidosPorRodada;
        garcomDados[i]->qntDeRodadasGratis = &qntDeRodadasGratis;
        garcomDados[i]->atendimentosPorRodada = &atendimentosPorRodada;

        garcomDados[i]->mtx_atendimentos_por_rodada = &mtx_atendimentos_por_rodada;
        garcomDados[i]->mtx_rodada_gratis = &mtx_rodada_gratis;
        garcomDados[i]->mtx_qnt_pedidos_rodada = &mtx_qnt_pedidos_rodada;

        garcomDados[i]->sem_proxima_rodada = &sem_proxima_rodada;

        pthread_create(&garcons[i], NULL, threadGarcom, (void *)garcomDados[i]);
    }

    for (int i = 0; i < qntDeClientes; i++)
    {
        clienteDados[i] = malloc(sizeof(cliente_t));
        clienteDados[i]->id = i;
        clienteDados[i]->qntDeGarcons = qntDeGarcons;
        clienteDados[i]->maxTempoAntesDeNovoPedido = atoi(argv[5]);
        clienteDados[i]->maxTempoConsumindoBebida = atoi(argv[6]);
        clienteDados[i]->queue = queue;

        clienteDados[i]->fechouBar = &fechouBar;

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
        // Libera a memória alocada para o garcom

        free(garcomDados[i]);
        // Libera a memória alocada para a queue
        pthread_mutex_destroy(queue[i]->mtx_editar_queue);
        free(queue[i]->mtx_editar_queue);

        sem_destroy(queue[i]->sem_entregar_pedido);
        free(queue[i]->sem_entregar_pedido);

        sem_destroy(queue[i]->sem_atender_cliente);

        free(queue[i]->sem_atender_cliente);
        free(queue[i]->clientes);
        free(queue[i]);
    }

    sem_destroy(&sem_proxima_rodada);

    pthread_mutex_destroy(&mtx_qnt_pedidos_rodada);

    pthread_mutex_destroy(&mtx_rodada_gratis);

    pthread_mutex_destroy(&mtx_atendimentos_por_rodada);

    printf("Fechou o bar\n");

    return 0;
}
