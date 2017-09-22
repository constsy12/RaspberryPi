#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>


#define PAGE_SIZE (4096)
#define GPIO_BASE ((volatile unsigned int)0x3F200000)
#define GPFSEL0 0x00	// select 0
#define GPFSEL1 0x04	// select 1
#define GPFSEL2 0x08	// select 2
#define GPSET0 0x1C	// PIN SET
#define GPCLR0 0x28	// PIN CLEAR
#define GPLEV0 0x34	// PIN LEVEL
#define GPPUD 0x94	// PIN PULL_UP/DOWN
#define GPPUDCLK0 0x98	// PIN PULL_UP/DOWN Enable Clock


#define RMAX 5
#define LMAX 3
#define SMAX 5
/*
int map[8][9] = {{0,0,0,1,0,1,0,1},
		 {0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0},
		 {0,0,1,0,1,0,1,0},
		 {0,0,1,0,1,0,1,0},
		 {0,0,1,0,1,0,1,0},
		 {0,0,1,0,1,0,1,0},
		 {0,0,1,0,1,0,1,0},
		 {0,0,0,0,0,0,0,0}};
*/
int map[8][9] = {{0,0,0,0,0,0,0,0,0},
		 {0,0,0,0,0,0,0,0,0},
		 {0,0,0,2,2,2,2,2,0},
		 {5,0,0,0,0,0,0,0,0},
		 {0,0,0,2,2,2,2,2,0},
		 {6,0,0,0,0,0,0,0,0},
		 {0,0,0,2,2,2,2,2,0},
		 {7,0,0,0,0,0,0,0,0}};

typedef struct Position{
	int x;
	int y;
}P;

typedef struct Local{
	int no;		// location num
	int state;	// check input or output
	int match;	// set when robot has matched
	P store[SMAX];	// position of store
	int count;	// number of stored 
	int r;		// ultra_ range
	int dist;	// ultra_ check input
	int pop;	// ultra_ check output
	int full;	// full check of store
}L;

typedef struct Robot{
//	char ip[10];	// client ip
	int loc;	// location num
	int sock;	// clnt_sock
	int state;	// 0: unusable, 1: usable, 2: location, 3: moving, 4: return
	int dir;	// direction
			// 0: North, 1: East, 2: South, 3: West 
	P rpos;		// real_time pos
}R;

/*
R Robots[RMAX]={{0,0,0,{3,0},{3,0},{3,0}},
	        {0,0,0,{4,0},{4,0},{4,0}},
	        {0,0,0,{5,0},{5,0},{5,0}},
	        {0,0,0,{6,0},{6,0},{6,0}},
	        {0,0,0,{7,0},{7,0},{7,0}}};
};
*/
R Robots[RMAX] = {0};
L Loc[LMAX] = {{1,0,0,{{2,3},{2,4},{2,5},{2,6},{2,7}},0,0,0,0,0},
   	       {2,0,0,{{4,3},{4,4},{4,5},{4,6},{4,7}},0,0,0,0,0},
	       {3,0,0,{{6,3},{6,4},{6,5},{6,6},{6,7}},0,0,0,0,0}};

P start = {1,2};

int cnt = 0;
int pin_read = 0;
int pin_time = 0;
unsigned int *addr = 0;
int Queue[LMAX] = {-1,-1,-1};
int front = 0;
int rear = 0;

/* socket */
int serv_sock;
int clnt_sock;
struct sockaddr_in serv_addr;
struct sockaddr_in clnt_addr;
int clnt_addr_size;
int clnt_num = 0;
int str_len;
char msg[50];
pthread_t thread_map;
pthread_t thread;
pthread_mutex_t mu;

void *thread_handler(void*);
void *thread_map_handler(void*);

unsigned int tmp = 0x00;

void *mmaped;

void CallRobot(int location);
void Ultra(int num,int trig,int echo);
void error_m(char* str);


void sig_handler(int signum){
	int i=0,j=0;
	/* Ultra Sensor Run */

	Ultra(0,3,18);
	Ultra(1,8,7);
	Ultra(2,23,24);

	printf("\n >> RESULT : %d cm, %d cm, %d cm\n\n",Loc[0].r,Loc[1].r,Loc[2].r);

	printf("Queue = ");
	for(i=0;i<LMAX;i++){
		printf(" %d ",Queue[i]);
	}
	printf("\n");

	if(Queue[rear]!=-1){
	        CallRobot(Queue[rear]);
        }

	for(j=0;j<9;j++){
		for(i=0;i<8;i++){
			printf(" %d ",map[i][j]);
		}
		printf("\n");
	}
	

	alarm(2);
}

