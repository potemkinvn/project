#include "clientfunction.h"

extern int debugMode;
extern message ms;
extern char buff[1024];
extern char name[50];
extern char pass[30];
extern int bytes_sent,bytes_received;
extern int client_sock;
extern struct sockaddr_in server_addr;
extern int bytes_sent,bytes_received;
extern message ms;
extern playerType thisPlayer;

extern piece board[8][8];
extern int enpass_a,enpass_b;
extern int epturn; //en passant turn: 1~white 2~black
extern const piece startup[8][8];

void myFlush()
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

/** \brief Parse Message
 *         usage: ms = ParseMessage(buff);
 *         char buff[] = "101 ~ abc" => ms.command = 101; ms.message = "abc"
 * \param char recv_data[]
 * \return message type variable
 *
 */
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


/** \brief Login function
 *
 * \param
 * \return int -1 if disconnected, 0 if invalid credential, 1 if success
 *
 */
int Login()
{
    int i;
    ms.command = 100;

    // 1. Send server user name
    while(ms.command != 104) {
        memset(buff,'\0',(strlen(buff)+1));
        printf("Username: ");
        scanf("%s",name);
        sprintf(buff, "%d ~ %s", ms.command, name);
        bytes_sent = send(client_sock, buff, sizeof(buff), 0);
        if(bytes_sent <= 0) {
            printf("\nError! Cannot send data to sever!\n");
            close(client_sock);
            exit(-1);
        }

        bytes_received = recv(client_sock, buff, sizeof(buff),0);
        if(bytes_received <= 0) {
            printf("\nError! Cannot receive data from server!\n");
            close(client_sock);
            exit(-1);
        }

        ms = ParseMessage(buff);
        if(ms.command == 104 || ms.command == 107) {
            break;
        } else if(ms.command == 100) {
            printf("%s Remains attempt 3 times! Status: %d\n", ms.message, ms.command);
        } else if(ms.command == 101) {
            printf("%s Remains attempt 2 times! Status: %d\n", ms.message, ms.command);
        } else if(ms.command == 102) {
            printf("%s Remains attempt 1 times! Status: %d\n", ms.message, ms.command);
        } else if(ms.command == 103) {
            printf("%s Too many attempt! Exiting! Status: %d\n", ms.message, ms.command);
            close(client_sock);
            exit(-1);
        }
    }

    if(ms.command == 104) {
        printf("User name exists! Status: 104\n");
    } else if(ms.command == 107) {
        printf("%s", ms.message);
        return 1;
    }

    // 2. Receive challenge message from server
    int challenge = atoi(ms.message);
    printf("Server challenge: %d\n", challenge);

    // 3. Send user hashed-challenge
    ms.command = 105;
    printf("Password: ");
    scanf("%s", pass);
    // calculate hashed-challenge
    int response = 0;
    for(i=0; i<strlen(pass); i++) {
        response += pass[i];
    }
    //printf("Pass: %d\n",response);
    response += challenge;
    printf("Hashed-challenge: %d\n", response);
    sprintf(buff, "%d ~ %d", ms.command, response);
    bytes_sent = send(client_sock, buff, sizeof(buff), 0);
    if(bytes_sent <= 0) {
        printf("\nError! Cannot send data to sever!\n");
        close(client_sock);
        exit(-1);
    }

    // 5. Did password match ?
    bytes_received = recv(client_sock,buff,1024,0);
    if(bytes_received <= 0) {
        printf("\nError! Cannot receive data from sever!\n");
        close(client_sock);
        exit(-1);
    }
    ms = ParseMessage(buff);
    if(ms.command == 107) {
        printf("User authenticated!\n");
        strcpy(thisPlayer.username, name);
        return 1;
    } else if(ms.command == 109) {
        printf("Password not match! Exiting!\n");
        return 0;
    }

    return 0;
}

/** \brief List all player and their status
 *
 * \param
 * \param
 * \return -1 if disconnected, 0 if list success
 *
 */
int listAllPlayerAndStatus()
{
    sprintf(buff, "%d", 207);
    bytes_sent = send(client_sock, buff, sizeof(buff), 0);
    if(bytes_sent <= 0) {
        printf("\nError! Cannot send data to sever!\n");
        close(client_sock);
        exit(-1);
    }

    bytes_received = recv(client_sock,buff,1024,0);
    if(strcmp(buff, "") == 0) {
        printf("None user available currently!\n");
    } else {
        if(bytes_received <= 0) {
            printf("\nError! Cannot receive data from sever!\n");
            close(client_sock);
            exit(-1);
        }
        printf("\nList all players:\n%s", buff);
    }
    return 0;
}

/** \brief Get this player invitation list
 *
 * \param
 * \param
 * \return -1 if disconnected, 0 if list empty, 1 if list has more than 1 user
 *
 */
int getMyInvitationList()
{
    sprintf(buff, "%d", 208);
    bytes_sent = send(client_sock, buff, sizeof(buff), 0);
    if(bytes_sent <= 0) {
        printf("\nError! Cannot send data to sever!\n");
        close(client_sock);
        exit(-1);
    }

    bytes_received = recv(client_sock,buff,1024,0);
    if(strcmp(buff, "") == 0) {
        printf("Your invitation list is empty.\n\n");
        return 0;
    } else {
        if(bytes_received <= 0) {
            printf("\nError! Cannot receive data from sever!\n");
            close(client_sock);
            exit(-1);
        }
        printf("You get invitation to play from player: %s\n\n", buff);
        return 1;
    }
}

/** \brief Player A send invitation to player B
 *
 * \param
 * \param
 * \return -1 if disconnected, 0 if refused, 1 if accepted
 *
 */
