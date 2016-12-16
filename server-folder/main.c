// truongnm 309 (123456)
// dungta 444 (123abc)
// 1 49 (1)
// 2 49 (1)

#include "serverfunction.h"


#define TRUE   1
#define FALSE  0
#define PORT 5550

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
FILE *input;

int main()
{
    top = NULL;
    user = NULL;
    opt = TRUE;
    max_clients = 30;
    int max_clients = 30;
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
        player[i].opponentSockdes = 0;
        player[i].isLogged = 0;
        player[i].inviteSockdes = 0;
        strcpy(player[i].ipAddress, "");
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
            char thisClientIp[10] = "";
            sprintf(thisClientIp,"%s", inet_ntoa(address.sin_addr));
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n", new_sock, thisClientIp, ntohs(address.sin_port));
            char *message = "Welcome to the multi-player console chess server\n";
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
                    strcpy(player[i].ipAddress,thisClientIp);
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
                case 102: {
                    TestUsername();
                    break;
                }
                case 103:
                    RetryUsernameExceed();
                    break;
                case 105:
                /// client send response
                case 106:
                    /// server recv response
                    SendAndProcessResponse();
                    break;

                /// Lobby
                case 200: {
                    /// client A send request client B: 200 ~ [B's name]
                    ReceiveInvitationRequest();
                    break;
                }
                case 201: {
                    /// recv player A stop listening status
                    player[i].inviteSockdes = 0;
                    break;
                }
                case 202: {
                    /// receive B accept A's invite: 202 ~ [A's name]
                    /// start step 2 in 3 ways handshake
                    /// forward it (the info that B accepted) to A: 203 ~ [B's name]
                    StartStep2In3WaysHandshake();
                    break;
                }
                case 206: {
                    /// receive A ready (ack) status: 206 ~ [B's name]
                    /// forward it to B: 206 ~ [A's name]
                    StartStep3In3WaysHandshake();
                    InitLog();
                    break;
                }
                case 207: {
                    /// List all players
                    ListAllPlayers();
                    break;
                }
                case 208:
                    /// get this player invitation list
                    GetInvitationList();
                    break;

                /// Game
                case 300:
                    /// Receive move from playerX: 300 ~ [cordinate]
                    /// forward to playerY: 301 ~ [cordinate]
                    GetMoveAndForwardMove();
                    break;
                case 302:
                    /// Receive game result signal: 302
                    /// send game log, after all done, reset both player
                    ProcessGameResult();
                    break;
                case 309: {
                    /// quit signal
                    int opponentSockdes = player[i].opponentSockdes;
                    player[i].isAvailable = 1;
                    player[i].opponentSockdes = 0;
                    player[opponentSockdes].isAvailable = 1;
                    player[opponentSockdes].opponentSockdes = 0;
                    SendLog();

                    break;
                }
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
