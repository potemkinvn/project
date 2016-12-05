#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string.h>

typedef struct message {
    int command;
    char message[1024];
} message;

void myFlush()
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

void printMenu()
{
    printf("\n\nChess Multiplayer\n");
    printf("-------------------------------------\n");
    printf("    1. Login\n");
    printf("    2. List player\n");
    printf("    3. Connect player\n");
    printf("    4. ...\n");
    printf("    5. ...\n");
    printf("Your choice (1-5, other to quit): ");
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

int main()
{
    int client_sock;
    char buff[1024];
    struct sockaddr_in server_addr;
    int bytes_sent,bytes_received;

    int total_bytes_sent = 0;
    char name[50];
    char pass[30];
    char * buffer;
    int command_code, choice,i;
    client_sock=socket(AF_INET,SOCK_STREAM,0);
    message ms;

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
    printf("Server: %s\n",buff);

    do {
        printMenu();
        scanf("%d",&choice);
        switch (choice) {
        case 1:
            ms.command = 100;
            // 1. Send server user name
            while(command_code != 104) {
                memset(buff,'\0',(strlen(buff)+1));
                printf("Username: ");
                scanf("%s",name);
                sprintf(buff, "%d ~ %s", ms.command, name);
                bytes_sent = send(client_sock, buff, sizeof(buff), 0);
                if(bytes_sent == -1) {
                    printf("\nError! Cannot send data to sever!\n");
                    close(client_sock);
                    return 1;
                }

                bytes_received = recv(client_sock, buff, sizeof(buff),0);
                if(bytes_received == -1) {
                    printf("\nError! Cannot receive data from server!\n");
                    close(client_sock);
                    return 1;
                }

                ms = ParseMessage(buff);
                if(ms.command == 104 || ms.command == 107) {
                    break;
                } else if(ms.command == 100) {
                    printf("Username not exists! Remains attempt 3 times! Status: %d\n", ms.command);
                } else if(ms.command == 101) {
                    printf("Username not exists! Remains attempt 2 times! Status: %d\n", ms.command);
                } else if(ms.command == 102) {
                    printf("Username not exists! Remains attempt 1 times! Status: %d\n", ms.command);
                } else if(ms.command == 103) {
                    printf("Username not exists! Too many attempt! Exiting! Status: %d\n", ms.command);
                    close(client_sock);
                    return 1;
                }
            }

            if(ms.command == 104){
                printf("Username exists! Status: 104\n");
            }else if(ms.command == 107){
                printf("%s", ms.message);
                break;
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
            if(bytes_sent == -1) {
                printf("\nError! Cannot send data to sever!\n");
                close(client_sock);
                return 1;
            }

            // 5. Did password match
            bytes_received = recv(client_sock,buff,1024,0);
            if(bytes_received == -1) {
                printf("\nError! Cannot receive data from sever!\n");
                close(client_sock);
                return 1;
            }
            ms = ParseMessage(buff);
            if(ms.command == 107) {
                printf("User authenticated! Exiting!\n");
            } else if(ms.command == 109) {
                printf("Password not match! Exiting!\n");
            }

            if(ms.command == 400) {
                close(client_sock);
                return 0;
            }
            break;

        case 2:
            /// list all player
            ms.command = 204;
            sprintf(buff, "%d", ms.command);
            bytes_sent = send(client_sock, buff, sizeof(buff), 0);
            if(bytes_sent == -1) {
                printf("\nError! Cannot send data to sever!\n");
                close(client_sock);
                return 1;
            }

            bytes_received = recv(client_sock,buff,1024,0);
            if(bytes_received == -1) {
                printf("\nError! Cannot receive data from sever!\n");
                close(client_sock);
                return 1;
            }
            printf("List player:\n%s", buff);

            break;


        }
    } while(choice>=1&&choice<=5);

    close(client_sock);
    return 0;
}
