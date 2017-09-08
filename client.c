#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

void error_m(char* e){
	printf("\n%s\n",e);
	exit(1);
}
int main(int argc, char** argv){
	int sock;
	struct sockaddr_in serv_addr;
	char msg[100] = {0};
	int len = 0;
	char comm = 'G';
	int i = 0;

	if(argc!=3){
		printf("Usage: %s <IP> <PORT>\n",argv[0]);
		exit(1);
	}
	
	sock = socket(PF_INET,SOCK_STREAM,0);
	if(sock==-1)
		error_m("socket_error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1)
		error_m("connect_error");

	memset(msg,0,sizeof(msg));
	strcpy(msg,"hiserver");
	write(sock,msg,sizeof(msg));
	
	sleep(3);


	while(1){
	
		write(sock,&comm,1);
		printf("write %c to server\n",comm);

		len = read(sock,&comm,1);
		printf("Get %c from server\n",comm);
/*
		switch(comm){
		case 'F':
			printf("  ### Get F ###\n\n");
			comm = 'G';
	
			break;
		case 'R':
			printf("  ### Get R ###\n\n");
			comm = 'G';
			break;
		case 'L':
			printf("  ### Get L ###\n\n");
			comm = 'G';
			break;
		case 'B':
			printf("  ### Get B ###\n\n");
			comm = 'G';
			break;
		default:
			printf(" [!] Error %c \n\n",comm);
			comm = 'E';
				
		}
*/
		if(comm=='F'){
			printf("  ### Get F ###\n\n");
			comm = 'G';
		}else if(comm=='R'){
			printf("  ### Get R ###\n\n");
			comm = 'G';
		}else if(comm=='L'){
			printf("  ### Get L ###\n\n");
			comm = 'G';
		}else if(comm=='B'){
			printf("  ### Get B ###\n\n");
			comm = 'G';
		}else if(comm=='W'){
			printf("  Wait for landing...\n");	
			comm = 'G';
		}else if(comm=='D'){
			printf("  Landing...\n");
			comm = 'G';
		}else if(comm=='C'){
			printf("  Call Back...\n");
			comm = 'G';
		}else{
			printf("  Error %c\n\n",comm);
			comm = 'E';
		}
		sleep(1);		
//		sleep(3);
	}
}