int sendInvitation()
{
    ms.command = 200;
    if(!debugMode) printf("\e[2J\e[H");
    listAllPlayerAndStatus();
    printf("\nEnter player name to invite: ");
    scanf("%s", name);
    sprintf(buff, "%d ~ %s", ms.command, name);
    bytes_sent = send(client_sock, buff, sizeof(buff), 0);
    if(bytes_sent <= 0) {
        printf("\nError! Cannot send data to sever!\n");
        close(client_sock);
        exit(-1);
    }

    /// set timeout 30 seconds
    //struct timeval tv;
    // tv.tv_sec = 30;  /* 30 Secs Timeout */
    // tv.tv_usec = 0;
    //  setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

    /// wait for result: acceptance from player B or failed
    printf("Waiting response from player '%s' for 30 seconds ...\n", name);
    bytes_received = recv(client_sock,buff,1024,0);
    if(bytes_received <= 0) {
        /// send to server stop invite status (to toggle inviteSockdes back to 0)
        sprintf(buff, "%d", 201);
        bytes_sent = send(client_sock, buff, sizeof(buff), 0);
        if(bytes_sent <= 0) {
            printf("\nError! Cannot send data to sever!\n");
            close(client_sock);
            exit(-1);
        }

        printf("Not receive response from player '%s'\nPress any key will go back to MAIN MENU\n", name);
        myFlush();
        getchar();
        return 0;
    }
    ms = ParseMessage(buff);

    if(ms.command == 205) {
        /// B refuse to play or B disconnected: 205 ~ [B's name]
        printf("Player %s refused to play or disconnected. (2/3)\n", ms.message);
        printf("Press any key will go back to MAIN MENU.\n");
        myFlush();
        getchar();
        return 0;
    } else if(ms.command == 209) {
        /// B not exist or A send invite to A
        printf("\n%s\n", ms.message);
        printf("Press any key will go back to MAIN MENU.\n");
        myFlush();
        getchar();
        return 0;
    } else if(ms.command == 203) {
        /// B accept to play: 203 ~ [B's name]
        /// send B ready (ack) status: 206 ~ [B's name]
        ms.command = 206;
        printf("Player %s accepted to play. (2/3)\n", ms.message);
        printf("Sending back ready status. (3/3)\n");
        sprintf(buff, "%d ~ %s", ms.command, ms.message);
        bytes_sent = send(client_sock, buff, sizeof(buff), 0);
        if(bytes_sent <= 0) {
            printf("\nError! Cannot send data to sever!\n");
            close(client_sock);
            exit(-1);
        }
        printf("Connect finished. '%s' will go first.\n", name);
        printf("Press any key will start game.\n");
        myFlush();
        getchar();
        return 1;
    }
    return 0;
}

/** \brief Player B send accept to player A
 *
 * \param
 * \param
 * \return -1 if disconnected, 0 if smt failed, 1 if accepted
 *
 */
int acceptInvitation()
{
    ms.command = 202;
    if(!debugMode) printf("\e[2J\e[H");
    if(getMyInvitationList() == 0) {
        printf("Press any key will go back to MAIN MENU.\n");
        myFlush();
        getchar();
        return 0;
    }

    printf("\nEnter player name to accept: ");
    scanf("%s", name);
    sprintf(buff, "%d ~ %s", ms.command, name);
    bytes_sent = send(client_sock, buff, sizeof(buff), 0);
    if(bytes_sent <= 0) {
        printf("\nError! Cannot send data to sever!\n");
        close(client_sock);
        exit(-1);
    }

    /// wait for ready (ack) status from A
    bytes_received = recv(client_sock,buff,1024,0);
    if(bytes_received <= 0) {
        printf("\nError! Cannot receive data from sever!\n");
        close(client_sock);
        exit(-1);
    }
    ms = ParseMessage(buff);
    if(ms.command == 206) {
        /// received ready (ack) status from A: 206 ~ [A's name]
        printf("(3/3) Player %s ready.\n", ms.message);
        printf("Connect finished. You will go first.\n");
        printf("Press any key will start game.\n");
        myFlush();
        getchar();
        return 1;
    } else if(ms.command == 204) {
        /// can't send accept to A, maybe A disconnected
        printf("Can't connect to '%s'.\n", name);
        printf("Press any key will go back to MAIN MENU.\n");
        myFlush();
        getchar();
        return 0;
    }
    return 0;
}


void setup()
{
    int i,j;
    for(i=0; i<8; i++) {
        for(j=0; j<8; j++) {
            board[i][j]=startup[i][j];
        }
    }
}

void printboard()
{
    int a,b;
    char piece;
    for(a=7; a>-1; a--) {
        printf("\n %d| ",a);
        for(b=0; b<8; b++) {
            switch(board[a][b]) {
            case blank:
                piece = ' ';
                break;
            case wpawn:
                piece = 'P';
                break;
            case wknight:
                piece = 'N';
                break;
            case wbishop:
                piece = 'B';
                break;
            case wrook:
                piece = 'R';
                break;
            case wking:
                piece = 'K';
                break;
            case wqueen:
                piece = 'Q';
                break;
            case bpawn:
                piece = 'p';
                break;
            case bknight:
                piece = 'n';
                break;
            case bbishop:
                piece = 'b';
                break;
            case brook:
                piece = 'r';
                break;
            case bqueen:
                piece = 'q';
                break;
            case bking:
                piece = 'k';
                break;
            }
            if(piece==32) printf("[%c]", piece);
            if(piece<97&&piece!=32) printf(RED"[%c]"RESET, piece);
            if(piece>=97) printf(BLUE"[%c]"RESET, piece);
        }
    }
    printf("\n");
    printf("    ________________________\n");
    printf("     0  1  2  3  4  5  6  7\n");
}

void Move(int a,int b,int c,int d)
{
    board[c][d] = board[a][b];
    board[a][b] = blank;
}
int ValidSquare(int a,int b,int c,int d)
{
    if((a<0)||a>7) return 0;
    if((b<0)||b>7) return 0;
    if((c<0)||c>7) return 0;
    if((d<0)||d>7) return 0;

    return 1;
}
int CheckBlack(int *whitelist)
{
    int x,y; // WKing's position
    int a,b,c,d;
    int i;
    int num;
    int result=0;
    for(a=0; a<8; a++) {
        for(b=0; b<8; b++) {
            if(board[a][b]==bking) {
                x=a;
                y=b;
                break;
            }
        }
    }
    for(a=0; a<8; a++) {
        //      if(result==1) break;
        for(b=0; b<8; b++) {
            //       if(result==1) break;
            for(i=0; i<218; i++) {
                num = a*1000+b*100+x*10+y;
                if(num == *(whitelist+i)) {
                    result = 1;
                    break;
                }
            }
        }
    }
    return result;
}

