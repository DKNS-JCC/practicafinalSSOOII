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

 struct utilidades {
    int semaforo;
    int mem;
    void *pt;
    int buzon;
    int pids[33];
    int flag;
};
struct utilidades u;

union semun
{
    int val;
    struct semid_ds *buf;
    ushort_t *array;
};

void liberar();
void alarmhandler();

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


void alarmhandler(){
    int i;
    for (i = 0; i < 32; i++)
        {
            kill(u.pids[i], SIGUSR1);
        }
        liberar();
}

void liberar()
{

    if (u.pids[32] == getpid())
    {
        struct sembuf singal[1];
        singal[0].sem_num = 0;
        singal[0].sem_op = 32;
        singal[0].sem_flg = 0;
        semop(u.semaforo, singal, 1);
        
        int i;
        for (i = 0; i < 32; i++)
        {
            if (wait(NULL) == -1)
            {
                perror("Error en wait");
            }
        }
        refrescar();
        finCambios();

        if (shmdt(u.pt) == -1)
        {
            perror("Error al liberar memoria compartida");
        }

        shmctl(u.mem, IPC_RMID, NULL); // Liberamos memoria compartida

        if (semctl(u.semaforo, 0, IPC_RMID, 0) == -1) // Liberamos semáforo
        {
            perror("Error liberando semáforo");
        }
        if (msgctl(u.buzon, IPC_RMID, NULL) == -1) // Liberamos buzon
        {
            perror("Error liberando buzon");
        }
        exit(0);
    }
    else
    {
        if (u.flag == 1)
        {
            if (shmdt(u.pt) == -1)
            {
                perror("Error al liberar memoria compartida");
            }
            exit(0);
        }
        else
        {
            u.flag=0;
        }
    }
    
}

//==============================================================================

