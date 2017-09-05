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

int map[8][9] = {0};

typedef struct Position{
	int x;
	int y;
}P;

typedef struct Local{
	int no;		// location num
	int state;	// check input or output
	P store[SMAX];	// position of store
	int count;	// number of stored 
	int r;		// ultra_ range
	int dist;	// ultra_ check input
	int pop;	// ultra_ check output
}L;

typedef struct Robot{
//	char ip[10];	// client ip
	int loc;	// location num
	int sock;	// clnt_sock
	int state;	// 0: unusable, 1: usable, 2: location, 3: moving, 4: return
	int dir;	// direction
			// 0: North, 1: East, 2: South, 3: West 
			// 0: Forward, 1: Right, 2: Left, 3: Backward
	P rpos;		// real_time pos
	P base;		// base pos
	P dest;		// destination pos
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
L Loc[LMAX] = {{1,0,{{2,3},{2,4},{2,5},{2,6},{2,7}},0,0,0,0},
   	       {2,0,{{4,3},{4,4},{4,5},{4,6},{4,7}},0,0,0,0},
	       {3,0,{{6,3},{6,4},{6,5},{6,6},{6,7}},0,0,0,0}};

P start = {1,2};

int cnt = 0;
int pin_read = 0;
int pin_time = 0;
unsigned int *addr = 0;
int Queue[LMAX] = {0};
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
pthread_t thread;
pthread_mutex_t mu;

void *thread_handler(void*);

unsigned int tmp = 0x00;

void *mmaped;

void CallRobot(int location);
void Ultra(int num,int trig,int echo);



void sig_handler(int signum){

	/* Ultra Sensor Run */

	Ultra(1,3,18);
	Ultra(2,8,7);
	Ultra(3,23,24);

	printf("\n >> RESULT : %d cm, %d cm, %d cm\n\n",Loc[1].r,Loc[2].r,Loc[3].r);


	if(Queue[rear]!=0){
	        CallRobot(Queue[rear]);
        }

	alarm(1);
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

int main(){
	addr = get_base_addr();
	int check = 0;
	int i=0,j=0;
	char* auth = "auth";
	char conf[10];


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


	/* Set Signal */
	signal(SIGALRM,sig_handler);
	alarm(1);

	while(1){
		clnt_sock = accept(serv_sock,(struct sockaddr*)&clnt_addr,&clnt_addr_size);

		if(clnt_sock==-1)
			error_m("accept_error\n\n");

		printf("[%s] connected...\n",inet_ntoa(clnt_addr.sin_addr));

		/* Configuration */		
		write(clnt_sock,auth,strlen(auth));
		sleep(1);
		memset(config,'\0',sizeof(config));
		read(clnt_sock,config,10);

		if(strcmp(config,"hiserver")==0){

			/* Initializing Robot */
			pthread_mutex_lock(&mu);

			Robots[clnt_num].sock = clnt_sock;
			Robots[clnt_num].state = 1;
			Robots[clnt_num].dir = 1;
			Robots[clnt_num].rpos.x = 0;
			Robots[clnt_num].rpos.y = 2 + clnt_num;
			Robots[clnt_num].base = rpos;
			Robots[clnt_num++].dest = rpos;
		
			pthread_mutex_unlock(&mu);
			
			/* Create Sock Thread */
			pthread_create(&thread,NULL,thread_handler,(void*)clnt_num);

		}else{
			printf("  [!] Unknown Access\n\n");
			close(clnt_sock);
		}
	}


	munmap(mmaped,PAGE_SIZE);
	return 0;
}

void* thread_handler(void* num){
	int idx = (int)num;
	int sock = Robots[idx].sock;
	int len;
	char msg[100];
	char comm = 'S';
	char r_state = 'S';
	

	while(1){
	read(sock,&comm,sizeof(comm));
	
	if(comm!='G'){

	switch(Robots[idx].state){
		case 2:			// waiting
			/* base to start point */
			if(Robots[idx].rpos.x < start.x){
				//if(Robots[idx].dir!=1){
				//	comm = GetDir(idx,1);
				//}else{
				//	comm = 'F';
				//}
			
				if(map[Robots[idx].rpos.x+1][Robots[idx].rpos.y]==0){
					pthread_mutex_lock(&mu);
					map[Robots[idx].rpos.x++][Robots[idx].rpos.y] = 0;	// free
					map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;	// alloc
					comm = 'F';		
					pthread_mutex_unlock(&mu);		 	
				}

			}else{// if(Robots[idx].rpos.x==start.x){
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
			if(Robots[idx].rpos.x < Robots[idx].loc*2){
				if(map[Robots[idx].rpos.x+1][Robots[idx].rpos.y]==0){
					pthread_mutex_lock(&mu);
					map[Robots[idx].rpos.x++][Robots[idx].rpos.y]=0;
					map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;
					comm = 'F';
					Pthread_mutex_unlock(&mu);
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
					if(Loc[Robots[idx].loc-1].state==1){
						Robots[idx].state++;
					}
				}
					
			}
			
			
			break;
		case 4:			// moving
			/* location to store */
			/* landing & change dest to base */

			if(Robots[idx].rpos.y < Robots[idx].dest.y){
				if(map[Robots[idx].rpos.x][Robots[idx].rpos.y+1]==0){
					pthread_mutex_lock(&mu);
					map[Robots[idx].rpos.x][Robots[idx].rpos.y++] = 0;
					map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;
					comm = 'F';
					pthread_mutex_unlock(&mu);
				}
			}else{
				comm = 'D';
				Robots[idx].state++;
			}

			break;
		case 5:			// return to base
			/* return to base */
			if(Robots[idx].rpos.y < 8){
				if(map[Robots[idx].rpos.x][Robots[idx].rpos.y+1]==0){
					pthread_mutex_lock(&mu);
					map[Robots[idx].rpos.x][Robots[idx].rpos.y++] = 0;
					map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;
					comm = 'F';
					pthread_mutex_unlock(&mu);
				}else{
					if(Robots[idx].dir==2){
						comm = 'R';
						Robots[idx].dir = 3;
					}

				}
			}else if(Robots[idx].rpos.x > start.x){
				if(map[Robots[idx].rpos.x-1][Robots[idx].rpos.y]==0){
					pthread_mutex_lock(&mu);
					map[Robots[idx].rpos.x--][Robots[idx].rpos.y] = 0;
					map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;
					comm = 'F';
					pthread_mutex_unlock(&mu);
				}			
			}else{
				if(Robots[idx].rpos.y > Robots[idx].base.y){
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
						if(Robots[idx].rpos.x > Robots[idx].base.x){
							pthread_mutex_lock(&mu);							map[Robots[idx].rpos.x--][Robots[idx].rpos.y] = 0;
							map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;
							comm = 'B';
							Robots[idx].state = 1;
							pthread_mutex_unlock(&mu);
						}
					}

				}
			}

			break;
	}

	write(sock,comm,sizeof(comm));
	comm = 'S';	

//#######################################################################
	if(Robots[idx].state == 2){


		pthread_mutex_lock(&mu);
		
		map[Robots[idx].rpos.x][Robots[idx].rpos.y] = 1;

		pthread_mutex_unlock(&mu);
		
		
	}
//######################################################################
	}
	}
}

char GetDir(int rnum,int dir){
	char comm;

	if(rnum>dir

	

	return comm;
}

void Run(char comm){
	switch(comm){
		case 'F':
			
			break;
		case 'B':

			break;
		case 'R':

			break;
		case 'L':

			break;
	}
}

void CallRobot(int location){
	int i;
	for(i=0;i<RMAX;i++){
		if(Robots[i].state==1){
			Robots[i].state = 2;
			Robots[i].loc = location;
		//	Robots[i].dest.x = Loc[location].store[Loc[location].count].x;
		//	Robots[i].dest.y = Loc[location].store[Loc[location].count++].y;
			
			/* clear queue */
			Queue[rear++%LMAX] = 0;
			
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
	if(Loc[num].dist==5){
		if(Loc[num].state==0){
			Loc[num].state = 1;
			Queue[front++%LMAX] = num;
			printf(" ##### Local [%d] push\n\n",num);
		}
	}

	/* POP */
	if(Loc[num].dist>=5 && Loc[num].r>10){
		Loc[num].pop++;
		if(Loc[num].pop==5){
			Loc[num].pop = 0;
			Loc[num].dist = 0;
			Loc[num].state = 0;
			printf(" $$$$$ Local [%d] pop\n\n",num);
		}
	}
}