int *BlackMoveList(int epa,int epb)
{
    int i;
    static int allmoves[218];
    int a=0;
    int b=0;
    int c=0;
    int d=0;
    int e=0;
    int f=0;
    int z=0;
    for(i=0; i<218; i++) {
        allmoves[i]=0;
    }
//   printf("\nepa: %d, epb: %d\n",epa,epb);
    for(a=0; a<8; a++) {
        for(b=0; b<8; b++) {
            switch(board[a][b]) {
            case bqueen:
                for(e = -1; e <= 1; e++) {
                    for(f= -1; f <=1; f++) {
                        c= a+e;
                        d= b+f;
                        if(!(f==0&&e==0)) {
                            do {
                                if(ValidSquare(a,b,c,d)!=0) {
                                    if(board[c][d]==0) {
                                        a=a;
                                        b=b;
                                        c=c;
                                        d=d;
                                        allmoves[z]=a*1000+b*100+c*10+d;
                                        z++;
                                    }
                                    if(board[c][d]>6) {
                                        a=a;
                                        b=b;
                                        c=c;
                                        d=d;
                                        allmoves[z]=a*1000+b*100+c*10+d;
                                        z++;
                                        break;
                                    }
                                    if(board[c][d]<7&&board[c][d]!=0) break;
                                    c = c+e;
                                    d = d+f;
                                }
                            } while(ValidSquare(a,b,c,d)!=0);
                        }
                    }

                }
                break;
            case bpawn:
                c = a-1;
                d = b-1;
                if(ValidSquare(a,b,c,d)!=0) {
                    if(board[c][d]>6||(c==epa&&d==epb&&epturn==2)) {
                        a = a;
                        b = b;
                        c = c;
                        d = d;
                        allmoves[z]=a*1000+b*100+c*10+d;
                        z++;
                    }
                }
                c = a-1;
                d = b+1;
                if(ValidSquare(a,b,c,d)!=0) {
                    if(board[c][d]>6||(c==epa&&d==epb&&epturn==2)) {
                        a = a;
                        b = b;
                        c = c;
                        d = d;
                        allmoves[z]=a*1000+b*100+c*10+d;
                        z++;
                    }
                }
                if(a==6) {
                    c=5;
                    d=b;
                    if(ValidSquare(a,b,c,d)!=0) {
                        if(board[c][d]==0) {
                            a = a;
                            b = b;
                            c = c;
                            d = d;
                            allmoves[z]=a*1000+b*100+c*10+d;
                            z++;
                            c = 4;
                            d = b;
                            if(ValidSquare(a,b,c,d)!=0) {
                                if(board[c][d]==0) {
                                    a = a;
                                    b = b;
                                    c = c;
                                    d = d;
                                    allmoves[z]=a*1000+b*100+c*10+d;
                                    z++;
                                }
                            }
                        }
                    }
                } else {
                    c = a-1;
                    d = b;
                    if(ValidSquare(a,b,c,d)!=0) {
                        if(board[c][d]==0) {
                            a = a;
                            b = b;
                            c = c;
                            d = d;
                            allmoves[z]=a*1000+b*100+c*10+d;
                            z++;
                        }
                    }
                }
                break;
            case brook:
                for(e=-1; e<=1; e++) {
                    for(f=-1; f<=1; f++) {
                        c = a+e;
                        d = b+f;
                        if(abs(e)+abs(f)==1) {
                            do {
                                if(ValidSquare(a,b,c,d)!=0) {
                                    if(board[c][d]==0) {
                                        a = a;
                                        b = b;
                                        c = c;
                                        d = d;
                                        allmoves[z]=a*1000+b*100+c*10+d;
                                        z++;
                                    }
                                    if(board[c][d]>6) {
                                        a = a;
                                        b = b;
                                        c = c;
                                        d = d;
                                        allmoves[z]=a*1000+b*100+c*10+d;
                                        z++;
                                        break;
                                    }
                                    if(board[c][d]<7&&board[c][d]!=0) break;
                                    c = c+e;
                                    d = d+f;
                                }
                            } while(ValidSquare(a,b,c,d)!=0);
                        }
                    }
                }
                break;
            case bknight:
                for(e=-2; e<=2; e++) {
                    for(f=-2; f<=2; f++) {
                        c = a+e;
                        d = b+f;
                        if(abs(e)+abs(f)==3) {
                            if(ValidSquare(a,b,c,d)!=0) {
                                if(board[c][d]>6||board[c][d]==0) {
                                    a = a;
                                    b = b;
                                    c = c;
                                    d = d;
                                    allmoves[z]=a*1000+b*100+c*10+d;
                                    z++;
                                }
                            }
                        }
                    }
                }
                break;
            case bbishop:
                for(e=-1; e<=1; e++) {
                    for(f=-1; f<=1; f++) {
                        c = a+e;
                        d = b+f;
                        if(abs(e)+abs(f)==2) {
                            do {
                                if(ValidSquare(a,b,c,d)!=0) {
                                    if(board[c][d]==0) {
                                        a = a;
                                        b = b;
                                        c = c;
                                        d = d;
                                        allmoves[z]=a*1000+b*100+c*10+d;
                                        z++;
                                    }
                                    if(board[c][d]>6) {
                                        a = a;
                                        b = b;
                                        c = c;
                                        d = d;
                                        allmoves[z]=a*1000+b*100+c*10+d;
                                        z++;
                                        break;
                                    }
                                    if(board[c][d]<7&&board[c][d]!=0) break;
                                }
                                c = c+e;
                                d = d+f;
                            } while(ValidSquare(a,b,c,d)!=0);
                        }
                    }
                }
                break;
            case bking:
                for(e=-1; e<=1; e++) {
                    for(f=-1; f<=1; f++) {
                        if(!(e==0&&f==0)) {
                            c = a+e;
                            d = b+f;
                            if(ValidSquare(a,b,c,d)!=0) {
                                if(board[c][d]>6||board[c][d]==0) {
                                    a = a;
                                    b = b;
                                    c = c;
                                    d = d;
                                    allmoves[z]=a*1000+b*100+c*10+d;
                                    z++;
                                }
                            }
                        }
                    }
                }
                if(a==7 && b==4 && board[7][7]==brook && board[7][6]==0 && board[7][5]==0) {
                    a = 7;
                    b = 7;
                    c = 7;
                    d = 7;
                    allmoves[z]=a*1000+b*100+c*10+d;
                    z++;
                }
                if(a==7 && b==4 && board[7][0]==brook && board[7][1]==0 && board[7][2]==0 && board[7][3]==0) {
                    a = 6;
                    b = 6;
                    c = 6;
                    d = 6;
                    allmoves[z]=a*1000+b*100+c*10+d;
                    z++;
                }
            }
        }
    }
    return allmoves;
}

