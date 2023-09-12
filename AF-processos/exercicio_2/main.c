#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

//                          (principal)
//                               |
//              +----------------+--------------+
//              |                               |
//           filho_1                         filho_2
//              |                               |
//    +---------+-----------+          +--------+--------+
//    |         |           |          |        |        |
// neto_1_1  neto_1_2  neto_1_3     neto_2_1 neto_2_2 neto_2_3

// ~~~ printfs  ~~~
//      principal (ao finalizar): "Processo principal %d finalizado\n"
// filhos e netos (ao finalizar): "Processo %d finalizado\n"
//    filhos e netos (ao inciar): "Processo %d, filho de %d\n"

// Obs:
// - netos devem esperar 5 segundos antes de imprmir a mensagem de finalizado (e terminar)
// - pais devem esperar pelos seu descendentes diretos antes de terminar

int main(int argc, char **argv)
{

    // ....

    /*************************************************
     * Dicas:                                        *
     * 1. Leia as intruções antes do main().         *
     * 2. Faça os prints exatamente como solicitado. *
     * 3. Espere o término dos filhos                *
     *************************************************/

    // processo principal
    for (size_t i = 0; i < 2; i++)
    {
        pid_t pid = fork();
        if (pid > 0)
        {
            printf("Processo %d, filho de %d\n", pid, getpid());
        }
        else if (pid == 0)
        {
            for (size_t j = 0; j < 3; j++)
            {
                pid_t pid2 = fork();
                if (pid2 > 0)
                {
                    printf("Processo %d, filho de %d\n", pid2, getpid());
                }
                else if (pid2 == 0)
                {
                    sleep(5);
                    printf("Processo %d finalizado\n", getpid());
                    return 0;
                }
            }
            wait(NULL);
            printf("Processo %d finalizado\n", getpid());
            return 0;
        }
    }
    wait(NULL);

    printf("Processo principal %d finalizado\n", getpid());
    return 0;
}
