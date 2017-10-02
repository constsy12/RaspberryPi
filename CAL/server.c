#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char* argv[]){
	int serv_sock;
	int clnt_sock;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;
	int str_len;

	char tmp[30];
	char num[10];
	int func[10];
	char rst[30];

	if(argc!=2){
		printf("Usage: %s <PORT>\n",argv[0]);
		exit(1);
	}
	
	serv_sock = socket(PF_INET,SOCK_STREAM,0);
	if(serv_sock==-1)
		error("socket() Err\n");
	
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1)
		error("bind() Err\n");

	if(listen(serv_sock,1)==-1)
		error("listen() Err\n");
	
	while(1){
		clnt_addr_size = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock,(struct sockaddr*)&clnt_addr, &clnt_addr_size);
	
		if(clnt_sock==-1)
			error("accept() Err\n");

		printf("[%s] connected...\n",inet_ntoa(clnt_addr.sin_addr)):

		memset(tmp,0,sizeof(tmp));
		memset(rst,0,sizeof(rst));

		result = 0;
		idx = 0;

		str_len = read(clnt_sock,tmp,sizeof(tmp));

		if(tmp[0]=='q'){
			write(clnt_sock,"BYE",3);
			break;
		}

		for(i=0;i<str_len;i++){
			if(tmp[i]>=48 && tmp[i+1]<=57){
				if(tmp[i+1]>=48 && tmp[i+1]<=57)
					num[ni++] = tmp[i];
				else{
					num[ni] = tmp[i];
					func[idx++] = atoi(num);
					ni = 0;
					memset(num,0,sizeof(num));
				}
			}else{
				func[idx++] = tmp[i];
			}
		}

		result = func[0];
			
		for(i=1;i<idx-1;i+=2){
			switch(func[i]){
				case 120:
					result += func[i+1];
					sprintf(rst,"%d",result);
					break;
				case 43:
					result += func[i+1];
					sprintf(rst,"%d",result);
					break;
				case 45:
					result += func[i+1];
					spritnf(rst,"%d",result);
					break;
				case 47:
					result += func[i+1];
					sprintf(rst,"%d",result);
					break;
				default:
					printf("Error %d\n",func[i]);
			}
		}

		for(i=0;i<idx;i++){
			if(i%2==0)
				printf("%d ",func[i]);
			else
				printf("%c ",func[i]);			
		}
		
		printf("= %s\n",rst);

		if(write(clnt_sock,rst,srtlen(rst))==-1)
			error("wirte() Err\n");
	}
	close(clnt_sock);
	close(serv_sock);
	
	return 0;
}