int *WhiteMoveList(int epa,int epb)
{
    int i;
    static int allmoves[218];
    int a=0;
    int b=0;
    int c=0;
    int d=0;
    int e=0;
    int f=0;
    int z=0;
    for(i=0; i<218; i++) {
        allmoves[i]=0;
    }
//  printf("\nepa: %d, epb: %d\n",epa,epb);
    for(a=0; a<8; a++) {
        for(b=0; b<8; b++) {
            switch(board[a][b]) {
            case wqueen:
                for(e = -1; e <= 1; e++) {
                    for(f= -1; f <=1; f++) {
                        c= a+e;
                        d= b+f;
                        if(!(f==0&&e==0)) {
                            do {
                                if(ValidSquare(a,b,c,d)!=0) {
                                    if(board[c][d]==0) {
                                        a=a;
                                        b=b;
                                        c=c;
                                        d=d;
                                        allmoves[z]=a*1000+b*100+c*10+d;
                                        z++;
                                    }
                                    if((board[c][d]<7)&&board[c][d]!=0) {
                                        a=a;
                                        b=b;
                                        c=c;
                                        d=d;
                                        allmoves[z]=a*1000+b*100+c*10+d;
                                        z++;
                                        break;
                                    }
                                    if(board[c][d]>6) break;
                                    c = c+e;
                                    d = d+f;
                                }
                            } while(ValidSquare(a,b,c,d)!=0);
                        }
                    }

                }
                break;
            case wpawn:
                c = a+1;
                d = b-1;
                if(ValidSquare(a,b,c,d)!=0) {
                    if(((board[c][d]<7)&&board[c][d]!=0)||(c==epa&&d==epb&&epturn==1)) {
                        a = a;
                        b = b;
                        c = c;
                        d = d;
                        allmoves[z]=a*1000+b*100+c*10+d;
                        z++;
                    }
                }
                c = a+1;
                d = b+1;
                if(ValidSquare(a,b,c,d)!=0) {
                    if(((board[c][d]<7)&&board[c][d]!=0)||(c==epa&&d==epb&&epturn==1)) {
                        a = a;
                        b = b;
                        c = c;
                        d = d;
                        allmoves[z]=a*1000+b*100+c*10+d;
                        z++;
                    }
                }
                if(a==1) {
                    c=2;
                    d=b;
                    if(ValidSquare(a,b,c,d)!=0) {
                        if(board[c][d]==0) {
                            a = a;
                            b = b;
                            c = c;
                            d = d;
                            allmoves[z]=a*1000+b*100+c*10+d;
                            z++;
                        }
                    }
                    c = 3;
                    d = b;
                    if(ValidSquare(a,b,c,d)!=0) {
                        if(board[c][d]==0) {
                            a = a;
                            b = b;
                            c = c;
                            d = d;
                            allmoves[z]=a*1000+b*100+c*10+d;
                            z++;
                        }
                    }
                } else {
                    c = a+1;
                    d = b;
                    if(ValidSquare(a,b,c,d)!=0) {
                        if(board[c][d]==0) {
                            a = a;
                            b = b;
                            c = c;
                            d = d;
                            allmoves[z]=a*1000+b*100+c*10+d;
                            z++;
                        }
                    }
                }
                break;
            case wrook:
                for(e=-1; e<=1; e++) {
                    for(f=-1; f<=1; f++) {
                        c = a+e;
                        d = b+f;
                        if(abs(e)+abs(f)==1) {
                            do {
                                if(ValidSquare(a,b,c,d)!=0) {
                                    if(board[c][d]==0) {
                                        a = a;
                                        b = b;
                                        c = c;
                                        d = d;
                                        allmoves[z]=a*1000+b*100+c*10+d;
                                        z++;
                                    }
                                    if(board[c][d]<7&&board[c][d]!=0) {
                                        a = a;
                                        b = b;
                                        c = c;
                                        d = d;
                                        allmoves[z]=a*1000+b*100+c*10+d;
                                        z++;
                                        break;
                                    }
                                    if(board[c][d]>6) break;
                                    c = c+e;
                                    d = d+f;
                                }
                            } while(ValidSquare(a,b,c,d)!=0);
                        }
                    }
                }
                break;
            case wknight:
                for(e=-2; e<=2; e++) {
                    for(f=-2; f<=2; f++) {
                        c = a+e;
                        d = b+f;
                        if(abs(e)+abs(f)==3) {
                            if(ValidSquare(a,b,c,d)!=0) {
                                if(board[c][d]<7) {
                                    a = a;
                                    b = b;
                                    c = c;
                                    d = d;
                                    allmoves[z]=a*1000+b*100+c*10+d;
                                    z++;
                                }
                            }
                        }
                    }
                }
                break;
            case wbishop:
                for(e=-1; e<=1; e++) {
                    for(f=-1; f<=1; f++) {
                        c = a+e;
                        d = b+f;
                        if(abs(e)+abs(f)==2) {
                            do {
                                if(ValidSquare(a,b,c,d)!=0) {
                                    if(board[c][d]==0) {
                                        a = a;
                                        b = b;
                                        c = c;
                                        d = d;
                                        allmoves[z]=a*1000+b*100+c*10+d;
                                        z++;
                                    }
                                    if(board[c][d]<7&&board[c][d]!=0) {
                                        a = a;
                                        b = b;
                                        c = c;
                                        d = d;
                                        allmoves[z]=a*1000+b*100+c*10+d;
                                        z++;
                                        break;
                                    }
                                    if(board[c][d]>6) break;
                                }
                                c = c+e;
                                d = d+f;
                            } while(ValidSquare(a,b,c,d)!=0);
                        }
                    }
                }
                break;
            case wking:
                for(e=-1; e<=1; e++) {
                    for(f=-1; f<=1; f++) {
                        if(!(e==0&&f==0)) {
                            c = a+e;
                            d = b+f;
                            if(ValidSquare(a,b,c,d)!=0) {
                                if(board[c][d]<7) {
                                    a = a;
                                    b = b;
                                    c = c;
                                    d = d;
                                    allmoves[z]=a*1000+b*100+c*10+d;
                                    z++;
                                }
                            }
                        }
                    }
                }
                if(a==0 && b==4 && board[0][7]==wrook && board[0][6]==0 && board[0][5]==0) {
                    a = 2;
                    b = 2;
                    c = 2;
                    d = 2;
                    allmoves[z]=a*1000+b*100+c*10+d;
                    z++;
                }
                if(a==0 && b==4 && board[0][0]==wrook && board[0][1]==0 && board[0][2]==0 && board[0][3]==0) {
                    a = 1;
                    b = 1;
                    c = 1;
                    d = 1;
                    allmoves[z]=a*1000+b*100+c*10+d;
                    z++;
                }
            }
        }
    }
    return allmoves;
}

