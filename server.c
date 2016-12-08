// truongnm 309 (123456)
// dungta 444 (123abc)
// 1 49 (1)
// 2 49 (1)


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
    int     isWaiting;
} playerType;

void ResetPlayer(playerType *pl)
{
    strcpy(pl->username, "");
    pl->sockdes = 0;
    pl->challenge = -9999;
    pl->isAvailable = 0;
    pl->opponent = NULL;
    pl->isLogged = 0;
    pl->isWaiting = 0;
}

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

playerType FindPlayerWaitingOnUsername(char username[], playerType player[30])
{
    playerType not_exist_player;
    not_exist_player.sockdes = -9999;
    int i;
    for(i=0; i<30; i++) {
        if(strcmp(username,player[i].username)==0 && (player[i].isWaiting == 1) ) {
            return player[i];
        }
    }
    return not_exist_player;
}

playerType FindPlayerOnUsername(char username[], playerType player[30])
{
    playerType not_exist_player;
    not_exist_player.sockdes = -9999;
    int i;
    for(i=0; i<30; i++) {
        if(strcmp(username,player[i].username)==0) {
            return player[i];
        }
    }
    return not_exist_player;
}

int GetPlayerIndexOnUsername(char username[], playerType player[30])
{
    int i;
    for(i=0; i<30; i++) {
        if(strcmp(username,player[i].username)==0) {
            return i;
        }
    }
    return -9999;
}
void PrintPlayerType(playerType player)
{
    printf("Username: %s, sockdes: %d, opponent: %p, challenge: %d, status: %d",\
           player.username, player.sockdes, player.opponent, player.challenge, player.isAvailable);
}

//char ListAllPlayersInUsernameAndStatus(playerType player[])
//{
//    int i = 0;
//    char result[1000];
//    char tmp[100];
//    for(i = 0; i<30; i++) {
//        sprintf(tmp,"%s %d\n", player[i].username, player[i].isAvailable);
//        strcat(result, tmp);
//    }
//    printf("%s",result);
//    return result;
//}