unsigned int *get_base_addr(){
	int fd = open("/dev/mem",O_RDWR|O_SYNC);
	if(fd<0){
		printf("fail to open file...\n\n");
		exit(-1);
	}

	mmaped = mmap(NULL,PAGE_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,GPIO_BASE);
	if(mmaped<0){
		printf("fail to mmap...\n\n");
		exit(-1);
	}
	
	close(fd);
	return (unsigned int*)mmaped;
}

int main(int argc, char** argv){
	addr = get_base_addr();
	int check = 0;
	int i=0,j=0;
	char* auth = "auth";
	char conf[10];
	int tmp=0;

	void* thread_rst;


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

	clnt_addr_size = sizeof(clnt_addr);

	


	/* Ultra Sensor Pin Config */
	addr[GPFSEL0/4] |= (1<<9);      //GPIO_PIN_3  FSEL0 001 >> output
        addr[GPFSEL0/4] |= (1<<24);     //GPIO_PIN_8  FSEL0 001 >> output
        addr[GPFSEL2/4] |= (1<<9);      //GPIO_PIN_23 FSEL2 001 >> output
        

        addr[GPPUD/4] |= 0x01;          //GPPUD set Pull-Down Control
        addr[GPPUDCLK0/4] |= (1<<18);   //GPIO_PIN_18 CLK
        addr[GPPUDCLK0/4] |= (1<<7);    //GPIO_PIN_7  CLK
        addr[GPPUDCLK0/4] |= (1<<24);   //GPIO_PIN_23 CLK

        printf("\n addr = %x\n",*addr);

	/* thread for sending map */
	pthread_create(&thread_map,NULL,thread_map_handler,(void*)tmp);


	/* Set Signal */
	signal(SIGALRM,sig_handler);
	alarm(2);

	while(1){
		clnt_sock = accept(serv_sock,(struct sockaddr*)&clnt_addr,&clnt_addr_size);

		if(clnt_sock==-1)
			error_m("accept_error\n\n");

		printf("[%s] connected...\n",inet_ntoa(clnt_addr.sin_addr));

		/* Configuration */		
		write(clnt_sock,auth,strlen(auth));
		sleep(1);
		memset(conf,'\0',sizeof(conf));
		read(clnt_sock,conf,10);

		if(strcmp(conf,"hiserver")==0){
			printf("*** Auth complete...\n");
			/* Initializing Robot */
			pthread_mutex_lock(&mu);

			Robots[clnt_num].sock = clnt_sock;
			Robots[clnt_num].state = 1;
			Robots[clnt_num].dir = 1;
			Robots[clnt_num].rpos.x = 0;
			Robots[clnt_num].rpos.y = 3 + clnt_num;

			printf("***initialized...\n");		
			
			map[Robots[clnt_num].rpos.x][Robots[clnt_num].rpos.y]=1;

			pthread_mutex_unlock(&mu);
			
			/* Create Sock Thread */
			pthread_create(&thread,NULL,thread_handler,(void*)clnt_num);
			clnt_num++;
		}else{
			printf("  [!] Unknown Access\n\n");
			close(clnt_sock);
		}
	}
	
	pthread_join(thread,&thread_rst);
	pthread_join(thread_map,&thread_rst);
	close(clnt_sock);

	munmap(mmaped,PAGE_SIZE);
	return 0;
}
void* thread_map_handler(void* tmp){
	int sock;
	struct sockaddr_in serv_addr;
	int msg[100] = {0};
	int len = 0;
	int i=0;
	int t = 0;
	char send;
	int cnt=0;

	sock = socket(PF_INET,SOCK_STREAM,0);
	if(sock==-1)
		error_m("client sock fail\n");
	
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(5000);

	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1)
		error_m("connect_error\n");

	while(1){
		
		for(i=0;i<72;i++){
			t = map[i/9][i%9];
			switch(t){
				case 0:
					send = 'M';
					break;
				case 1:
					send = 'R';
					break;
				case 2:
					send = 'E';
					break;
				case 3:
					send = 'F';
					break;
				case 4: 
					send = 'W';
					break;
				case 5:
					send = 'A';
					break;
				case 6:
					send = 'B';
					break;
				case 7:
					send = 'C';
					break;
			}
			
				write(sock,&send,sizeof(send));
				printf("%c ",send);
				if(i%9==0)
					printf("\n");
			
		}
		
/*
		memset(msg,0,sizeof(msg));
		//printf("##################");
		for(i=0;i<72;i++){
			msg[i] = map[i/9][i%9];
		//	printf("%d ",msg[i]);
		}
		//printf("##################");
		write(sock,msg,sizeof(msg));
*/
		
				
		cnt++;
		printf("##   %d    ##",cnt);
		sleep(5);
	}

	close(sock);
	
}
void* thread_handler(void* num){
	int idx = (int)num;
	int sock = Robots[idx].sock;
	int len;
	char msg[100];
	char comm = 'G';
	char r_state = 'G';
	char test[100];
	
	printf("\n>> thread running...\n");	

	while(1){
	read(sock,&comm,1);
	printf("*** Got %c\n\n",comm);	
	comm = 'G';

	if(comm=='E'){
		printf("*** Got E\n");
		comm = 'G';
	}

	if(comm=='G'){

	switch(Robots[idx].state){
		case 1:
			comm = 'W';
			break;
		case 2:			// waiting
			/* base to start point */
			if(Robots[idx].rpos.x < start.x){
				if(map[Robots[idx].rpos.x+1][Robots[idx].rpos.y]==0){
					pthread_mutex_lock(&mu);
					map[Robots[idx].rpos.x++][Robots[idx].rpos.y] = 0;	// free
					map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;	// alloc
					comm = 'F';		
					pthread_mutex_unlock(&mu);		 					
				}

			}else{ 
				if(Robots[idx].rpos.y > start.y){
					if(Robots[idx].dir==1){
						comm = 'L';
						Robots[idx].dir = 0;
					}
					else{
						if(map[Robots[idx].rpos.x][Robots[idx].rpos.y-1]==0){
							pthread_mutex_lock(&mu);
							map[Robots[idx].rpos.x][Robots[idx].rpos.y--] = 0;
							map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;	
							comm = 'F';
							pthread_mutex_unlock(&mu);
							
						}
					}
				}else{
					comm = 'R';
					Robots[idx].dir = 1;
					Robots[idx].state++;
				}
				
			}
			break;
		case 3:			// location
			/* start point to location */
			/* change dest to store when robot departed */
			if(Robots[idx].rpos.x <= Robots[idx].loc*2){
				if(map[Robots[idx].rpos.x+1][Robots[idx].rpos.y]==0){
					pthread_mutex_lock(&mu);
					map[Robots[idx].rpos.x++][Robots[idx].rpos.y]=0;
					map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;

					comm = 'F';
					pthread_mutex_unlock(&mu);

				
				}
			}else{
				if(Robots[idx].rpos.y > 1){
					if(Robots[idx].dir == 1){
						comm = 'R';
						Robots[idx].dir = 2;
					}else{
						if(map[Robots[idx].rpos.x][Robots[idx].rpos.y-1]==0){
							pthread_mutex_lock(&mu);
							map[Robots[idx].rpos.x][Robots[idx].rpos.y--] = 0;
							map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;
							comm = 'B';
							pthread_mutex_unlock(&mu);
							
						}
					}
				}else{	// wait until loading
					comm = 'W';
					if(Loc[Robots[idx].loc-1].state==0){
						Robots[idx].state++;
						map[Loc[Robots[idx].loc-1].no*2+1][0] = Robots[idx].loc+4;

						Loc[Robots[idx].loc-1].match = 0;
					}
				}
					
			}
			
			
			break;
		case 4:			// moving
			/* location to store */
			/* landing & change dest to base */
			if(Robots[idx].rpos.y < Loc[Robots[idx].loc-1].store[Loc[Robots[idx].loc-1].count].y){
				if(map[Robots[idx].rpos.x][Robots[idx].rpos.y+1]==0){
					pthread_mutex_lock(&mu);
					map[Robots[idx].rpos.x][Robots[idx].rpos.y++] = 0;
					map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;
					comm = 'F';

					pthread_mutex_unlock(&mu);

					
				}
			}else{
				comm = 'D';
				pthread_mutex_lock(&mu);
				map[Loc[Robots[idx].loc-1].store[Loc[Robots[idx].loc-1].count].x][Loc[Robots[idx].loc-1].store[Loc[Robots[idx].loc-1].count++].y] = 3;

				pthread_mutex_unlock(&mu);
				Robots[idx].state++;
			}

			break;
		case 5:			// return to base
			/* return to base */
			if(Robots[idx].rpos.y < 8 && Robots[idx].rpos.x==Robots[idx].loc*2+1){
				if(map[Robots[idx].rpos.x][Robots[idx].rpos.y+1]==0){
					pthread_mutex_lock(&mu);
					map[Robots[idx].rpos.x][Robots[idx].rpos.y++] = 0;
					map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;
					comm = 'F';
					pthread_mutex_unlock(&mu);

					
				}
			}else if(Robots[idx].rpos.x > start.x){
				if(Robots[idx].dir==2){
					comm = 'R';
					Robots[idx].dir = 3;
				}else{ 
					if(map[Robots[idx].rpos.x-1][Robots[idx].rpos.y]==0){
						pthread_mutex_lock(&mu);
						map[Robots[idx].rpos.x--][Robots[idx].rpos.y] = 0;
						map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;
						comm = 'F';
						pthread_mutex_unlock(&mu);

						
					}
				}			
			}else{
				
				if(Robots[idx].rpos.y > idx+3){
					
					if(Robots[idx].dir==3){
						comm = 'R';
						Robots[idx].dir = 0;	
					}else{
		   
						if(map[Robots[idx].rpos.x][Robots[idx].rpos.y-1]==0){
							pthread_mutex_lock(&mu);
							map[Robots[idx].rpos.x][Robots[idx].rpos.y--] = 0;
							map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;
							comm = 'F';
							pthread_mutex_unlock(&mu);
							
						}

					}
				}else{
					if(Robots[idx].dir==0){
						comm = 'R';
						Robots[idx].dir = 1;
					}else{
						if(Robots[idx].rpos.x > 0){
							pthread_mutex_lock(&mu);
							map[Robots[idx].rpos.x--][Robots[idx].rpos.y] = 0;
							map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;
							comm = 'B';
							Robots[idx].state = 1;
							
							pthread_mutex_unlock(&mu);
							
							printf("  *** Moved completely...\n\n");
						}	
					}

				}
			}
			break;
	}
	if(comm=='G'){
		printf("!!!!!!!!!!!!!!No Command\n\n");
		comm = 'C';
	}
	sleep(4);
	write(sock,&comm,sizeof(comm));
	comm = 'S';	

	}
		
	}
}

