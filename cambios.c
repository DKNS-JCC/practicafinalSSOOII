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
struct persona // Ocupa 2 bytes
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

        finCambios();
        if (shmdt(pt) == -1)

        {
            perror("Error al liberar memoria compartida");
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
    else
    {
        if (shmdt(pt) == -1)
        {
            perror("Error al liberar memoria compartida");
        }
    }

    exit(0);
}

int main(int argc, char const *argv[])
{

    typedef struct mensaje
    {
        long tipo;
        char mensaje[100];
    } mensaje;

    mensaje msg;

    char nombres[32] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'L', 'M', 'N', 'O', 'P', 'R', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'l', 'm', 'n', 'o', 'p', 'r'};
    struct sembuf operacion[1];
    operacion[0].sem_num = 0;
    operacion[0].sem_op = 0;
    operacion[0].sem_flg = 0;

    ppid = getpid();
    char alonso = 'A'; //<-- No se usa
    int speed = 0;
    int i;
    if (argc < 2)
    {
        speed = 0;
        alarm(6);
    }
    else
    {
        speed = atoi(argv[1]);
        if (speed == 0)
        {
            alarm(20);
        }
        else
        {
            alarm(30);
        }
    }
    // MANEJADORA
    signal(SIGINT, &liberar);
    signal(SIGALRM, &liberar);

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
    ((struct grupos *)pt)->personas[8].nombre = ((struct grupos *)pt)->personas[9].nombre =
        ((struct grupos *)pt)->personas[18].nombre = ((struct grupos *)pt)->personas[19].nombre =
            ((struct grupos *)pt)->personas[28].nombre = ((struct grupos *)pt)->personas[29].nombre =
                ((struct grupos *)pt)->personas[38].nombre = ((struct grupos *)pt)->personas[39].nombre = ' ';
    inicioCambios(speed, semaforo, pt);
    i = 0;
    char nombre;

    for (i = 0; i < 32; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            nombre = nombres[i];
            break;
        }
    }

    if (pid == 0)
    {
        operacion[0].sem_op = 1;
        int pos;
        if (i < 8)
        {
            ((struct grupos *)pt)->personas[i].nombre = nombre;
            ((struct grupos *)pt)->personas[i].grupo = 1;
            pos = i;
        }
        else if (i < 16)
        {
            ((struct grupos *)pt)->personas[i + 2].nombre = nombre;
            ((struct grupos *)pt)->personas[i + 2].grupo = 2;
            pos = i + 2;
        }
        else if (i < 24)
        {
            ((struct grupos *)pt)->personas[i + 4].nombre = nombre;
            ((struct grupos *)pt)->personas[i + 4].grupo = 3;
            pos = i + 4;
        }
        else
        {
            ((struct grupos *)pt)->personas[i + 6].nombre = nombre;
            ((struct grupos *)pt)->personas[i + 6].grupo = 4;
            pos = i + 6;
        }

        semop(semaforo, operacion, 1);

        ((struct grupos *)pt)->personas[pos].grupo = aQuEGrupo(pos / 10 + 1);
    }

    else
    {
        operacion[0].sem_op = -32;
        semop(semaforo, operacion, 1);
        refrescar();
        msgrcv(buzon, &msg, sizeof(mensaje), 0, 0);
    }

    liberar();
    return 0;
}