int CheckWhite(int *blacklist)
{
    int x,y; // WKing's position
    int a,b;
    int i;
    int num;
    int result=0;
    for(a=0; a<8; a++) {
        for(b=0; b<8; b++) {
            if(board[a][b]==wking) {
                x=a;
                y=b;
                break;
            }
        }
    }
    for(a=0; a<8; a++) {
        for(b=0; b<8; b++) {
            for(i=0; i<218; i++) {
                num = a*1000+b*100+x*10+y;
                if(num == *(blacklist+i)) {
//                   printf("check move is: list[%d] %d \n",i,blacklist[i]);
                    result = 1;
                    break;
                }
            }
        }
    }
    return result;
}

int WhiteMove(char *cmd,int turn,int castlingcheck)
{
    int a,b,c,d;
    int num;
    int i;
    int *list = WhiteMoveList(enpass_a,enpass_b);
    int *blacklist = BlackMoveList(enpass_a,enpass_b);
    int castling=0;
    int queensideok=1;
    int kingsideok=1;
    int x,y;
    for(x=0; x<8; x++) {
        if(queensideok==0) break;
        for(y=0; y<8; y++) {
            if(queensideok==0) break;
            for(i=0; i<218; i++) {
                int val1=x*1000 + y*100 + 1;
                int val2=x*1000 + y*100 + 2;
                int val3=x*1000 + y*100 + 3;
                if(val1==blacklist[i]||val2==blacklist[i]||val3==blacklist[i]) {
                    queensideok=0;
                    break;
                }
            }
        }
    }
    for(x=0; x<8; x++) {
        if(kingsideok==0) break;
        for(y=0; y<8; y++) {
            if(kingsideok==0) break;
            for(i=0; i<218; i++) {
                int val1=x*1000 + y*100 + 5;
                int val2=x*1000 + y*100 + 6;
                if(val1==blacklist[i]||val2==blacklist[i]) {
                    kingsideok=0;
                    break;
                }
            }
        }
    }
    // printf("queenside: %d\n",queensideok);

    int result=0;

    if(turn != 0) return 0;
    if(strlen(cmd)!=4) return 0;
    a = cmd[0]-'0';
    b = cmd[1]-'0';
    c = cmd[2]-'0';
    d = cmd[3]-'0';
    num = a*1000 + b*100 + c *10 +d;
    if(a==c&&b==d&&(a==1&&b==1)&&queensideok==1&&castlingcheck==0&&CheckWhite(blacklist)==0) {
        castling = 1;
    }
    if(a==c&&b==d&&(a==2&&b==2)&&kingsideok==1&&castlingcheck==0&&CheckWhite(blacklist)==0) {
        castling = 1;
    }
    // printf("castling: %d\n",castling);
    if(a==c&&b==d&&castling==0) return 0;
    if(ValidSquare(a,b,c,d) == 0) return 0;
    if(board[a][b]<7&&castling==0) return 0;
    for(i=0; i<218; i++) {
        if(num==*(list+i)) {
            result = 1;
            break;
        }

    }
    return result;
}


