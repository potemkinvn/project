#include "serverfunction.h"

extern node*       top;
extern node*       user;
extern int opt;
extern int listen_sock;
extern int addrlen;
extern int new_sock;
extern playerType player[30];
extern int max_clients;
extern int activity;
extern int i;
extern int sd;
extern int max_sd;
extern struct sockaddr_in address;
extern int bytes_sent,bytes_received;
extern char buff[1024];
extern message ms;
extern fd_set readfds;
extern FILE *input;


/** \brief Reset this player info, due to this player disconnected
 *
 * \param playerType *pl :this player pointer
 * \return
 *
 */
void ResetPlayer(playerType *pl)
{
    strcpy(pl->username, "");
    pl->sockdes = 0;
    pl->challenge = -9999;
    pl->isAvailable = 0;
    pl->opponent = NULL;
    pl->isLogged = 0;
    pl->inviteSockdes = 0;
}

void myFlush()
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

/** \brief Push this user to user list from file
 *
 * \param FILE *input :file descriptor
 * \param node **top :user list
 * \return
 *
 */
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

/** \brief Find node user pointer on user name
 *
 * \param node **top :user list
 * \param char buff[] :user name
 * \return user node pointer on match name or null if none
 *
 */
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

/** \brief Get connected player index on user name
 *
 * \param char username[] :user name to find
 * \param playerType player[30] :array of all player in server
 * \return
 *
 */
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

/** \brief Test username function
 *
 * \param
 * \return
 *
 */
void TestUsername()
{
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
            return;
        }
        return;
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
                return;
            }
            return;
        }
    }
    if(hasLoggedInAnotherSession)
        return;

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
            return;
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
            return;
        }
    }
}

/** \brief Retry login more than 3 times, disconnect this player
 *
 * \param
 * \param
 * \return
 *
 */
void RetryUsernameExceed()
{
    printf("User tried login more than 3 times. Closing!\n");
    close(sd);
    ResetPlayer(&player[i]);
}

/** \brief Send (to user) and process response (from user)
 *
 * \param
 * \param
 * \return send to user 107 if password match, 108 if password not match
 *
 */
void SendAndProcessResponse()
{
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
            return;
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
}

/** \brief Receive client A send invitation request client B: 200 ~ [B's name]
 *
 * \param
 * \param
 * \return result: log clientA.inviteSockes is client B's sockdes
 *          or (clientB not exist or clientA send request to clientA) send A 209
 *
 */
void ReceiveInvitationRequest()
{
    int playerBIndex = GetPlayerIndexOnUsername(ms.message, player);
    if(player[playerBIndex].sockdes == -9999 || player[playerBIndex].sockdes == player[i].sockdes) {
        /// clientB not exist or clientA send request to clientA
        ms.command = 209;
        sprintf(buff,"%d ~ User not exists or you send invite to yourself.",ms.command);
        bytes_sent = send(sd,buff,sizeof(buff),0);
        if (bytes_sent <= 0) {
            printf("Error! Can not sent data to client!\n");
            close(sd);
            ResetPlayer(&player[i]);
        }
        player[i].inviteSockdes = 0; // playerA end invitation process
    } else {
        /// log clientA.inviteSockes is client B's sockdes
        player[i].inviteSockdes = player[playerBIndex].sockdes;
    }
}

/** \brief Receive B accept A's invite: 202 ~ [A's name]
 *          start step 2 in 3 ways handshake:
 *          forward it (the info that B accepted) to A: 203 ~ [B's name]
 * \param
 * \param
 * \return
 *
 */
void StartStep2In3WaysHandshake()
{
    ms.command = 203;
    int playerAIndex = GetPlayerIndexOnUsername(ms.message, player);
    sprintf(buff,"%d ~ %s",ms.command, player[i].username);
    bytes_sent = send(player[playerAIndex].sockdes,buff,sizeof(buff),0);
    if (bytes_sent <= 0) {
        printf("Error! Can not sent data to client!\n");
        close(player[playerAIndex].sockdes);
        ResetPlayer(&player[playerAIndex]);

        // can't forward accept to A, send this info back to B
        ms.command = 204;
        sprintf(buff,"%d",ms.command);
        bytes_sent = send(sd,buff,sizeof(buff),0);
        if (bytes_sent <= 0) {
            printf("Error! Can not sent data to client!\n");
            close(sd);
            ResetPlayer(&player[i]);
        }
    }
}

/** \brief Start step 3 in 3 ways handshake:
 *          receive A ready (ack) status: 206 ~ [B's name]
 *          forward it to B: 206 ~ [A's name]
 * \param
 * \param
 * \return
 *
 */
void StartStep3In3WaysHandshake()
{
    /// receive A ready (ack) status: 206 ~ [B's name]
    /// forward it to B: 206 ~ [A's name]
    ms.command = 206;
    int playerBIndex = GetPlayerIndexOnUsername(ms.message, player);
    sprintf(buff,"%d ~ %s",ms.command, player[playerBIndex].username);
    bytes_sent = send(player[playerBIndex].sockdes,buff,sizeof(buff),0);
    if (bytes_sent <= 0) {
        printf("Error! Can not sent data to client!\n");
        close(player[playerBIndex].sockdes);
        ResetPlayer(&player[playerBIndex]);
    }
}

/** \brief List all players
 *
 * \param
 * \param
 * \return
 *
 */
void ListAllPlayers()
{
    memset(buff,'\0',(strlen(buff)+1));
    char tmp[100];
    strcpy(buff, "");
    int j;
    for(j = 0; j<30; j++) {
        if(player[j].isAvailable == 1 && player[j].isLogged == 1 && player[j].sockdes == player[i].sockdes) {
            sprintf(tmp,"\t(available) %s (you)\t\n", player[j].username);
            strcat(buff, tmp);
        } else if(player[j].isAvailable == 1 && player[j].isLogged == 1 && player[j].sockdes > 0) {
            sprintf(tmp,"\t(available) %s \n", player[j].username);
            strcat(buff, tmp);
        } else if(player[j].isAvailable == 0 && player[j].isLogged == 1 && player[j].sockdes > 0) {
            sprintf(tmp,"\t( playing ) %s \n", player[j].username);
            strcat(buff, tmp);
        }
    }
//                    printf("%s\n",buff);
    bytes_sent = send(sd,buff,sizeof(buff),0);
    if (bytes_sent <= 0) {
        printf("Error! Can not sent data to client!\n");
        close(sd);
        ResetPlayer(&player[i]);
        return;
    }
}

/** \brief Get this player invitation list
 *
 * \param
 * \param
 * \return
 *
 */
void GetInvitationList()
{
    /// get this player invitation list
    memset(buff,'\0',(strlen(buff)+1));
    char tmp[100];
    strcpy(buff, "");
    int j;
    for(j = 0; j<30; j++) {
        if(player[j].isAvailable == 1 && player[j].inviteSockdes == player[i].sockdes) {
            sprintf(tmp,"%s ", player[j].username);
            strcat(buff, tmp);
        }
    }
//                    printf("%s\n",buff);
    bytes_sent = send(sd,buff,sizeof(buff),0);
    if (bytes_sent <= 0) {
        printf("Error! Can not sent data to client!\n");
        close(sd);
        ResetPlayer(&player[i]);
        return;
    }
}
