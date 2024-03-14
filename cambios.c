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

    if (shmdt(pt) == -1)
    {
        perror("Error al liberar memoria compartida");
    }

    if (ppid == getpid())
    {

        for (int i = 0; i < 32; i++)
        {
            if (wait(NULL) == -1)
            {
                perror("Error en wait");
                break;
            }
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
        puts("recursos liberados");
    }
    
    exit(0);
}

int main(int argc, char const *argv[])
{
    

    struct sembuf operacion[1];
    operacion[0].sem_num = 0;
    operacion[0].sem_op = 0;
    operacion[0].sem_flg = 0;

    ppid = getpid();
    char alonso; //<-- No se usa
    int speed = 0;
    int i;
    if (argc < 2) // Si no se recibe argumento
    {
        speed = 0; // Se inicializa en 0
    }
    else
    {
        speed = atoi(argv[1]);
        if (speed == 0 && *argv[1] != '0')
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
    inicioCambios(speed, semaforo, pt);
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
        operacion[0].sem_op = 1;
        semop(semaforo, operacion, 1);
        int pos;
        if (i < 8)
        {
            ((struct grupos *)pt)->personas[i].nombre = 'A';
            ((struct grupos *)pt)->personas[i].grupo = 1;
            pos=i;
        }
        else if (i < 16)
        {
            ((struct grupos *)pt)->personas[i+2].nombre = 'B';
            ((struct grupos *)pt)->personas[i+2].grupo = 2;
            pos=i+2;
        }
        else if (i < 24)
        {
            ((struct grupos *)pt)->personas[i+4].nombre = 'C';
            ((struct grupos *)pt)->personas[i+4].grupo = 3;
            pos=i+4;
        }
        else
        {
            ((struct grupos *)pt)->personas[i+6].nombre = 'D';
            ((struct grupos *)pt)->personas[i+6].grupo = 4;
            pos=i+6;
        }
        refrescar();

        for (;;)
        {
        }
        
    }

    else
    {
        operacion[0].sem_op = -32;
        semop(semaforo, operacion, 1);
        for (;;)
        {
        }
    }

    liberar();
    return 0;
}