int WhiteFaultCheck(int a,int b,int c,int d)
{
    piece oldboard[8][8];
    int result = 1;
    piece captured = board[c][d];
    //  printf("\n%d %d %d %d\n",a,b,c,d);
    //printf("%captured piece is : %d",captured);
    Move(a,b,c,d);
    int *list = BlackMoveList(enpass_a,enpass_b);
    if(CheckWhite(list)==1) result = 0;
    // for(i=0;i<8;i++){
    //     for(j=0;j<8;j++){
    //         board[i][j]=oldboard[i][j];
    //     }
    // }
    //printboard();
    Move(c,d,a,b);

    board[c][d] = captured;
    //printboard();
    return result;
}
int BlackMove(char *cmd,int turn,int castlingcheck)
{
    int a,b,c,d;
    int num;
    int i;
    int *list = BlackMoveList(enpass_a,enpass_b);
    int *whitelist = WhiteMoveList(enpass_a,enpass_b);
    int castling=0;
    int queensideok=1;
    int kingsideok=1;
    int x,y;
    for(x=0; x<8; x++) {
        if(queensideok==0) break;
        for(y=0; y<8; y++) {
            if(queensideok==0) break;
            for(i=0; i<218; i++) {
                int val1=x*1000 + y*100 + 71;
                int val2=x*1000 + y*100 + 72;
                int val3=x*1000 + y*100 + 73;
                //  printf("i:%d | val3: %d | %d \n",i,val3,blacklist[i]);
                if(val1==whitelist[i]||val2==whitelist[i]||val3==whitelist[i]) {
                    queensideok=0;
                    break;
                }
            }
        }
    }
    for(x=0; x<8; x++) {
        if(kingsideok==0) break;
        for(y=0; y<8; y++) {
            if(kingsideok==0) break;
            for(i=0; i<218; i++) {
                int val1=x*1000 + y*100 + 75;
                int val2=x*1000 + y*100 + 76;
                //  printf("i:%d | val3: %d | %d \n",i,val3,blacklist[i]);
                if(val1==whitelist[i]||val2==whitelist[i]) {
                    kingsideok=0;
                    break;
                }
            }
        }
    }
    // printf("queenside: %d\n",queensideok);

    int result=0;

    if(turn == 0) return 0;
    if(strlen(cmd)!=4) return 0;
    a = cmd[0]-'0';
    b = cmd[1]-'0';
    c = cmd[2]-'0';
    d = cmd[3]-'0';
    num = a*1000 + b*100 + c *10 +d;
    if(a==c&&b==d&&a==6&&b==6&&queensideok==1&&castlingcheck==0&&CheckBlack(whitelist)==0) {
        castling = 1;
    }
    if(a==c&&b==d&&a==7&&b==7&&kingsideok==1&&castlingcheck==0&&CheckBlack(whitelist)==0) {
        castling = 1;
    }
//   printf("castling: %d\n",castling);
    if(ValidSquare(a,b,c,d) == 0) return 0;
    //  printf("2nd: %d\n",result);
    if(board[a][b]>6&&castling==0) return 0;
    for(i=0; i<218; i++) {
        if(num==*(list+i)) {
            result = 1;
            break;
        }

    }
    return result;
}

int BlackFaultCheck(int a,int b,int c,int d)
{
    piece oldboard[8][8];
    int result = 1;
    piece captured = board[c][d];
    Move(a,b,c,d);
    int *list = WhiteMoveList(enpass_a,enpass_b);
    if(CheckBlack(list)==1) result = 0;;
    Move(c,d,a,b);
    board[c][d] = captured;
    return result;
}       //0 ~ fault,1 ~ not fault

int WhiteMate(int *whitelist)
{
    int result = 1;
    int i;
    int val;
    int a,b,c,d;
    for(i=0; i<218; i++) {
        val = *(whitelist+i);
        a = val/1000;
        b = val/100 - a*10;
        c = val/10 - a*100 - b*10;
        d = val%10;
        if(WhiteFaultCheck(a,b,c,d)!=0) {
            result = 0;
            break;
        }
    }
    return result;
}

int BlackMate(int *blacklist)
{
    int result = 1;
    int i;
    int val;
    int a,b,c,d;
    for(i=0; i<218; i++) {
        val = *(blacklist+i);
        a = val/1000;
        b = val/100 - a*10;
        c = val/10 - a*100 - b*10;
        d = val%10;
        if(BlackFaultCheck(a,b,c,d)!=0) {
            result = 0;
            break;
        }
    }
    return result;
}

/// 0: white - receive invite, 1: black - send invite

int updateList(int *list1,int *list2)
{
    list1 = WhiteMoveList(enpass_a,enpass_b);
    list2 = BlackMoveList(enpass_a,enpass_b);
    return 0;
}

