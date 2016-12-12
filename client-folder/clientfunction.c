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
 * \return -1 if disconnected, 0 if list success
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
    } else {
        if(bytes_received <= 0) {
            printf("\nError! Cannot receive data from sever!\n");
            close(client_sock);
            exit(-1);
        }
        printf("You get invitation to play from player: %s\n\n", buff);
    }
    return 0;
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
    struct timeval tv;
    tv.tv_sec = 10;  /* 30 Secs Timeout */
    tv.tv_usec = 0;
    setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

    /// wait for result: acceptance from player B or failed
    printf("Waiting response from player '%s' for 15 seconds ...\n", name);
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
    getMyInvitationList();
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
    }else if(ms.command == 204){
        /// can't send accept to A, maybe A disconnected
        printf("Can't connect to '%s'. Maybe '%s' disconnected.\n", name, name);
        printf("Press any key will go back to MAIN MENU.\n");
        myFlush();
        getchar();
        return 0;
    }
    return 0;
}
