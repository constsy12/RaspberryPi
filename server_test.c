#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>


int clnt_sock_arr[10];
char clnt_ip[10][30];
int clnt_num = 0;

void* clnt_func(void* sock);
void error_m(char* message);

int main(int argc, char** argv){
	int serv_sock;
	int clnt_sock;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;
	int str_len;
	char message[100];
	int quit=0;

	if(argc!=2){
		printf("Usage: %s <PORT>\n",argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET,SOCK_STREAM,0);
	if(serv_sock==-1)
		error_m("socket_err\n");
	
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1)
		error_m("bind_err\n");
	
	if(listen(serv_sock,5)==-1)
		error_m("listen_err\n");
	
//	while(1){
		clnt_addr_size = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock,(struct sockaddr*)&clnt_addr,&clnt_addr_size);
		if(clnt_sock==-1)
			error_m("accept_err\n");
		strcpy(clnt_ip[clnt_num],inet_ntoa(clnt_addr.sin_addr));
		printf("[%s] connected...\n",clnt_ip[clnt_num]);

		clnt_sock_arr[clnt_num++] = clnt_sock;
//	}
	memset(message,'\0',sizeof(message));
	sleep(5);
	str_len = read(clnt_sock,message,100);

	printf("Client>> %s\n",message);

	if(write(clnt_sock,message,str_len)==-1)
		error_m("write_err\n");

	while(quit!=5){	

	memset(message,'\0',sizeof(message));
	if(quit%2==0)
		message[0]='S';
	else
		message[0]='F';
	printf("Server>> %s\n",message);
	write(clnt_sock,message,strlen(message));
//		error_m("write_to_clnt_err\n");

	memset(message,'\0',sizeof(message));
	str_len = read(clnt_sock,message,100);
	printf("Client>> %s\n",message);
	sleep(5);
	quit++;

	}
	close(clnt_sock);
	close(serv_sock);

	return 0;
}

void error_m(char* message){
	fputs(message,stderr);
	exit(1);
}
