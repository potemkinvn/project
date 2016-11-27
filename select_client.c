#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string.h>

int main(){
	int client_sock;
    char buff[1024];
    struct sockaddr_in server_addr;
    int bytes_sent,bytes_received;

    int total_bytes_sent = 0;
    char tmp[50];
    char * buffer;
	client_sock=socket(AF_INET,SOCK_STREAM,0);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5550);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");



	if(connect(client_sock,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))!=0){
		printf("\nError!Can not connect to sever!Client exit imediately! ");
		return 0;
	}

	bytes_received = recv(client_sock,buff,1024,0);
	if(bytes_received == -1){
		printf("\nError!Cannot receive data from sever!\n");
		close(client_sock);
		exit(-1);
	}
	printf("\nInsert string to send:");
	memset(buff,'\0',(strlen(buff)+1));
	gets(buff);
	bytes_sent = send(client_sock,buff,strlen(buff),0);

	if(bytes_sent == -1){
		printf("\nError!Cannot send data to sever!\n");
		close(client_sock);
		exit(-1);
	}
	close(client_sock);
	return 0;
}
