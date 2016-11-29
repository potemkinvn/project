#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string.h>

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
    printf("    2. ...\n");
    printf("    3. ...\n");
    printf("    4. ...\n");
    printf("    5. ...\n");
    printf("Your choice (1-5, other to quit): ");
}


int main()
{
    int client_sock;
    char buff[1024];
    struct sockaddr_in server_addr;
    int bytes_sent,bytes_received;

    int total_bytes_sent = 0;
    char tmp[50];
    char pass[30];
    char * buffer;
    int command_code, choice,i;
    client_sock=socket(AF_INET,SOCK_STREAM,0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5550);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(client_sock,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))!=0) {
        printf("\nError!Can not connect to sever!Client exit imediately! ");
        return 0;
    }

    // 1. Receive server welcome
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
            command_code = 100;
            // 2. Send server user name
            while(command_code != 200) {
                printf("Username: ");
                scanf("%s",buff);
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
                command_code = atoi(buff);
                if(command_code == 200) {
                    printf("Username exists! Status: 200\n");
                    break;
                } else if(command_code == 100) {
                    printf("Username not exists! Remains attempt 3 times! Status: %d\n", command_code);
                } else if(command_code == 101) {
                    printf("Username not exists! Remains attempt 2 times! Status: %d\n", command_code);
                } else if(command_code == 102) {
                    printf("Username not exists! Remains attempt 1 times! Status: %d\n", command_code);
                } else if(command_code == 103) {
                    printf("Username not exists! Too many attempt! Exiting! Status: %d\n", command_code);
                    close(client_sock);
                    return 1;
                }
            }

            // 3. Receive challenge message from server
            if(command_code != 200) {
                close(client_sock);
                return 1;
            }
            bytes_received = recv(client_sock,buff,1024,0);
            if(bytes_received == -1) {
                printf("\nError! Cannot receive data from sever!\n");
                close(client_sock);
                return 1;
            }
            int challenge = atoi(buff);
            printf("Server challenge: %d\n", challenge);

            // 4. Send user hashed-challenge
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
            sprintf(buff,"%d",response);
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
            command_code = atoi(buff);
            if(command_code == 300) {
                printf("User authenticated! Exiting!\n");
            } else if(command_code == 400) {
                printf("Password not match! Exiting!\n");
            }

            if(command_code == 309) {
                close(client_sock);
                return 0;
            }
        }
    } while(choice>=1&&choice<=5);

    close(client_sock);
    return 0;
}
