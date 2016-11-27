#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define TRUE   1
#define FALSE  0
#define PORT 5550

void ToUpperString(char input[])
{
    int i;
    for(i=0; i<strlen(input); i++) {
        input[i] = toupper(input[i]);
    }
}

int main()
{
    int opt = TRUE;
    int listen_sock;
    int addrlen;
    int new_sock;
    int client_sock[30];
    int max_clients = 30;
    int activity;
    int i;
    int sd;
    int max_sd;
    struct sockaddr_in address;
    char *buffer;
    int bytes_sent,bytes_received;
    char recv_data[1024];
    int sin_size;

    fd_set readfds;

    char *message = "Welcome to the server \r\n";

    for (i = 0; i < max_clients; i++)
    {
        client_sock[i] = 0;
    }

    if( (listen_sock = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if( setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to localhost port 8888
    if (bind(listen_sock, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    if (listen(listen_sock, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    puts("Waiting  ...");

    while(1)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(listen_sock, &readfds);
        max_sd = listen_sock;

        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++)
        {
            //socket descriptor
            sd = client_sock[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select(max_sd + 1,&readfds,NULL,NULL,NULL);
        if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(listen_sock, &readfds))
        {
            if ((new_sock = accept(listen_sock, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_sock , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

            //send new connection greeting message
            if( send(new_sock, message, strlen(message), 0) != strlen(message) )
            {
                perror("send");
            }

            puts("Welcome message sent successfully");

            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                //if position is empty
                if( client_sock[i] == 0 )
                {
                    client_sock[i] = new_sock;
                    printf("Adding to list of sockets as %d\n" , i);

                    break;
                }
            }
        }

        //else its some IO operation on some other socket :)
        for (i = 0; i < max_clients; i++)
        {
            sd = client_sock[i];

            if (FD_ISSET( sd , &readfds))
            {
        bytes_received = recv(sd,recv_data,1024,0);
        if (bytes_received < 0) {
            printf("Error! Can not receive data from client!\n");
            close(sd);
            exit(-1);
        }
        recv_data[bytes_received] = '\0';

    printf("received message from client on socket %d \n", sd);
    puts(recv_data);
    // Get the upcased-string in buffer, send back to user

                close(sd);
                client_sock[i]=0;
            }

    }
    }

    close(listen_sock);
    return 0;
}
