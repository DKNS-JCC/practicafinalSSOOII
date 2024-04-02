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
#include <errno.h>

int semaforo;
int mem;
void *pt;
int buzon;
int pids[33];

void liberar();

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
    if (pids[32] == getpid())
    {
        
        for (int i = 0; i < 32; i++)
        {
            kill(pids[i], SIGINT);
        }
        for (int i = 0; i < 32; i++)
        {
            if (wait(NULL) == -1)
            {
                perror("Error en wait");
            }
        }
        refrescar();
        printf("%d", finCambios());

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

//==============================================================================
int main(int argc, char const *argv[])
{

    typedef struct mensaje
    {
        long tipo;
        int origen;
        int destino;
    } mensaje;

    mensaje msg;

    char nombres[32] = {'A', 'B', 'C', 'D', 'a', 'b', 'c', 'd', 'E', 'F', 'G', 'H', 'e', 'f', 'g', 'h', 'I', 'J', 'L', 'M', 'i', 'j', 'l', 'm', 'N', 'O', 'P', 'R', 'n', 'o', 'p', 'r'};
    struct sembuf operacion[1];
    operacion[0].sem_num = 0;
    operacion[0].sem_op = 0;
    operacion[0].sem_flg = 0;

    struct sembuf wait[1];
    wait[0].sem_num = 0;
    wait[0].sem_op = -1;
    wait[0].sem_flg = 0;

    struct sembuf singal[1];
    singal[0].sem_num = 0;
    singal[0].sem_op = 1;
    singal[0].sem_flg = 0;

    pids[32] = getpid();
    char alonso = 'A'; //<-- No se usa
    int speed = 0;
    int i;
    if (argc < 2)
    {
        speed = 0;
        alarm(20);
    }
    else
    {
        speed = atoi(argv[1]);
        if (speed == 0)
        {
            alarm(30);
        }
        else
        {
            alarm(30);
        }
    }

    // Manejadoras
    signal(SIGINT, &liberar);
    signal(SIGALRM, &liberar);

    // Declarar IPCS
    if ((mem = shmget(IPC_PRIVATE, sizeof(struct grupos), IPC_CREAT | 0600)) == -1) // Creamos memoria compartida
    {
        perror("ERROR AL CREAR MEMORIA COMPARTIDA");
        exit(0);
    }

    // Asignamos memoria compartida
    pt = shmat(mem, 0, 0);

    // Creamos buzon
    if ((buzon = msgget(IPC_PRIVATE, IPC_CREAT | 0600)) == -1)
    {
        perror("ERROR AL CREAR BUZON");

        shmdt(pt);
        shmctl(mem, 0, IPC_RMID);
        exit(0);
    }

    if ((semaforo = semget(IPC_PRIVATE, 5, IPC_CREAT | 0600)) == -1) // Creamos semaforo
    {
        perror("ERROR AL CREAR SEMAFORO");

        shmdt(pt);
        shmctl(mem, 0, IPC_RMID);
        msgctl(buzon, 0, IPC_RMID);
        exit(0);
    }

    // Inicializamos semaforo
    semctl(semaforo, 0, SETVAL, 0);
    semctl(semaforo, 1, SETVAL, 0);
    semctl(semaforo, 2, SETVAL, 0);
    semctl(semaforo, 3, SETVAL, 0);
    semctl(semaforo, 4, SETVAL, 1);

    pid_t pid;

    ((struct grupos *)pt)->personas[8].nombre = ((struct grupos *)pt)->personas[9].nombre =
        ((struct grupos *)pt)->personas[18].nombre = ((struct grupos *)pt)->personas[19].nombre =
            ((struct grupos *)pt)->personas[28].nombre = ((struct grupos *)pt)->personas[29].nombre =
                ((struct grupos *)pt)->personas[38].nombre = ((struct grupos *)pt)->personas[39].nombre = ' ';

    inicioCambios(speed, semaforo, pt);
    i = 0;
    char nombre;

    // Crear procesos hijos
    for (i = 0; i < 32; i++)
    {
        pids[i] = fork();
        if (pids[i] == 0)
        {
            nombre = nombres[i];
            break;
        }
    }

    // Asignar grupos y nombres
    if (pids[32] != getpid())
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
        while (1)
        {
            ((struct grupos *)pt)->personas[pos].grupo = aQuEGrupo(pos / 10 + 1);
            msg.origen = pos / 10;
            msg.destino = ((struct grupos *)pt)->personas[pos].grupo - 1;

            //ESTO SUSTITUYE AL MEGA SWITCH
            msg.tipo=msg.origen*10+msg.destino;
            
            // Enviar mensaje solicitando cambio
            if (msgsnd(buzon, &msg, sizeof(struct mensaje) - sizeof(long), IPC_NOWAIT) == -1)
            {
                perror("Error al enviar el mensaje");
                exit(1);
            }
            // Recibir mensaje bloqueante
            if (msgrcv(buzon, &msg, sizeof(struct mensaje) - sizeof(long), msg.tipo + 100, 0) == -1)
            {
                perror("Error al recibir el mensaje");
                exit(1);
            }
            else
            {
                wait[0].sem_num = ((struct grupos *)pt)->personas[pos].grupo - 1;
                singal[0].sem_num = ((struct grupos *)pt)->personas[pos].grupo - 1;
                semop(semaforo, wait, 1);
                for (int i = ((struct grupos *)pt)->personas[pos].grupo * 10 - 10; i < ((struct grupos *)pt)->personas[pos].grupo * 10; i++)
                {
                    if (((struct grupos *)pt)->personas[i].nombre == ' ')
                    {
                        ((struct grupos *)pt)->personas[i].nombre = nombre;
                        ((struct grupos *)pt)->personas[i].grupo = ((struct grupos *)pt)->personas[pos].grupo;
                        ((struct grupos *)pt)->personas[pos].nombre = ' ';
                        pos = i;
                        break;
                    }
                }
                semop(semaforo, singal, 1);
                
                wait[0].sem_num = 4;
                singal[0].sem_num = 4;
                semop(semaforo, wait, 1);
                incrementarCuenta();
                ((struct grupos *)pt)->contador++;
                semop(semaforo, singal, 1);
            }
        }
    }
    else // Proceso padre
    {
        operacion[0].sem_op = -32;
        semop(semaforo, operacion, 1);
        refrescar();
        operacion[0].sem_op = 1;
        //Iniciar semaforos a 1
        for (int i = 0; i < 4; i++)
        {
            operacion[0].sem_num = i;
            semop(semaforo, operacion, 1);
        }
        int solicitudes[4][4];
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                solicitudes[i][j] = 0;
            }
        }
        int contador=0;
        int multiple[4]={0,0,0,0};
        int fila=0;
        int bandera = 0;
        while (1)
        {
            refrescar();
            msgrcv(buzon, &msg, sizeof(mensaje) - sizeof(long), 0, 0);
            
            if (solicitudes[msg.destino][msg.origen] != 0)
            {
               
                msg.tipo += 100;
                msgsnd(buzon, &msg, sizeof(mensaje) - sizeof(long), IPC_NOWAIT);

                solicitudes[msg.destino][msg.origen]--;
                msg.tipo = msg.destino * 10 + msg.origen + 100;
                msgsnd(buzon, &msg, sizeof(mensaje) - sizeof(long), IPC_NOWAIT);
            }
            else
            {
                solicitudes[msg.origen][msg.destino]++;
                fila = msg.origen;
                while (contador < 4)
                {
                    for (int j = 0; j < 4; j++)
                    {
                        
                        if (solicitudes[fila][j] != 0)
                        {
                            multiple[contador] = fila * 10 + j;
                            fila = j;
                            if (j == msg.origen)
                            {
                                bandera = 0;
                            }
                            break;
                        }
                    }
                    if (bandera == 0)
                    {
                        break;
                    }
                    contador++;
                }
                if (bandera == 0)
                {
                    for (int i = 0; i <= contador; i++)
                    {
                        msg.tipo = multiple[i]+100;
                        solicitudes[multiple[i] / 10][multiple[i] % 10]--;
                        msgsnd(buzon, &msg, sizeof(mensaje) - sizeof(long), IPC_NOWAIT);
                    }
                }
            }
            bandera = 1;
            contador = 0;
        }
    }

    liberar();
    return 0;
}
