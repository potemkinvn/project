#include "clientfunction.h"

int debugMode = 0;
message ms;
char buff[1024];
char name[50];
char pass[30];
int bytes_sent,bytes_received;
int client_sock;
struct sockaddr_in server_addr;
int bytes_sent,bytes_received;
message ms;
playerType thisPlayer;

void printMenu()
{
    listAllPlayerAndStatus();
    getMyInvitationList();
    printf("\t1. Refresh player list\n");
    printf("\t2. Send invitation\n");
    printf("\t3. Accept invitation\n");
    printf("Your choice (1-3, other to quit): ");
}

int main()
{
    int choice;
    thisPlayer.isLogged = 0;
    strcpy(thisPlayer.username, "");
    client_sock=socket(AF_INET,SOCK_STREAM,0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5550);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(client_sock,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))!=0) {
        printf("\nError!Can not connect to sever!Client exit imediately! ");
        return 0;
    }

    // Receive server welcome
    bytes_received = recv(client_sock,buff,1024,0);
    if(bytes_received == -1) {
        printf("\nError!Cannot receive data from sever!\n");
        close(client_sock);
        exit(-1);
    }
    if(!debugMode) printf("\e[2J\e[H");
    printf("Server: %s",buff);

    do {
        /// user must be logged in first
        if(thisPlayer.isLogged == 0) {
            if(Login() != 1) {
                return 0;
            } else thisPlayer.isLogged = 1;
        }

        if(!debugMode) printf("\e[2J\e[H");
        printMenu();
        scanf("%d",&choice);
        switch (choice) {
        case 1:
            /// refresh player list
            break;

        case 2:
            /// send invitation
            if(sendInvitation() == 1) {
                printf("Game on!!!");
                /// go to game phase
            }
            break;
        case 3: {
            /// accept invitation
            if(acceptInvitation() == 1) {
                printf("Game on!!!");
                /// go to game phase
            }
            break;
        }
        }
    } while(choice>=1&&choice<=3);

    close(client_sock);
    return 0;
}
