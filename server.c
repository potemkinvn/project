#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#include "linkedlist.h"

#define TRUE   1
#define FALSE  0
#define PORT 5550

typedef struct message {
    int command;
    char message[1024];
} message;

typedef struct player {
    char    username[20];
    int     sockdes;
    struct  player *opponent;
    int     challenge;
    int     isAvailable; // 0: busy; 1: available
    int     isLogged; // 0: not logged, 1: logged
} playerType;

void myFlush()
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

void GetUserInfo(FILE *input, node **top)
{
    int tmp;
    elementType el;
    char name[20];
    char tmp2[20];
    while(!feof(input)) {
        tmp = fscanf(input, "%s %s\n", name, tmp2);
        strcpy(el.name, name);
        el.pass = atoi(tmp2);

        if(tmp>0)
            AddTop(top,el);
    }
}

node* FindNodeOnUsername(node **top, char buff[])
{
    node *tmp = *top;
    while(tmp!=NULL) {
        if(strcmp(tmp->element.name,buff)==0)
            return tmp;
        else
            tmp = tmp->pNext;
    }
    return tmp;
}

message ParseMessage(char recv_data[])
{
    /// parse command and command content
    message ms;
    char tmp[3];

    strncpy(tmp, recv_data, 3);
    tmp[3] = '\0';
    ms.command = atoi(tmp);

    strncpy(ms.message, &recv_data[6], sizeof(ms.message));

    return ms;
}

playerType FindPlayerOnUsername(char username[], playerType player[])
{
    playerType not_exist_player;
    not_exist_player.sockdes = -9999;
    int i;
    for(i=0; i<30; i++) {
        if(strcmp(username,player[i].username) && (player[i].isAvailable == 1) ) {
            return player[i];
        }
    }
    return not_exist_player;
}

void PrintPlayerType(playerType player)
{
    printf("Username: %s, sockdes: %d, opponent: %d, challenge: %d, status: %d",\
           player.username, player.sockdes, player.opponent, player.challenge, player.isAvailable);
}

char ListAllPlayersInUsernameAndStatus(playerType player[])
{
    int i = 0;
    char result[1000];
    char tmp[100];
    for(i = 0; i<30; i++) {
        sprintf(tmp,"%s %d\n", player[i].username, player[i].isAvailable);
        strcat(result, tmp);
    }
    printf("%s",result);
    return result;
}