///0 - draw;
///1 - white won by mate;
///2 - black won by mate;
///3 - black won by fault;
///4 - white won by fault
///5 - black won - white quited
///6 - white won - black quited
int PlayGame(int side)
{
    setup();
    printboard();
    int tmp;
    int blackfault=0;
    int whitefault=0;
    int gameresult=0; //0 - draw; 1 - white won by mate; 2 - black won by mate; 3 - black won by fault; 4 - white won by fault
    int a,b,c,d;
    int wka,wkb,bka,bkb;
    int check; //king's position
    int blackcastled=0;
    int whitecastled=0;
    char command[4];
    int *whitelist = WhiteMoveList(enpass_a,enpass_b);
    int *blacklist = BlackMoveList(enpass_a,enpass_b);
    int turn = 0; //0 is white,1 is black
    while(gameresult == 0) {
        //   for(i=0;i<218;i++)
        //      {
        //          if(*(whitelist+i)==0) break;
//           if(*(whitelist+i)==1737) printf("abcxyz");
//           if(*(whitelist+i)!=0) printf("%d|",*(whitelist+i));
        //      }
        //      printf("\n");
        //      for(i=0;i<218;i++)
        //     {
        //         if(*(blacklist+i)!=0) printf("%d|",*(blacklist+i));
        //     }
        printf("\n");
        switch(turn) {
        /// white turn
        case 0:
            if(gameresult!=0) break;
            check = 0;
            for(a=0; a<8; a++) {
                if(check!=0) break;
                for(b=0; b<8; b++) {
                    if(board[a][b]==bking) {
                        bka = a;
                        bkb = b;
                        check = 1;
                        break;
                    }
                }
            }

            /// make move and send to server: 300 ~ [cordinate]
            /// white turn - I am white
            if(turn == side) {
                printf("\nIt's White's turn!! input next move: ");
                scanf("%s",command);
                while(WhiteMove(command,turn,whitecastled)==0) {
                    printf("Invalid Move!! Please make a valid move: ");
                    scanf("%s",command);
                }
                a = command[0]-'0';
                b = command[1]-'0';
                c = command[2]-'0';
                d = command[3]-'0';
                while(WhiteFaultCheck(a,b,c,d)==0) {
                    if(whitefault<3) {
                        printf("White has committed a technical fault!! White still has %d technical faults left \nPlease make another move :",3-whitefault);
                        whitefault++;
                        scanf("%s",command);
                        while(WhiteMove(command,turn,whitecastled)==0) {
                            printf("Invalid Move!! Please make a valid move: ");
                            scanf("%s",command);
                        }
                        a = command[0]-'0';
                        b = command[1]-'0';
                        c = command[2]-'0';
                        d = command[3]-'0';
                    }
                    if(whitefault==3) {
                        printf("White has committed 3 technical fault!! White has lost the game!! \n");
                        gameresult = 3;
                        break;
                    }
                }
                if(gameresult != 0) break;

                ms.command = 300;
                sprintf(buff, "%d ~ %s", ms.command, command);
                bytes_sent = send(client_sock, buff, sizeof(buff), 0);
                if(bytes_sent <= 0) {
                    printf("\nError! Cannot send data to sever!\n");
                    close(client_sock);
                    exit(-1);
                }

            }

            /// receive move from server and extract into a,b,c,d: 301 ~ [cordinate]
            /// white turn - I am black
            if(turn!=side) {
                /// set timeout 30 seconds
                //        struct timeval tv;
                //        tv.tv_sec = 30;  /* 30 Secs Timeout */
                //        tv.tv_usec = 0;
                //       setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

                /// wait for move from server for 30s
                printf("\nWaiting for opponent's move\n");
                bytes_received = recv(client_sock,buff,1024,0);
                if(bytes_received <= 0) {
                    /// send to server I win
                    sprintf(buff, "%d", 201);
                    bytes_sent = send(client_sock, buff, sizeof(buff), 0);
                    if(bytes_sent <= 0) {
                        printf("\nError! Cannot send data to sever!\n");
                        close(client_sock);
                        exit(-1);
                    }

                }

                ms = ParseMessage(buff);

                if(ms.command == 303) {
                    /// my opponent has quited, black won cause white quited
                    gameresult = 5;
                    break;
                }

                strcpy(command,ms.message);
                a = command[0]-'0';
                b = command[1]-'0';
                c = command[2]-'0';
                d = command[3]-'0';
            }

            if(strcmp(command,"1111")==0) {
                // tmp=1;
                // printf("asdsad:%d \n",tmp);
                printf("White castled queen's side \n");
                Move(0,4,0,2);
                Move(0,0,0,3);
                updateList(whitelist,blacklist);
                turn++;
                printf("\e[2J\e[H");
                printboard();
                break;
            }
            if(strcmp(command,"2222")==0) {
                printf("White castled king's side \n ");
                Move(0,4,0,6);
                Move(0,7,0,5);
                updateList(whitelist,blacklist);
                turn++;
                printf("\e[2J\e[H");
                printboard();
                break;
            }
            if(board[a][b]==wrook || board[a][b]==wking) whitecastled =1;
            if(board[a][b]==wpawn&&c==enpass_a&&d==enpass_b) {
                Move(a,b,c,d);
                board[enpass_a-1][enpass_b]=blank;
                updateList(whitelist,blacklist);
                turn++;
                printf("\e[2J\e[H");
                printboard();
                break;
            }
            if(board[a][b]==wpawn&&a<3) {
                enpass_a = c-1;
                enpass_b = d;
            } else {
                enpass_a = -1;
                enpass_b = -1;
            }
            Move(a,b,c,d);


            turn++;
            epturn = 2;
            updateList(whitelist,blacklist);
            printf("\e[2J\e[H");
            if(BlackMate(blacklist)==1) {
                printf("White has done a mate!!\n");
                gameresult = 1;
                printf("Result: %d\n",gameresult);
                break;
            }
            printboard();
            break;

        /// black turn
        case 1:
            if(gameresult!=0) break;
            check = 0;
            for(a=0; a<8; a++) {
                if(check!=0) break;
                for(b=0; b<8; b++) {
                    if(board[a][b]==wking) {
                        wka = a;
                        wkb = b;
                        check = 1;
                        break;
                    }
                }
            }

            /// black turn - I am black
            if(turn == side) {
                printf("\nIt's Black's turn!! input next move: ");
                scanf("%s",command);
                while(BlackMove(command,turn,blackcastled)==0) {
                    printf("Invalid Move!! Please make a valid move: ");
                    scanf("%s",command);
                }
                a = command[0]-'0';
                b = command[1]-'0';
                c = command[2]-'0';
                d = command[3]-'0';
                while(BlackFaultCheck(a,b,c,d)==0) {
                    if(blackfault<3) {
                        printf("Black has committed a technical fault!! Black still has %d technical faults left \n Please make another move :",3-blackfault);
                        blackfault++;
                        scanf("%s",command);
                        while(BlackMove(command,turn,blackcastled)==0) {
                            printf("Invalid Move!! Please make a valid move: ");
                            scanf("%s",command);
                        }
                        a = command[0]-'0';
                        b = command[1]-'0';
                        c = command[2]-'0';
                        d = command[3]-'0';
                    }
                    if(blackfault==3) {
                        printf("Black has committed 3 technical fault!! Black has lost the game!! \n");
                        gameresult = 4;
                        break;
                    }
                }
                if(gameresult != 0) break;
                ms.command = 300;
                sprintf(buff, "%d ~ %s", ms.command, command);
                bytes_sent = send(client_sock, buff, sizeof(buff), 0);
                if(bytes_sent <= 0) {
                    printf("\nError! Cannot send data to sever!\n");
                    close(client_sock);
                    exit(-1);
                }
            }

            /// black turn - I am white
            if(turn != side) {
                /// set timeout 30 seconds
                //        struct timeval tv;
                //        tv.tv_sec = 30;  /* 30 Secs Timeout */
                //       tv.tv_usec = 0;
                //       setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

                /// wait for move from server for 30s
                printf("\nWaiting for opponent's move\n");
                bytes_received = recv(client_sock,buff,1024,0);
                if(bytes_received <= 0) {
                    /// send to server I win
                    sprintf(buff, "%d", 201);
                    bytes_sent = send(client_sock, buff, sizeof(buff), 0);
                    if(bytes_sent <= 0) {
                        printf("\nError! Cannot send data to sever!\n");
                        close(client_sock);
                        exit(-1);
                    }
                }

                ms = ParseMessage(buff);
                if(ms.command == 303) {
                    /// my opponent has quited, white won cause black quited
                    gameresult = 6;
                    break;
                }

                strcpy(command,ms.message);
                printf("\nNext Move is: %s\n",command);
                a = command[0]-'0';
                b = command[1]-'0';
                c = command[2]-'0';
                d = command[3]-'0';
            }
            if(strcmp(command,"6666")==0) {
                printf("Black castled queen's side \n");
                Move(7,4,7,2);
                Move(7,0,7,3);
                updateList(whitelist,blacklist);
                turn--;
                printf("\e[2J\e[H");
                printboard();
                break;
            }
            if(strcmp(command,"7777")==0) {
                printf("Black castled king's side \n ");
                Move(7,4,7,6);
                Move(7,0,7,5);
                updateList(whitelist,blacklist);
                turn--;
                printf("\e[2J\e[H");
                printboard();
                break;
            }
            if(board[a][b]==brook || board[a][b]==bking) blackcastled =1;
            if(board[a][b]==bpawn&&c==enpass_a&&d==enpass_b) {
                Move(a,b,c,d);
                board[enpass_a+1][enpass_b]=blank;
                updateList(whitelist,blacklist);
                turn--;
                printf("\e[2J\e[H");
                printboard();
                break;
            }
            if(board[a][b]==bpawn&&a>5) {
                enpass_a = c+1;
                enpass_b = d;
            } else {
                enpass_a = -1;
                enpass_b = -1;
            }
            Move(a,b,c,d);
            turn--;
            epturn = 1;
            updateList(whitelist,blacklist);
            printf("\e[2J\e[H");
            printboard();
            // printf("whitemate: %d?\n",WhiteMate(whitelist));
            if(WhiteMate(whitelist)==1) {
                printf("Black has done a mate!!\n");
                gameresult = 2;
                printf("Result: %d\n",gameresult);
                break;
            }

            break;
        }
    }
    return gameresult;
}

