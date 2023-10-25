#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

sem_t semaforoAguardandoAtendimento;
sem_t semaforoAguardandoPedido;
sem_t semaforoGarcomLivre;
sem_t semaforoFezPedido;
sem_t semafotoGarcomEsperaPedidosNaRodada;

bool fechouBar = false;

int qntRodadasGratis;
int qntPedidosTotaisNaRodada = 0;
int qntPedidosParaForcarRodada = 0;
int qntGarcons = 0;

typedef struct
{
    int id;
    int capacidadeGarcom;
    bool fechouBar;
} garcom_t;

typedef struct
{
    int id;
    int maxTempoAntesDeNovoPedido;
    int maxTempoConsumindoBebida;
    bool fechouBar;

} cliente_t;

void conversaComAmigos(int id, int maxTempoConversa)
{
    printf("Cliente %d conversando com amigos\n", id);

    int r = rand() % (maxTempoConversa + 1);

    printf("Conversando: %d \n", r);

    sleep(r / 1000); // TODO: randomizar

    printf("Cliente %d terminou de conversar com amigos\n", id);
}

void consomePedido(int id, int maxTempoConsumindoBebida)
{
    printf("Cliente %d consumindo bebida\n", id);

    int r = rand() % (maxTempoConsumindoBebida + 1);

    printf("Consumindo: %d \n", r);

    sleep(r / 1000); // TODO: randomizar
    printf("Cliente %d terminou de consumir bebida\n", id);
}

void fazPedido(int id)
{
    printf("Cliente %d aguardando atendimento\n", id);

    sem_post(&semaforoAguardandoAtendimento);

    sem_wait(&semaforoGarcomLivre);

    printf("Cliente %d fez um pedido\n", id);

    sem_post(&semaforoFezPedido);
}

void esperaPedido(int id)
{
    printf("Cliente %d esperando pedido\n", id);

    sem_wait(&semaforoAguardandoPedido);

    printf("Cliente %d recebeu pedido\n", id);
}

void *threadCliente(void *arg)
{
    cliente_t *clienteDados = (cliente_t *)arg;

    while (!fechouBar)
    {

        conversaComAmigos(clienteDados->id, clienteDados->maxTempoAntesDeNovoPedido);

        if (fechouBar)
        {
            pthread_exit(NULL);
        }

        fazPedido(clienteDados->id);
        esperaPedido(clienteDados->id);
        consomePedido(clienteDados->id, clienteDados->maxTempoConsumindoBebida);

        if(fechouBar)
        {
            pthread_exit(NULL);
        }
        
    }

    pthread_exit(NULL);
}

void receberPedido(int id)
{
    printf("Garcom %d livre\n", id);

    sem_wait(&semaforoAguardandoAtendimento);

    sem_post(&semaforoGarcomLivre);

    sem_wait(&semaforoFezPedido);

    printf("Garcom %d: recebeu o pedido\n", id);
}
void entregaPedidos(int id)
{
    sem_post(&semaforoAguardandoPedido);

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

            if (qntPedidosTotaisNaRodada != qntPedidosParaForcarRodada) {
                printf("Garcom %d esperando termino da rodada\n", garcomDados->id);
                sem_wait(&semafotoGarcomEsperaPedidosNaRodada);
            } else {
                printf("Garcom %d responsavel por passar a rodada\n", garcomDados->id);
                for(size_t i = 0; i < qntGarcons; i++) {
                    sem_post(&semafotoGarcomEsperaPedidosNaRodada);
                }

                qntRodadasGratis--;
                qntPedidosTotaisNaRodada = 0;

                printf("NOVA RODADA\n");
            }
                
            qntPedidosAtuais = 0;
            printf("Garcom %d: Indo para a copa\n", garcomDados->id);
            printf("Garcom %d: Voltando da copa\n", garcomDados->id);

            for (size_t i = 0; i < garcomDados->capacidadeGarcom; i++)
            {
                entregaPedidos(garcomDados->id);
            }

            if (qntRodadasGratis-- <= 0)
            {
                printf("Garcom %d: Fechando a copa\n", garcomDados->id);
                fechouBar = true;
                pthread_exit(NULL);
            }
        } else {
            receberPedido(garcomDados->id);
            qntPedidosAtuais++;
            qntPedidosTotaisNaRodada++;
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    if (argc < 6)
    {
        printf("Indique a quantidade correta de paramentos na linha de comando: <clientes> <garcons> <clientes/garcon> <rodadas> <max.conversa> <max.consumo>");
        return 1;
    }

    int qntClientes = atoi(argv[1]);
    qntGarcons = atoi(argv[2]);

    sem_init(&semaforoAguardandoPedido, 0, 0);
    sem_init(&semaforoAguardandoAtendimento, 0, 0);
    sem_init(&semaforoGarcomLivre, 0, 0);
    sem_init(&semaforoFezPedido, 0, 0);

    pthread_t clientes[qntClientes];
    pthread_t garcons[qntGarcons];

    cliente_t *clienteDados[qntClientes];
    garcom_t *garcomDados[qntGarcons];

    int capMaxGarcom = atoi(argv[3]);
    int capMaxGarcomTotal = capMaxGarcom * qntGarcons;

    if (capMaxGarcomTotal == qntClientes || capMaxGarcomTotal < qntClientes)
    {
        qntPedidosParaForcarRodada = capMaxGarcomTotal;
    }
    else if (capMaxGarcomTotal > qntClientes)
    {
        qntPedidosParaForcarRodada = qntClientes;
        while (qntPedidosParaForcarRodada % capMaxGarcom != 0)
        {
            qntPedidosParaForcarRodada--;
        }
    }

    qntRodadasGratis = atoi(argv[4]);

    printf("Clientes: %d \nGarcons: %d\nQntParaForcarRodada: %d\nQuantidade rodadas: %d\n", qntClientes, qntGarcons, qntPedidosParaForcarRodada, qntRodadasGratis);

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
