//Andrew Fernandez
//Anthony Reveron
//Matthew Sutton

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <pthread.h>

#define PORTNUM 3441       /* the port number that the server is listening to*/
#define DEFAULT_PROTOCOL 0 /*constant for default protocol*/

char endString[36] = "- - - - \n- - - - \n- - - - \n- - - -\n";

void main()

{
    int port;
    int socketid;     /*will hold the id of the socket created*/
    int status;       /* error status holder*/
    char buffer[256]; /* the message buffer*/
    int cont = 1;
    int pos = 1;
    struct sockaddr_in serv_addr;

    struct hostent *server;

    /* this creates the socket*/
    socketid = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
    if (socketid < 0)
    {
        printf("error in creating client socket\n");
        exit(-1);
    }

    server = gethostbyname("osnode05");

    if (server == NULL)
    {
        printf(" error trying to identify the machine where the server is running\n");
        exit(0);
    }

    port = PORTNUM;
    /*This function is used to initialize the socket structures with null values. */
    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(port);

    /* connecting the client socket to the server socket */

    status = connect(socketid, (struct sockaddr *)&serv_addr,
                     sizeof(serv_addr));
    if (status < 0)
    {
        printf(" error in connecting client socket with server	\n");
        exit(-1);
    }
RESETGAME:

    printf("Enter \"ready\" to play the game or \"exit\" to exit.\n");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);
    status = write(socketid, buffer, strlen(buffer));
    if (status < 0)
    {
        printf("error while sending client message to server\n");
    }

    if (strcmp(buffer, "exit\n") == 0)
    {
        close(socketid);
    }
    else if (strcmp(buffer, "ready\n") == 0)
    {
        bzero(buffer, 256);
        status = read(socketid, buffer, 255); //either "Waiting on other client." or "Sending Letters. Good luck."
        printf(buffer);

        if (status < 0)
        {
            printf("error while reading message from server");
            exit(1);
        }
        if (strcmp(buffer, "Waiting on other client.\n") == 0)
        {
            bzero(buffer, 256);
            status = read(socketid, buffer, 255); //reading "Sending letters"
            printf(buffer);
            if (status < 0)
            {
                printf("error while reading message from server");
                exit(1);
            }
        }

        bzero(buffer, 256);
        status = read(socketid, buffer, 255); // Getting letters here
        if (status < 0)
        {
            printf("error while reading message from server");
            exit(1);
        }
        printf("\nChoose a letter: \n%s\n", buffer); //printing letters here
        int recieved = 0;
        while (cont)
        {
            bzero(buffer, 256);
            fgets(buffer, 255, stdin);
            //input validation
            if (buffer[0] == '\n')
                continue;

            status = write(socketid, buffer, 255); //sending chosen letter
            if (strcmp(buffer, "exit\n") == 0)
            {
                break;
            }
            bzero(buffer, 256);
            status = read(socketid, buffer, 255); //this gets the letter found message from the p selection 
            printf(buffer);
            bzero(buffer, 256);
            status = read(socketid, buffer, 255); // Getting letters here // This gets the empty string and the do you want to play again
            if (!strcmp(buffer, endString) || status == 0)
            {
                printf("Game over.\n");
                break;
            }
            printf("\nChoose a letter: \n%s\n", buffer); //printing letters here
        }
        status = read(socketid, buffer, 255); // Would you like to play again?
        printf(buffer);
        bzero(buffer, 256);
        fgets(buffer, 255, stdin); //y or n

        //this write never happens
        status = write(socketid, buffer, 255); //sending chosen letter

        if (!strcmp(buffer, "Y\n") || !strcmp(buffer, "y\n"))
        {
            goto RESETGAME;
        }
    }
    else
    {
        printf("To play the game reconnect and send \"ready\"\n");
    }
    close(socketid);
}