void CallRobot(int location){
	int i;
	for(i=0;i<RMAX;i++){
	//	if(Loc[location].full==1){
	//		return;
	//	}

		if(Robots[i].state==1 && Loc[location].match==0){
			Loc[location].match=1;
			Robots[i].state = 2;
			Robots[i].loc = location+1;
			
					
			/* clear queue */
			Queue[rear] = -1;
			if((rear+1)%LMAX==0)
				rear = 0;
			else
				rear++;
		
		//	if(Loc[location].count+1==5)
		//		Loc[location].full = 1;
			printf(" *** Match Robot[%d] to Locate[%d]\n\n",i,location);
			return;
		}
	
	}
	printf("  [!] All Robots are busy now\n\n");
	return;
}

void Ultra(int num,int trig,int echo){
	/* CHECK */

	/* SEND TRIGGER */
        pin_read = 0;
        pin_time = 0;
        addr[GPSET0/4] |= (1<<trig);
        usleep(20);
        addr[GPCLR0/4] |= (1<<trig);

        /* READ PIN */
        while(1){
                pin_read++;
                tmp = (addr[GPLEV0/4])&(1<<echo);
                if(tmp==(1<<echo)){
                        pin_time++;
                        usleep(2);
                }
                if(pin_time!=0 && tmp==0x00)
                        break;
                if(pin_read>100000)
                        break;
        }

	Loc[num].r = pin_time*340/4/58;
	usleep(60000);
	if(Loc[num].r < 10)
		Loc[num].dist++;

	/* PUSH */
	if(Loc[num].dist==3){
		if(Loc[num].state==0){
			map[Loc[num].no*2+1][0] = 4;
			Loc[num].state = 1;
			if(Loc[num].full<SMAX){
				Loc[num].full++;
				Queue[front] = num;
				if((front+1)%LMAX==0)
					front = 0;
				else
					front++;
				
				printf(" ##### Local [%d] push\n\n",num);
			}else{
				printf(" [!] No more stores are left...\n\n");
			}
		}
	}

	/* POP */
	if(Loc[num].dist>=3 && Loc[num].r>10){
		Loc[num].pop++;
		if(Loc[num].pop==3){
			Loc[num].pop = 0;
			Loc[num].dist = 0;
			Loc[num].state = 0;
			printf(" $$$$$ Local [%d] pop\n\n",num);
		}
	}
}

void error_m(char* str){
	fputs(str,stderr);
	exit(1);
}

