#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <pthread.h>

#define SHMKEY ((key_t)7890)
#define SEMKEY ((key_t)400L)
#define NSEMS 2
#define PORTNUM 3440       /* the port number the server will listen to*/
#define DEFAULT_PROTOCOL 0 /*constant for default protocol*/

// global mutexes for threads
pthread_mutex_t mutex_client1, mutex_client2;
int OTHER_CLIENT_WORKING = 0;

typedef struct
{
    char letters[36];
    char values[36];
    int socketID[2];
    int numClients;
    int displayed;
    int recieved;
} shared_mem;
shared_mem *data;

union {
    int val;
    struct semid_ds *buf;
    ushort *array;
} semctl_arg;

static struct sembuf Wait = {0, -1, 0};
static struct sembuf Signal = {0, 1, 0};

int ready1, ready2, numReady, status;
int semnum, semval;

void doprocessing(int sock);
void game(int sock);

void *client1_execution(void *args) {
    pthread_mutex_lock(&mutex_client1);

    if (OTHER_CLIENT_WORKING == 0) {
        OTHER_CLIENT_WORKING == 1;


        OTHER_CLIENT_WORKING == 0;
    }
    else
    {
        // im not sure... do we print and let them do stuff? 
        // prob use a semaphore instead of global var... that way it waits and
        // still executes the code (write, read, etc), need to make sure
        // we send the letter array to both clients data->socketID[0] and 
        // data->socketID[1] after reading chosen letter from ONE client
    }
    

    pthread_mutex_unlock(&mutex_client2);
    pthread_exit(0); // exit client 1 thread
}

void *client2_execution(void *args) {
    pthread_mutex_lock(&mutex_client2);

    if (OTHER_CLIENT_WORKING == 0) {

    }
}

int main(int argc, char *argv[])
{

    numReady = semget(SEMKEY, NSEMS, IPC_CREAT | 0660);
    if (numReady < 0)
    {
        printf("error in creating numReady");
        exit(1);
    }
    semnum = 0;
    semctl_arg.val = 0;
    status = semctl(numReady, semnum, SETVAL, semctl_arg);

    key_t key = 123; /* shared memory key */
    int shmid;
    char *shmadd;
    shmadd = (char *)0;
    if ((shmid = shmget(SHMKEY, sizeof(int), IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        exit(1);
    }

    if ((data = (shared_mem *)shmat(shmid, shmadd, 0)) == (shared_mem *)-1)
    {
        perror("shmat");
        exit(0);
    }

    char newLetters[36] = "a b c d \ne f g h \ni j k l \nm n o p\n";
    char letterValues[36] = "2 4 6 8 \n1 3 5 7 \n9 7 5 3 \n4 8 5 7\n";

    strcpy(data->letters, newLetters);
    strcpy(data->values, letterValues);
    data->numClients = 0;
    data->displayed = 0;
    data->recieved = 0;
    data->socketID[0] = 0;
    data->socketID[1] = 0;

    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int status, pid;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);

    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = PORTNUM;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/

    status = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    if (status < 0)
    {
        perror("ERROR on binding");
        exit(1);
    }

    // making threads
    pthread_t thread_client1, thread_client2;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    pthread_mutex_init(&mutex_client1, NULL);
    pthread_mutex_init(&mutex_client2, NULL);

    pthread_mutex_lock(&mutex_client1);
    pthread_mutex_lock(&mutex_client2);

    pthread_create(&thread_client1, &attr, client1_execution, NULL);
    pthread_create(&thread_client2, &attr, client2_execution, NULL);

    /* Now Server starts listening clients wanting to connect. No 	more than 5 clients allowed */
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

        if (data->socketID[0] == 0)
        {
            data->socketID[0] = newsockfd;
        }
        else
        {
            data->socketID[1] = newsockfd;
        }
        if (newsockfd < 0)
        {
            perror("ERROR on accept");
            exit(1);
        }

        // /* Create child process */
        pid = fork();

        // create new thread
        pthread_join(thread_client1, NULL);

        if (pid < 0)
        {
            perror("ERROR on fork");
            exit(1);
        }

        if (pid == 0)
        {
            /* This is the client process */
            close(sockfd);
            doprocessing(newsockfd);
            exit(0);
        }
        else
        {
            close(newsockfd);
        }

    } /* end of while */

    /* the next set of statements releases the shared memory */
    if ((shmctl(shmid, IPC_RMID, (struct shmid_ds *)0)) == -1)
    {
        perror("shmctl");
        exit(-1);
    }
    printf("Shared memory has been released.\n");

    /* this releases the semaphore numbered "0" the first in the set*/
    semctl_arg.val = 0;
    semnum = 0;
    status = semctl(numReady, semnum, IPC_RMID, semctl_arg);
    if (status < 0)
    {
        printf("error in releasing semaphore");
        exit(1);
    }
}

void doprocessing(int sock)
{
    int status;
    char buffer[256];
    int cont = 1;
    while (cont)
    {
        bzero(buffer, 256);
        status = read(sock, buffer, 255);
        if (status < 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }

        if (strcmp(buffer, "ready\n") == 0)
        {
            data->numClients++;
            if (data->numClients < 2)
            {
                status = write(sock, "Waiting on other client.\n", 26);
            }
            if (data->numClients == 2)
            {
                status = semop(numReady, &Signal, 1);
            }

            status = semop(numReady, &Wait, 1);
            status = write(sock, "Sending Letters. Good luck.\n", 29);
            status = semop(numReady, &Signal, 1);

            status = semop(numReady, &Wait, 1);
            status = write(sock, data->letters, strlen(data->letters));
            status = semop(numReady, &Signal, 1);

            game(sock);
        }
        else if ((strcmp(buffer, "exit\n") == 0))
        {
            status = write(sock, "Goodbye.\n", 10);
            return;
        }
        else
        {
            status = write(sock, "To play the game reconnect and send \"ready\"\n", 45);
            return;
        }
    }
}

void game(int sock)
{
    if (data->displayed == 0)
    {
        printf("\nLetters:\n");
        printf(data->letters);
        printf("\nValues:\n");
        printf(data->values);
        data->displayed = 1;
    }
    char buffer[256];

    int i, found = 0;
    while (data->recieved < 16)
    {
        bzero(buffer, 256);
        status = read(sock, buffer, 255); //recieving letter from client
        if (status < 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }
        printf("Recieved: %c", buffer[0]);
        for (i = 0; i < strlen(data->letters); i++)
        {
            if (data->letters[i] == buffer[0])
            {
                data->letters[i] = '-';
                // int x = data->values[i] - '0'; //this will convert the char to an int of that value
                data->recieved++;
                found = 1;
            }
        }
        if (!found)
        {
            printf("The recieved letter %c was not found.", buffer[0]);
        }
        status = write(data->socketID[0], data->letters, strlen(data->letters));
        status = write(data->socketID[1], data->letters, strlen(data->letters));
    }
}
