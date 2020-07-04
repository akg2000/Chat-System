#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#define PORT 8081

void help_menu(){
	printf("-----------------------------------Help Menu------------------------------------\n");
	printf("1. For broadcasting a message just type \"SEND * \" followed by the message\n");
	printf("2. For sending a message to client use \"SEND (client no)\" followed by the message\n");
	printf("3. Type \"USERS\" to see available users\n");
	printf("4. Type \"HELP\" for the help menu.\n");
	printf("5. Enter \"EXIT\" to exit from the chat system.\n");
}
void *recieve_message(void *sock)
{
	int their_sock = *((int *)sock);
	char msg[500];
	int len;
	while((len = recv(their_sock,msg,500,0)) > 0) {
		msg[len] = '\0';
		fputs(msg,stdout);
		memset(msg,'\0',sizeof(msg));
	}
}
int main()
{
	struct sockaddr_in their_addr;
	int my_sock,their_sock,their_addr_size;
	pthread_t sendt,recvt;
	char msg[500];
	char username[100];
	char res[600];
	char ip[INET_ADDRSTRLEN];
	int len;

	printf("Enter your name : ");
	fgets(username,sizeof(username),stdin);
	strtok(username, "\n");
	my_sock = socket(AF_INET,SOCK_STREAM,0);
	memset(their_addr.sin_zero,'\0',sizeof(their_addr.sin_zero));
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(PORT);
	their_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(my_sock,(struct sockaddr *)&their_addr,sizeof(their_addr)) < 0) {
		perror("connection not established");
		exit(1);
	}
	strcpy(res,"#");
	strcpy(res,username);
	strcat(res," is now connected to the server");
	// len = write(my_sock,res,strlen(res));
	printf("%s\n",res );
	memset(res,'\0',sizeof(res));
	inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);
	printf("connected to server\n");
	pthread_create(&recvt,NULL,recieve_message,&my_sock);
	help_menu();
	int temp=0;
	while(1) {
		scanf("%s",msg);
		if(strcmp(msg,"SEND")==0){
			scanf("%s",msg);
			if(strcmp(msg,"*")==0){
				fgets(msg,sizeof(msg),stdin);
				// msg[strlen(msg)-1]=" ";
				// strtok(msg, "\n");
				// scanf("%[^\n]s",msg);
				// sprintf(res,"#@ %s(to all): ")
				strcpy(res,"#@");
				strcat(res,username);
				strcat(res,"(to all):");
				strcat(res,msg);
				// strcat(res,"*");
				len = write(my_sock,res,strlen(res));
			}
			else{
				// printf("msg different than *%s\n",msg );
				temp=atoi(msg);
				strcpy(res,msg);
				fgets(msg,sizeof(msg),stdin);
				strcat(res,"@");
				strcat(res,username);
				strcat(res,"(to you):");
				strcat(res,msg);
				len = write(my_sock,res,strlen(res));
			}
		}
		else if(strcmp(msg,"HELP")==0)
			help_menu();
		else if(strcmp(msg,"USERS")==0){
			strcpy(res,"activeuserslist");
			len = write(my_sock,res,strlen(res));
		}
		else if(strcmp(msg,"EXIT")==0){
			printf("Exiting.................\n");
			exit(1);
		}
		else{
			printf("Wrong input please try again\n");
			continue;
		}
		// len = write(my_sock,res,strlen(res));
		if(len < 0) {
			perror("message not sent");
			exit(1);
		}
		memset(msg,'\0',sizeof(msg));
		memset(res,'\0',sizeof(res));
	}
	pthread_join(recvt,NULL);
	close(my_sock);

}