void SendResult(int resultCode)
{
    ms.command = 302;
    char resultText[50];
    ParseGameResult(resultCode, resultText);

    sprintf(buff, "%d ~ %s", ms.command, resultText);
    bytes_sent = send(client_sock, buff, sizeof(buff), 0);
    if(bytes_sent <= 0) {
        printf("\nError! Cannot send data to sever!\n");
        close(client_sock);
        exit(-1);
    }
}

int ReceiveLog()
{
    char logFileName[50];

    /// receive log file name
    memset(buff, '\0', (strlen(buff)+1));
    bytes_received = recv(client_sock,buff,sizeof(buff),0);
    if (bytes_received <= 0) {
        printf("Error! Can not receive data from server!\n");
        close(client_sock);
        exit(-1);
    }
    buff[bytes_received] = '\0';
    strcpy(logFileName, buff);

    /// send file name confirm
    bytes_sent = send(client_sock, logFileName, sizeof(logFileName), 0);
    if (bytes_sent < 0) {
        printf("Error! Can not sent data to client!\n");
        close(client_sock);
        return 1;
    }

    /// get file size from server
    bytes_received = recv(client_sock,buff,sizeof(buff),0);
    if (bytes_received <= 0) {
        printf("Error! Can not receive data from server!\n");
        close(client_sock);
        exit(-1);
    }
    buff[bytes_received] = '\0';
    int file_size = atoi(buff);
    printf("Log file size from server: %d bytes\n", file_size);

    /// send confirm file size received
    char tmp[50];
    sprintf(tmp,"%d",file_size);
    bytes_sent = send(client_sock, tmp, sizeof(tmp), 0);
    if (bytes_sent < 0) {
        printf("Error! Can not sent data to client!\n");
        close(client_sock);
        return 1;
    }

    /// Get chunks of input file from server
    int remain = file_size, ret;
    char *buffer = (char*)malloc(sizeof(char)*file_size);
    buffer[0] = '\0';
    memset(buff, '\0', (strlen(buff)+1));
    while(remain > 0) {
        ret = recv(client_sock, buff, sizeof(buff), 0);
        if(ret == -1) {
            printf("Error! Can not receive data from client!\n");
            close(client_sock);
            exit(-1);
        }
        remain -= ret;
        buff[ret] = '\0';
        strcat(buffer,buff);
    }

    /// write the result to log file
    FILE * output = fopen(logFileName, "w");
    if(!output) {
        printf("Error writing to file %s!", logFileName);
        return 1;
    }
    printf("\n%s", buffer);
    fprintf(output, "%s",buffer);

    printf("Received log file %s.", logFileName);

    free(buffer);
    fclose(output);

    return 0;
}

void FindAndPlayGame()
{
    while(acceptInvitation() == 1) {
        printf("Game on!!!");
        /// go to game phase
        int side = 0;
        int resultCode = PlayGame(side);
        char resultText[50];
        ParseGameResult(resultCode, resultText);
        SendResult(resultCode);
        ReceiveLog();

        char c;
        printf("\n\nGame end. Want to play with another player? (y/n): ");
        myFlush();
        scanf("%c", &c);
        if(c=='n')
            break;
    }
}

void ParseGameResult(int resultCode, char resultText[])
{
    switch(resultCode) {
    case 0:
        strcpy(resultText, "\nDraw game!!\n");
        break;
    case 1:
        strcpy(resultText, "\nWhite won by mate!!\n");
        break;
    case 2:
        strcpy(resultText, "\nBlack won by mate!!\n");
        break;
    case 3:
        strcpy(resultText, "\nWhite won by fault - Black has committed 3 technical fault!!\n");
        break;
    case 4:
        strcpy(resultText, "\nBlack won by fault - White has committed 3 technical fault!!\n");
        break;
    case 5:
        strcpy(resultText, "\nBlack won - White has quited!!\n");
        break;
    case 6:
        strcpy(resultText, "\nWhite won - Black has quited!!\n");
        break;
    }
}
