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

struct Node* start =NULL; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int clients[100];
int n = 0;

void slice_str(const char * str, char * buffer, size_t start, size_t end){
    size_t j = 0;
    for ( size_t i = start; i <= end; ++i ) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}
struct client_info {
	int sockno;
	char ip[INET_ADDRSTRLEN];
};
struct Node{
	struct client_info client;
	struct Node* next;
	struct Node* prev;
};
void sendtoeveryone(char *msg,int curr){
	int i;
	pthread_mutex_lock(&mutex);
	struct Node* temp = start ;
	for(i = 0; i < n; i++) {
		if(temp->client.sockno!=curr){
			if(send(temp->client.sockno,msg,strlen(msg),0) < 0) {
				perror("sending failure");
				continue;
			}
		}
		temp = temp->next;
	}
	pthread_mutex_unlock(&mutex);
}
void sendtoself(char *msg,int curr){
	int i;
	pthread_mutex_lock(&mutex);
	struct Node* temp = start ;
	char snd[1000];
	memset(snd,'\0',sizeof(snd));
	sprintf(snd,"Total no. of clients connected %d",n);
	if(send(curr,snd,strlen(snd),0) < 0) {
		perror("sending failure");
	}
	for(i = 0; i < n; i++) {
		memset(snd,'\0',sizeof(snd));
		// if(temp->client.sockno!=curr){
		sprintf(snd,"Client %d is connected at socket no. %d\n",i,temp->client.sockno);
		// strcpy(snd,"Client ");
		// strcat(snd,itoa(i));
		// strcat(snd," is connected at socket no ");
		// strcat(snd,itoa(temp->client.sockno));
		if(send(curr,snd,strlen(snd),0) < 0) {
			perror("sending failure");
			continue;
		}
		temp = temp->next;
	}
	pthread_mutex_unlock(&mutex);
}
void sendtospecific(char *msg,int curr,int no){
	pthread_mutex_lock(&mutex);
	if(send(clients[no-1],msg,strlen(msg),0) < 0) {
		perror("error sending message on socket");
	}
	pthread_mutex_unlock(&mutex);
}
void *recieve_message(void *sock){
	struct client_info cl = *((struct client_info *)sock);
	char msg[500];
	// char *temp;
	int len,i,j;
	char msg2[500];
	while((len = recv(cl.sockno,msg,500,0)) > 0) {
		// msg[len] = '\0';
		// char * temp = msg[0];
		if(strchr(msg,'#')!=NULL){
			// printf("lllll\n");
			slice_str(msg,msg,1,strlen(msg)-1);
			sendtoeveryone(msg,cl.sockno);
		}
		else if(strcmp(msg,"activeuserslist")==0){
			sendtoself(msg,cl.sockno);
		}
		else{
			slice_str(msg,msg2,0,0);
			// printf("%s\n",msg2);
			slice_str(msg,msg,1,strlen(msg)-1);
			sendtospecific(msg,cl.sockno,atoi(msg2));
		}
		memset(msg,'\0',sizeof(msg));
	}
	pthread_mutex_lock(&mutex);
	printf("client at socket no. %d disconnected\n",cl.sockno);
	for(i = 0; i < n; i++) {
		if(clients[i] == cl.sockno) {
			j = i;
			for(j=i;j<(n-1);j++){
				clients[j] = clients[j+1];
				j++;
			}
		}
	}
	n--;
	printf("Total no of clients connected : %d\n",n);
	pthread_mutex_unlock(&mutex);
}
int main(){
	struct sockaddr_in my_addr,their_addr;
	int server_sock,client_sock,len;
	socklen_t their_addr_size;
	pthread_t sendt,recvt;
	char msg[500];
	struct client_info cl;
	char ip[INET_ADDRSTRLEN];
	server_sock = socket(AF_INET,SOCK_STREAM,0);
	memset(my_addr.sin_zero,'\0',sizeof(my_addr.sin_zero));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT);
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	their_addr_size = sizeof(their_addr);
	if(bind(server_sock,(struct sockaddr *)&my_addr,sizeof(my_addr)) != 0) {
		perror("binding on socket unsuccessful");
		exit(1);
	}
	if(listen(server_sock,5) != 0) {
		perror("listen on socket unsuccessful");
		exit(1);
	}
	start = (struct Node*)malloc(sizeof(struct Node));
	struct Node* temp = start;

	while(1) {
		if((client_sock = accept(server_sock,(struct sockaddr *)&their_addr,&their_addr_size)) < 0) {
			perror("accept unsuccessful");
			exit(1);
		}
		pthread_mutex_lock(&mutex);
		inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);
		printf("client at socket no. %d connected\n",cl.sockno);
		cl.sockno = client_sock;
		strcpy(cl.ip,ip);
		temp->client.sockno = cl.sockno;
		strcpy(temp->client.ip , cl.ip);
		temp->next = (struct Node*)malloc(sizeof(struct Node));
		temp = temp->next;
		clients[n] = client_sock;
		n++;
		printf("Total no of clients connected : %d\n",n);
		pthread_create(&recvt,NULL,recieve_message,&cl);
		pthread_mutex_unlock(&mutex);
	}
	return 0;
}