int main()
{
    node*       top = NULL;
    node*       user = NULL;
    int opt = TRUE;
    int listen_sock;
    int addrlen;
    int new_sock;
    int client_sock[30];
    playerType player[30];
    int max_clients = 30;
    int activity;
    int i;
    int sd;
    int max_sd;
    struct sockaddr_in address;
    char *buffer;
    int bytes_sent,bytes_received;
    char buff[1024];
    int sin_size;
    message ms;

    fd_set readfds;

    int challenge;

    // get users info
    FILE *input = fopen("users","r");
    if(!input) {
        printf("Can't read users info from file.\n");
        exit(-1);
    }
    GetUserInfo(input,&top);
    fclose(input);

    for (i = 0; i < max_clients; i++) {
        player[i].sockdes = 0;
        strcpy(player[i].username, "");
        player[i].challenge = -9999;
        player[i].isAvailable = 0;
        player[i].opponent = NULL;
    }

    if( (listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if( setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to localhost port 8888
    if (bind(listen_sock, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    if (listen(listen_sock, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    puts("Waiting  ...");

    while(1) {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(listen_sock, &readfds);
        max_sd = listen_sock;

        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++) {
            //socket descriptor
            sd = player[i].sockdes;

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd, &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select(max_sd + 1,&readfds,NULL,NULL,NULL);
        if ((activity < 0) && (errno!=EINTR)) {
            printf("select error");
        }

        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(listen_sock, &readfds)) {
            if ((new_sock = accept(listen_sock, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n", new_sock, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            char *message = "Welcome to the server\n";
            //send new connection greeting message
            if( send(new_sock, message, strlen(message), 0) != strlen(message) ) {
                perror("send");
            }

            puts("Welcome message sent successfully");

            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++) {
                //if position is empty
                if( player[i].sockdes == 0 ) {
                    player[i].sockdes = new_sock;
                    printf("Adding to list of sockets as %d\n", i);

                    break;
                }
            }
        }

        //else its some IO operation on some other socket :)
        for (i = 0; i < max_clients; i++) {
            sd = player[i].sockdes;

            if (FD_ISSET( sd, &readfds)) {
                bytes_received = recv(sd,buff,1024,0);
                if (bytes_received < 0) {
                    printf("Error! Can not receive data from client!\n");
                    close(sd);
                    exit(-1);
                }
                printf("Received message from client on socket %d \n", sd);

                ms = ParseMessage(buff);

                switch (ms.command) {
                /// Login
                case 100:
                case 101:
                case 102:
                    /// recv username
                    printf("Input - Command: %d ~ Message: %s\n", ms.command, ms.message);
                    user = FindNodeOnUsername(&top, ms.message);
                    if(user==NULL) {
                        ms.command++;
                        sprintf(buff, "%d", ms.command);
                        bytes_sent = send(sd,buff,sizeof(buff),0);
                        if (bytes_sent < 0) {
                            printf("Error! Can not sent data to socket %d \n", sd);
                            close(sd);
                            player[i].sockdes = 0;
                            break;
                        }
                        printf("Output - Command: %d\n", ms.command);
                    } else {
                        // send challenge to user
                        ms.command = 104;
                        srand(time(NULL));
                        int r = rand() % 20;
                        challenge = r;
                        printf("Output - Command: %d ~ Message: %d\n", ms.command, r);
                        sprintf(buff,"%d ~ %d",ms.command,challenge);
                        bytes_sent = send(sd,buff,sizeof(buff),0);
                        if (bytes_sent < 0) {
                            printf("Error! Can not sent data to socket %d \n", sd);
                            close(sd);
                            player[i].sockdes = 0;
                            break;
                        }
                    }
                    break;
                case 103:
                    printf("User tried login more than 3 times. Closing!\n");
                    close(sd);
                    player[i].sockdes = 0;
                    break;
                case 105:
                /// client send response
                case 106:
                    /// server recv response
                    printf("Hashed-challenge from user: %s\n",ms.message);

                    int response = atoi(ms.message);
                    int pass = user->element.pass;
                    int hashed_challenge = 0;
                    hashed_challenge += pass + challenge;
                    printf("Hashed-challenge from db: %d\n", hashed_challenge);
                    if(hashed_challenge == response) {
                        ms.command = 107;
                        printf("User authenticated! Exiting!\n");
                        player[i].isAvailable = 1;
                        strcpy(player[i].username, user->element.name);
                        sprintf(buff,"%d",ms.command);
                        bytes_sent = send(sd,buff,sizeof(buff),0);
                        if (bytes_sent < 0) {
                            printf("Error! Can not sent data to client!\n");
                            close(sd);
                            player[i].sockdes = 0;
                            break;
                        }

                    } else {
                        ms.command = 108;
                        printf("Password not match! Exiting!\n");
                        sprintf(buff,"%d",ms.command);
                        bytes_sent = send(sd,buff,sizeof(buff),0);
                        if (bytes_sent < 0) {
                            printf("Error! Can not sent data to client!\n");
                            close(sd);
                            player[i].sockdes = 0;
                        }
                    }
                    break;

                /// Lobby
                case 200: {
                    /// client A send request client B
                    playerType opponent;
                    opponent = FindPlayerOnUsername(ms.message, player);
                    PrintPlayerType(opponent);
                    break;
                }
                case 201:
                    /// recv request
                    break;
                case 202:
                    /// send accept
                    break;
                case 203:
                    /// recv accept
                    break;
                case 204:
                    /// List All Players In Username Status\n
//                    strcpy(buff, ListAllPlayersInUsernameAndStatus(player));
                    memset(buff,'\0',(strlen(buff)+1));
                    char tmp[100];
                    for(i = 0; i<30; i++) {
                        sprintf(tmp,"%s %d\n", player[i].username, player[i].isAvailable);
                        strcat(buff, tmp);
                    }
                    bytes_sent = send(sd,buff,sizeof(buff),0);
                    if (bytes_sent < 0) {
                        printf("Error! Can not sent data to client!\n");
                        close(sd);
                        player[i].sockdes = 0;
                        break;
                    }


                    break;

                /// Game
                case 300:
                    /// move: 300 ~ [cordinate]
                    break;
                case 301:
                    /// move + win signal
                    break;
                case 302:
                    /// send gamelog
                    break;
                case 309:
                    /// quit signal
                    close(sd);
                    player[i].sockdes = 0;
                    break;
                default:
                    printf("Unrecognized code: %d\n", ms.command);
                    close(sd);
                    player[i].sockdes = 0;
                    break;
                }
            }
        }
    }

    close(listen_sock);
    return 0;
}