int main(int argc, char const *argv[])
{
    union semun arg;

    u.flag = 1;

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

    u.pids[32] = getpid();
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
    signal(SIGALRM, &alarmhandler);
    signal(SIGUSR1,&liberar);

    // Declarar IPCS
    if ((u.mem = shmget(IPC_PRIVATE, sizeof(struct grupos), IPC_CREAT | 0600)) == -1) // Creamos memoria compartida
    {
        perror("ERROR AL CREAR MEMORIA COMPARTIDA");
        exit(0);
    }

    // Asignamos memoria compartida
    u.pt = shmat(u.mem, 0, 0);

    // Creamos buzon
    if ((u.buzon = msgget(IPC_PRIVATE, IPC_CREAT | 0600)) == -1)
    {
        perror("ERROR AL CREAR BUZON");

        shmdt(u.pt);
        shmctl(u.mem, IPC_RMID, NULL);
        exit(0);
    }

    if ((u.semaforo = semget(IPC_PRIVATE, 7, IPC_CREAT | 0600)) == -1) // Creamos semaforo
    {
        perror("ERROR AL CREAR SEMAFORO");

        shmdt(u.pt);
        shmctl(u.mem, IPC_RMID, NULL);
        msgctl(u.buzon, IPC_RMID, NULL);
        exit(0);
    }

    // Inicializamos semaforo

    arg.val = 0;
    semctl(u.semaforo, 1, SETVAL, arg);
    semctl(u.semaforo, 2, SETVAL, arg);
    semctl(u.semaforo, 3, SETVAL, arg);
    semctl(u.semaforo, 4, SETVAL, arg);
    semctl(u.semaforo, 6, SETVAL, arg);
    arg.val = 1;
    semctl(u.semaforo, 5, SETVAL, arg);
    

    pid_t pid;

    // Inicializar grupos
    ((struct grupos *)u.pt)->contador = 0;

    ((struct grupos *)u.pt)->personas[8].nombre = ((struct grupos *)u.pt)->personas[9].nombre =
        ((struct grupos *)u.pt)->personas[18].nombre = ((struct grupos *)u.pt)->personas[19].nombre =
            ((struct grupos *)u.pt)->personas[28].nombre = ((struct grupos *)u.pt)->personas[29].nombre =
                ((struct grupos *)u.pt)->personas[38].nombre = ((struct grupos *)u.pt)->personas[39].nombre = 32;
    
    inicioCambios(speed, u.semaforo, u.pt);

    i = 0;
    char nombre;

    // Crear procesos hijos
    for (i = 0; i < 32; i++)
    {
        u.pids[i] = fork();
        if (u.pids[i] == 0)
        {
            nombre = nombres[i];
            break;
        }
    }

    // Asignar grupos y nombres
    if (u.pids[32] != getpid())
    {

        int pos;
        if (i < 8)
        {
            ((struct grupos *)u.pt)->personas[i].nombre = nombre;
            ((struct grupos *)u.pt)->personas[i].grupo = 1;
            pos = i;
        }
        else if (i < 16)
        {
            ((struct grupos *)u.pt)->personas[i + 2].nombre = nombre;
            ((struct grupos *)u.pt)->personas[i + 2].grupo = 2;
            pos = i + 2;
        }
        else if (i < 24)
        {
            ((struct grupos *)u.pt)->personas[i + 4].nombre = nombre;
            ((struct grupos *)u.pt)->personas[i + 4].grupo = 3;
            pos = i + 4;
        }
        else
        {
            ((struct grupos *)u.pt)->personas[i + 6].nombre = nombre;
            ((struct grupos *)u.pt)->personas[i + 6].grupo = 4;
            pos = i + 6;
        }
        singal[0].sem_num=6;
        semop(u.semaforo, singal, 1);
        int i;
        while (u.flag)
        {
            u.flag=1;
            
            ((struct grupos *)u.pt)->personas[pos].grupo = aQuEGrupo(pos / 10 + 1);
            
            msg.origen = pos / 10;
            msg.destino = ((struct grupos *)u.pt)->personas[pos].grupo - 1;


            msg.tipo = msg.origen * 10 + msg.destino;


            if (msgsnd(u.buzon, &msg, sizeof(struct mensaje) - sizeof(long), IPC_NOWAIT) == -1)
            {
                perror("Error al enviar el mensaje");
                exit(1);
            }
            // Recibir mensaje bloqueante
            
            if (msgrcv(u.buzon, &msg, sizeof(struct mensaje) - sizeof(long), msg.tipo + 100, 0) == -1)
            {
                perror("Error al recibir el mensaje");
                exit(1);
            }
            
            else
            {

                u.flag=2;
                wait[0].sem_num = ((struct grupos *)u.pt)->personas[pos].grupo;
                singal[0].sem_num = ((struct grupos *)u.pt)->personas[pos].grupo;
                semop(u.semaforo, wait, 1);
                
                for (i = ((struct grupos *)u.pt)->personas[pos].grupo * 10 - 10; i < ((struct grupos *)u.pt)->personas[pos].grupo * 10; i++)
                {
                    if (((struct grupos *)u.pt)->personas[i].nombre == 32)
                    {
                        ((struct grupos *)u.pt)->personas[i].nombre = nombre;
                        ((struct grupos *)u.pt)->personas[i].grupo = ((struct grupos *)u.pt)->personas[pos].grupo;
                        ((struct grupos *)u.pt)->personas[pos].nombre = 32;
                        pos = i;
                        break;
                    }
                }
                semop(u.semaforo, singal, 1);

                wait[0].sem_num = 5;
                singal[0].sem_num = 5;
                semop(u.semaforo, wait, 1);
                incrementarCuenta();
                ((struct grupos *)u.pt)->contador++;
                semop(u.semaforo, singal, 1);
                
            }
        }
    }
    else // Proceso padre
    {
        int i,j;
        operacion[0].sem_op = -32;
        operacion[0].sem_num = 6;
        semop(u.semaforo, operacion, 1);
        refrescar();
        
        // Iniciar semaforos a 1
        for (i = 0; i < 4; i++)
        {
            singal[0].sem_num = i + 1;
            semop(u.semaforo, singal, 1);
        }
        int solicitudes[4][4];
        for (i = 0; i < 4; i++)
        {
            for (j = 0; j < 4; j++)
            {
                solicitudes[i][j] = 0;
            }
        }
        int contador = 0;
        int multiple[4] = {0, 0, 0, 0};
        int fila = 0;
        int bandera = 1;
        while (u.flag)
        {
            
            refrescar();
            msgrcv(u.buzon, &msg, sizeof(mensaje) - sizeof(long), 0, 0);

            if (solicitudes[msg.destino][msg.origen] != 0)
            {

                msg.tipo += 100;
                msgsnd(u.buzon, &msg, sizeof(mensaje) - sizeof(long), IPC_NOWAIT);

                solicitudes[msg.destino][msg.origen]--;
                msg.tipo = msg.destino * 10 + msg.origen + 100;
                msgsnd(u.buzon, &msg, sizeof(mensaje) - sizeof(long), IPC_NOWAIT);
            }
            else
            {
                solicitudes[msg.origen][msg.destino]++;
                fila = msg.origen;
                while (contador < 4)
                {
                    for (j = 0; j < 4; j++)
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
                    for (i = 0; i <= contador; i++)
                    {
                        msg.tipo = multiple[i] + 100;
                        solicitudes[multiple[i] / 10][multiple[i] % 10]--;
                        msgsnd(u.buzon, &msg, sizeof(mensaje) - sizeof(long), IPC_NOWAIT);
                    }
                }
            }
            bandera = 1;
            contador = 0;
        }
    }
    u.flag=1;
    liberar();
    return 0;
}