int main()
{
    node*       top = NULL;
    node*       user = NULL;
    int opt = TRUE;
    int listen_sock;
    int addrlen;
    int new_sock;
    playerType player[30];
    int max_clients = 30;
    int activity;
    int i;
    int sd;
    int max_sd;
    struct sockaddr_in address;
    int bytes_sent,bytes_received;
    char buff[1024];
    message ms;

    fd_set readfds;

    // get users info
    FILE *input = fopen("users","r");
    if(!input) {
        printf("Can't read users info from file.\n");
        exit(-1);
    }
    GetUserInfo(input,&top);
    fclose(input);

    /// Init default value for player array
    for (i = 0; i < max_clients; i++) {
        player[i].sockdes = 0;
        strcpy(player[i].username, "");
        player[i].challenge = -9999;
        player[i].isAvailable = 0;
        player[i].opponent = NULL;
        player[i].isLogged = 0;
        player[i].isWaiting = 0;
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
                if (bytes_received <= 0) {
                    printf("Error! Can not receive data from socket %d \n", sd);
                    close(sd);
                    ResetPlayer(&player[i]);
                    break;
                }
                printf("\nReceived message from client on socket %d: %s\n", sd, buff);

                ms = ParseMessage(buff);

                switch (ms.command) {
                /// Login
                case 100:
                case 101:
                case 102:
                    /// recv username
                    printf("Input - Command: %d ~ Message: %s\n", ms.command, ms.message);

                    //check if this user already logged in this session
                    if(player[i].isLogged) {
                        printf("Output - Command: %d ~ Message: You are already logged in as '%s'!\n", 107, player[i].username);
                        sprintf(buff,"%d ~ You are already logged in as '%s'!",107, player[i].username);
                        bytes_sent = send(sd,buff,sizeof(buff),0);
                        if (bytes_sent <= 0) {
                            printf("Error! Can not sent data to client!\n");
                            close(sd);
                            ResetPlayer(&player[i]);
                            break;
                        }
                        break;
                    }

                    //check if this user already logged in another session
                    int j;
                    int hasLoggedInAnotherSession = 0;
                    for (j = 0; j < max_clients; j++) {
                        if( strcmp(player[j].username, ms.message) == 0 && player[j].isAvailable == 1) {
                            hasLoggedInAnotherSession = 1;
                            ms.command++;
                            printf("Output - Command: %d ~ Message: This user %s already logged, another user name is required!\n", ms.command, ms.message);
                            sprintf(buff,"%d ~ This user %s already logged, another user name is required!", ms.command, ms.message);
                            bytes_sent = send(sd,buff,sizeof(buff),0);
                            if (bytes_sent <= 0) {
                                printf("Error! Can not sent data to client!\n");
                                close(sd);
                                ResetPlayer(&player[i]);
                                break;
                            }
                            break;
                        }
                    }
                    if(hasLoggedInAnotherSession)
                        break;

                    // check if this user exist in database
                    user = FindNodeOnUsername(&top, ms.message);
                    if(user==NULL) {
                        ms.command++;
                        sprintf(buff, "%d ~ %s", ms.command, "User not exist!");
                        bytes_sent = send(sd,buff,sizeof(buff),0);
                        if (bytes_sent <= 0) {
                            printf("Error! Can not sent data to socket %d \n", sd);
                            close(sd);
                            ResetPlayer(&player[i]);
                            break;
                        }
                        printf("Output - Command: %d\n", ms.command);
                    } else {
                        strcpy(player[i].username, user->element.name);

                        // send challenge to user
                        ms.command = 104;
                        srand(time(NULL));
                        int r = rand() % 20;
                        player[i].challenge = r;

                        printf("Output - Command: %d ~ Message: %d\n", ms.command, player[i].challenge);
                        sprintf(buff,"%d ~ %d",ms.command, player[i].challenge);
                        bytes_sent = send(sd,buff,sizeof(buff),0);
                        if (bytes_sent <= 0) {
                            printf("Error! Can not sent data to socket %d \n", sd);
                            close(sd);
                            ResetPlayer(&player[i]);
                            break;
                        }
                    }
                    break;
                case 103:
                    printf("User tried login more than 3 times. Closing!\n");
                    close(sd);
                    ResetPlayer(&player[i]);
                    break;
                case 105:
                /// client send response
                case 106:
                    /// server recv response
                    printf("Hashed-challenge from user: %s\n",ms.message);

                    int response = atoi(ms.message);
                    user = FindNodeOnUsername(&top, player[i].username);
                    int pass = user->element.pass;
                    int hashed_challenge = 0;
                    hashed_challenge += pass + player[i].challenge;
                    printf("Hashed-challenge from db: %d\n", hashed_challenge);
                    if(hashed_challenge == response) {
                        ms.command = 107;
                        printf("User authenticated!\n");
                        player[i].isAvailable = 1;
                        player[i].isLogged = 1;
                        strcpy(player[i].username, user->element.name);
                        sprintf(buff,"%d",ms.command);
                        bytes_sent = send(sd,buff,sizeof(buff),0);
                        if (bytes_sent <= 0) {
                            printf("Error! Can not sent data to client!\n");
                            close(sd);
                            ResetPlayer(&player[i]);
                            break;
                        }

                    } else {
                        ms.command = 108;
                        printf("Password not match! Exiting!\n");
                        sprintf(buff,"%d",ms.command);
                        bytes_sent = send(sd,buff,sizeof(buff),0);
                        if (bytes_sent <= 0) {
                            printf("Error! Can not sent data to client!\n");
                            close(sd);
                            ResetPlayer(&player[i]);
                        }
                    }
                    break;

                /// Lobby
                case 200: {
                    /// client A send request client B: 200 ~ [B's name]
                    playerType opponent;
                    opponent = FindPlayerWaitingOnUsername(ms.message, player);
                    //PrintPlayerType(opponent);
                    player[i].isAvailable = 0;

                    // forward request to B: 201 ~ [A's name]
                    ms.command = 201;
                    int sd_client_B = opponent.sockdes;
                    sprintf(buff,"%d ~ %s",ms.command, player[i].username);
                    bytes_sent = send(sd_client_B,buff,sizeof(buff),0);
                    if (bytes_sent <= 0) {
                        // B disconnected, send back to A: 205 ~ [B's name]
                        printf("Error! Can not sent data to client!\n");
                        close(sd_client_B);
                        ResetPlayer(&opponent);

                        ms.command = 205;
                        sprintf(buff,"%d ~ %s",ms.command, ms.message);
                        bytes_sent = send(sd,buff,sizeof(buff),0);
                        if (bytes_sent <= 0) {
                            printf("Error! Can not sent data to client!\n");
                            close(sd);
                            ResetPlayer(&player[i]);
                        }
                    }

                    break;
                }
                case 204: {
                    /// receive B refuse A's invite: 204 ~ [A's name]
                    /// forward it back to A: 205 ~ [B's name]
                    ms.command = 205;
                    player[i].isWaiting = 0;
//                    playerType playerA;
//                    playerA = FindPlayerOnUsername(ms.message, player);
//                    playerA.isAvailable = 1;
                    int playerAIndex = GetPlayerIndexOnUsername(ms.message, player);
                    player[playerAIndex].isAvailable = 1;
                    sprintf(buff,"%d ~ %s",ms.command, player[i].username);
                    bytes_sent = send(player[playerAIndex].sockdes,buff,sizeof(buff),0);
                    if (bytes_sent <= 0) {
                        printf("Error! Can not sent data to client!\n");
                        close(player[playerAIndex].sockdes);
                        ResetPlayer(&player[playerAIndex]);
                    }

                    break;
                }
                case 202: {
                    /// receive B accept A's invite: 202 ~ [A's name]
                    /// start 3 ways acceptance
                    /// forward it to A: 203 ~ [B's name]
                    ms.command = 203;
                    int playerAIndex = GetPlayerIndexOnUsername(ms.message, player);
                    sprintf(buff,"%d ~ %s",ms.command, player[i].username);
                    bytes_sent = send(player[playerAIndex].sockdes,buff,sizeof(buff),0);
                    if (bytes_sent <= 0) {
                        printf("Error! Can not sent data to client!\n");
                        close(player[playerAIndex].sockdes);
                        ResetPlayer(&player[playerAIndex]);
                    }

                    break;
                }
                case 206: {
                    /// receive A ready (ack) status: 206 ~ [B's name]
                    /// forward it to B: 206 ~ [A's name]
                    ms.command = 206;
                    int playerBIndex = GetPlayerIndexOnUsername(ms.message, player);
                    player[playerBIndex].isWaiting = 0;
                    sprintf(buff,"%d ~ %s",ms.command, player[playerBIndex].username);
                    bytes_sent = send(player[playerBIndex].sockdes,buff,sizeof(buff),0);
                    if (bytes_sent <= 0) {
                        printf("Error! Can not sent data to client!\n");
                        close(player[playerBIndex].sockdes);
                        ResetPlayer(&player[playerBIndex]);
                    }

                    break;
                }
                case 207:{
                    /// List All Available Players In Username\n
                    memset(buff,'\0',(strlen(buff)+1));
                    char tmp[100];
                    strcpy(buff, "");
                    for(i = 0; i<30; i++) {
                        if(player[i].isAvailable == 1) {
                            sprintf(tmp,"%s \t", player[i].username);
                            strcat(buff, tmp);
                        }
                    }
                    bytes_sent = send(sd,buff,sizeof(buff),0);
                    if (bytes_sent <= 0) {
                        printf("Error! Can not sent data to client!\n");
                        close(sd);
                        ResetPlayer(&player[i]);
                        break;
                    }
                    break;
                }
                case 208:
                    /// recv player B listening status for 5 seconds
                    player[i].isWaiting = 1;
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
                    ResetPlayer(&player[i]);
                    break;
                default:
                    printf("Unrecognized code: %d\n", ms.command);
                    close(sd);
                    ResetPlayer(&player[i]);
                    break;
                }
            }
        }
    }

    close(listen_sock);
    return 0;
}
