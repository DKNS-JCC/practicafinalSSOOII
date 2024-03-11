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

//Structs
struct persona{
    char nombre;
    char grupo;
};

struct grupos{
    struct persona personas[40];    //Ocupa 80 bytes
    int vacio;                      //Ocupa 4 bytes
    int contador;                   //Ocupa 4 bytes
};

void liberar()
{
    puts("Liberando recursos...");
    //Desvincular memoria compartida
    if (shmdt(pt) == -1)
    {
        perror("Error al liberar memoria compartida");
    }
    wait(NULL); //Esperamos a que todos los hijos terminen
    shmctl(mem, 0, IPC_RMID); //Liberamos memoria compartida
    
    if (semctl(semaforo, 0, IPC_RMID, 0) == -1) //Liberamos semáforo
    {
        perror("Error liberando semáforo");
    }
    if (msgctl(buzon, 0, IPC_RMID) == -1) //Liberamos buzon
    {
        perror("Error liberando buzon");
    }
    exit(0);
}

int main(int argc, char const *argv[])
{
    //COMPROBACION ENTRADA
    int i = 0;
    if (argc < 2) //Si no se recibe argumento
    {
        i = 0;    //Se inicializa en 0
    }
    else
    {
        i = atoi(argv[1]);
        if (i == 0 && *argv[1] != '0')
        {
            return -1;
        }
    }
    //MANEJADORA (legal usar signal)
    signal(SIGINT, &liberar);

    //CREACION RECURSOS COMPARTIDOS
    int mem = shmget(IPC_PRIVATE, sizeof(struct grupos), IPC_CREAT | 0666);
    void *pt = shmat(mem, 0, 0);
    buzon = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    if ((semaforo = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600)) == -1)
    {
        printf("ERROR AL CREAR SEMAFORO");
        exit(0);
    }

    semctl(semaforo, 0, SETVAL, 0);
    pid_t pid=1;
    i=0;    
    while(i<32&&pid!=0){
        pid=fork();
        i++;
    }

    if (pid == 0)
    {
        printf("hijo %d",i);
        
    }

    else
    {
        sleep(1);
        printf("%s \n", (char *)pt);
        inicioCambios(i, semaforo, pt);
        for(;;){

        }
    }

    return 0;
}
