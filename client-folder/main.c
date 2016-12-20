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

piece board[8][8];
int enpass_a=-1,enpass_b=-1;
int epturn = 1; //en passant turn: 1~white 2~black
const piece startup[8][8]= {wrook,wknight,wbishop,wqueen,wking,wbishop,wknight,wrook,wpawn,wpawn,wpawn,wpawn,wpawn,wpawn,wpawn,wpawn,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank,blank, bpawn, bpawn, bpawn, bpawn, bpawn, bpawn, bpawn, bpawn, brook, bknight, bbishop, bqueen, bking, bbishop, bknight, brook};


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
            /// send invitation - not go first
            if(sendInvitation() == 1) {
                printf("Game on!!!");
                /// go to game phase
                int side = 1;
                int result = PlayGame(side);
                switch(result) {
                case 0:
                    printf("\nDraw game!!\n");
                    break;
                case 1:
                    printf("\nWhite won by mate!!\n");
                    break;
                case 2:
                    printf("\nBlack won by mate!!\n");
                    break;
                case 3:
                    printf("\nWhite won by fault - Black has committed 3 technical fault!!\n");
                    break;
                case 4:
                    printf("\nBlack won by fault - White has committed 3 technical fault!!\n");
                    break;
                }

                SendResult(result);
                ReceiveLog();

                char c;
                printf("\n\nGame end. Want to play with another player? (y/n): ");
                myFlush();
                scanf("%c", &c);
                if(c=='y')
                    FindAndPlayGame();
            }
            break;
        case 3: {
            /// accept invitation - go first
            FindAndPlayGame();
            break;
        }
        }
    } while(choice>=1&&choice<=5);

    close(client_sock);
    return 0;
}
