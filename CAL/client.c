#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char* argv[]){
	int sock;
	struct sockaddr_in serv_addr;
	char msg[30];

	int str_len;

	if(argc!=3){
		printf("Usage : %s <IP> <PORT>\n",argv[0]);
		exit(1);
	}

	while(1){
		sock = socket(PF_INET,SOCK_STREAM,0);
		if(sock==-1)
			error("sock() Err\n");

		memset(&serv_addr,0,sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
		serv_addr.sin_port = htons(atoi(argv[2]));
		
		if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1)
			error("connect() Err\n");

		memset(msg,0,sizeof(msg));
		printf("Client>> ");
		fgets(msg,sizeof(msg)-1,stdin);
		
		write(sock,msg,strlen(msg));

		memset(msg,0,sizeof(msg));

		printf("Server>> %s\n",msg);
		
		if(msg[0]=='B'){
			close(sock);
			break;
		}
	}
	return 0;
}
