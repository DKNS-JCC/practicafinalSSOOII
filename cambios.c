#include <stdio.h>
#include "cambios.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/msg.h>
int semaforo;
int mem;
void *pt;
int buzon;
int ppid;
// Structs
struct persona
{
    char nombre;
    char grupo;
};

struct grupos
{
    struct persona personas[40]; // Ocupa 80 bytes
    int vacio;                   // Ocupa 4 bytes
    int contador;                // Ocupa 4 bytes
};

void liberar()
{
    puts("Liberando recursos...");

    if (shmdt(pt) == -1)
    {
        perror("Error al liberar memoria compartida");
    }

    if (ppid == getpid())
    {

        for (int i = 0; i < 32; i++)
        {
            wait(NULL);
        }

        shmctl(mem, 0, IPC_RMID); // Liberamos memoria compartida

        if (semctl(semaforo, 0, IPC_RMID, 0) == -1) // Liberamos semáforo
        {
            perror("Error liberando semáforo");
        }
        if (msgctl(buzon, 0, IPC_RMID) == -1) // Liberamos buzon
        {
            perror("Error liberando buzon");
        }
    }

    exit(0);
}

int main(int argc, char const *argv[])
{
    ppid = getpid();
    int i = 0;
    if (argc < 2) // Si no se recibe argumento
    {
        i = 0; // Se inicializa en 0
    }
    else
    {
        i = atoi(argv[1]);
        if (i == 0 && *argv[1] != '0')
        {
            return -1;
        }
    }
    // MANEJADORA (legal usar signal)
    signal(SIGINT, &liberar);

    // Declarar IPCS

    if ((mem = shmget(IPC_PRIVATE, sizeof(struct grupos), IPC_CREAT | 0600)) == -1) // Creamos memoria compartida
    {
        perror("ERROR AL CREAR MEMORIA COMPARTIDA");
        exit(0);
    }
    pt = shmat(mem, 0, 0); // Asignamos memoria compartida

    if ((buzon = msgget(IPC_PRIVATE, IPC_CREAT | 0600)) == -1) // Creamos buzon
    {
        perror("ERROR AL CREAR BUZON");
        // liberar lo creado
        shmdt(pt);
        shmctl(mem, 0, IPC_RMID);
        exit(0);
    }

    if ((semaforo = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600)) == -1) // Creamos semaforo
    {
        perror("ERROR AL CREAR SEMAFORO");
        // liberar lo creado
        shmdt(pt);
        shmctl(mem, 0, IPC_RMID);
        msgctl(buzon, 0, IPC_RMID);
        exit(0);
    }

    semctl(semaforo, 0, SETVAL, 0); // Inicializamos semaforo

    pid_t pid;
    i = 0;
    for (i = 0; i < 32; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            break;
        }
    }

    if (pid == 0)
    {
        for (;;)
        {
        }
    }

    else
    {
        inicioCambios(i, semaforo, pt);
        for (;;)
        {
        }
    }

    liberar();
    return 0;
}
