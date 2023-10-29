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

sem_t semaforoGarcomAceitouPedido;

sem_t semaforoGarconsCriados;

pthread_mutex_t mtx;

bool fechouBar = false;

int qntRodadasGratis;
int qntPedidosTotaisNaRodada = 0;
int qntPedidosParaForcarRodada = 0;
int qntGarcons = 0;

typedef struct
{
    int id;
    int capacidadeGarcom;

    bool garcomLivre;
    sem_t semaforoGarcomLivre;

    sem_t semaforoAceitaPedido;
    sem_t semaforoEntregaPedido;
    sem_t semaforoAguardaOrdemPedido;

    int pedidoAtual;

    int indexFila;
    int* fila;
} garcom_t;

garcom_t** garcomList;

typedef struct
{
    int id;
    int maxTempoAntesDeNovoPedido;
    int maxTempoConsumindoBebida;

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

void fazPedido(int id, int garcomId)
{
    printf("Cliente %d aguardando atendimento\n", id);

    // sem_wait(&semaforoAguardandoAtendimento);

    garcom_t* garcom = garcomList[garcomId];

    sem_wait(&garcom->semaforoGarcomLivre);

    if(fechouBar) {
        printf("Cliente %d nao conseguiu fazer o pedido e foi embora\n", id);
        pthread_exit(NULL);
    }

    // sem_post(&garcom->semaforoAceitaPedido);

    printf("Cliente %d fez um pedido para garcom %d\n", id, garcomId);

    // sem_wait(&semaforoGarcomAceitouPedido);

    // printf("Cliente %d teve seu pedido aceito pelo garcom %d\n", id, garcomId);

    // sem_post(&semaforoFezPedido);
    
    garcom->indexFila++;
    printf("Garcom %d tem indexFila %d\n", garcom->id, garcom->indexFila);
    garcom->fila[garcom->indexFila] = id;
}

void esperaPedido(int id, int garcomId)
{
    garcom_t* garcom = garcomList[garcomId];
    printf("Cliente %d esperando pedido que vai ser entregue por %d\n", id, garcom->id);

    sem_wait(&garcom->semaforoEntregaPedido);

    printf("Cliente %d recebeu pedido do garcom %d\n", id, garcomId);

    pthread_mutex_lock(&mtx);
    garcom->fila[garcom->indexFila] = -1;
    garcom->indexFila--;
    pthread_mutex_unlock(&mtx);

}

void *threadCliente(void *arg)
{
    cliente_t *clienteDados = (cliente_t *)arg;

    int garcom;

    if(qntGarcons == 1) {
        garcom = 0;
    } else {
        garcom = rand() % (qntGarcons);
    }

    while (!fechouBar)
    {

        conversaComAmigos(clienteDados->id, clienteDados->maxTempoAntesDeNovoPedido);

        if (fechouBar)
        {
            pthread_exit(NULL);
        }

        fazPedido(clienteDados->id, garcom);
        esperaPedido(clienteDados->id, garcom);
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
    garcom_t* garcom = garcomList[id];

    printf("Garcom %d livre\n", garcom->id);

    sem_post(&garcom->semaforoGarcomLivre);

    // sem_post(&semaforoAguardandoAtendimento);

    // printf("Garcom %d aguardando chamada do cliente\n", id);

    // sem_wait(&garcom->semaforoAceitaPedido);

    // sem_post(&semaforoGarcomAceitouPedido);

    printf("Garcom %d: recebeu o pedido\n", garcom->id);
    sleep(1);

}
void entregaPedidos(int id)
{

    garcom_t* garcom = garcomList[id];

    for(int i = 0; i < garcom->capacidadeGarcom; i++) {
        sem_post(&garcom->semaforoEntregaPedido);
    }
}

void *threadGarcom(void* i)
{

    sleep(1);

    int* value = (int*)i;
    int index = *value;  

    garcom_t* garcom = garcomList[index];

    int qntPedidosAtuais = 0;

    while (!fechouBar)
    {
        if (qntPedidosAtuais == garcom->capacidadeGarcom)
        {

            garcom->garcomLivre = false;

            if (qntPedidosTotaisNaRodada != qntPedidosParaForcarRodada) {
                printf("Garcom %d esperando termino da rodada\n", garcom->id);
                sem_wait(&semafotoGarcomEsperaPedidosNaRodada);
            } else {
                printf("Garcom %d responsavel por passar a rodada\n", garcom->id);
                for(size_t i = 0; i < qntGarcons - 1; i++) {
                    sem_post(&semafotoGarcomEsperaPedidosNaRodada);
                }

                qntRodadasGratis--;
                qntPedidosTotaisNaRodada = 0;

                printf("NOVA RODADA\n");
            }
                
            qntPedidosAtuais = 0;
            printf("Garcom %d: Indo para a copa\n", garcom->id);
            printf("Garcom %d: Voltando da copa\n", garcom->id);

            for (size_t i = 0; i < garcom->capacidadeGarcom; i++)
            {
                entregaPedidos(garcom->id);
            }

            if (qntRodadasGratis <= 0)
            {
                printf("Garcom %d: Fechando a copa\n", garcom->id);
                fechouBar = true;
                for(size_t i = 0; i < garcom->capacidadeGarcom * qntPedidosParaForcarRodada; i++) {
                    sem_post(&garcom->semaforoGarcomLivre);
                }

                pthread_exit(NULL);


            }
        } else {
            receberPedido(garcom->id);
            qntPedidosAtuais++;

            pthread_mutex_lock(&mtx);
            qntPedidosTotaisNaRodada++;
            pthread_mutex_unlock(&mtx);
        }
    }

    free(i);
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

    pthread_mutex_init(&mtx, NULL);

    pthread_t clientes[qntClientes];
    pthread_t garcons[qntGarcons];

    cliente_t *clienteDados[qntClientes];
    garcomList = (garcom_t**) malloc(qntGarcons * sizeof(garcom_t*));

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

    int *arg = (int*) malloc(sizeof(int));

    for (int i = 0; i < qntGarcons; i++)
    {
        garcomList[i] = (garcom_t*) malloc(sizeof(garcom_t));
        garcomList[i]->id = i;
        garcomList[i]->capacidadeGarcom = atoi(argv[3]);
        garcomList[i]->indexFila = -1;
        garcomList[i]->garcomLivre = true;

        garcomList[i]->fila = (int*) malloc (capMaxGarcom * sizeof(int));

        for(int j = 0; j < capMaxGarcom; j++) {
            garcomList[i]->fila[j] = -1;
        }

        sem_init(&garcomList[i]->semaforoAceitaPedido, 0, 0);
        sem_init(&garcomList[i]->semaforoEntregaPedido, 0, 0);
        sem_init(&garcomList[i]->semaforoAguardaOrdemPedido, 0, 0);
        sem_init(&garcomList[i]->semaforoGarcomLivre, 0, 0);

        *arg = i;

        pthread_create(&garcons[i], NULL, threadGarcom, (void*) arg);     
    }

    for (int i = 0; i < qntClientes; i++)
    {
        clienteDados[i] = malloc(sizeof(cliente_t));
        clienteDados[i]->id = i;
        clienteDados[i]->maxTempoAntesDeNovoPedido = atoi(argv[5]);
        clienteDados[i]->maxTempoConsumindoBebida = atoi(argv[6]);
        pthread_create(&clientes[i], NULL, threadCliente, (void *)clienteDados[i]);
    }
    

    for (int i = 0; i < qntClientes; i++)
    {
        pthread_join(clientes[i], NULL);
        free(clienteDados[i]);
    }

    for (int i = 0; i < qntGarcons; i++)
    {
        pthread_join(garcons[i], NULL);
        // sem_destroy(&garcomDados[i].semaforoGarcom);
        free(garcomList[i]->fila);
        free(garcomList[i]);
        free(arg);
    }

    pthread_mutex_destroy(&mtx);

    printf("Fechou o bar\n");

    return 0;
}
