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


int map[10][10] = {0};

typedef struct Position{
	int x;
	int y;
}P;

typedef struct Local{
	int no;
	int state;
	P store[5];
	int count;
	int r;
	int dist;
	int pop;
}L;

typedef struct Robot{
	char ip[10];
	int state;	// 0: unusable, 1: usable, 2: using
	int dir;
	P rpos;
	P dest;
}R;

R Robots[10]={0};
L Loc[3] = {{1,0,{{0,0},{0,0},{0,0},{0,0},{0,0}},0,0,0,0},
	    {2,0,{{0,0},{0,0},{0,0},{0,0},{0,0}},0,0,0,0},
	    {3,0,{{0,0},{0,0},{0,0},{0,0},{0,0}},0,0,0,0}};


int cnt = 0;
int pin_read = 0;
int pin_time = 0;
unsigned int *addr = 0;
int Queue[3] = {0};
int front = 0;
int rear = 0;


unsigned int tmp = 0x00;
//int r1=0,r2=0,r3=0;
//int dist1=0,dist2=0,dist3=0;
//int pop1=0,pop2=0,pop3=0;

void *mmaped;

void CallRobot(int location);
void Ultra(int num,int trig,int echo);

void sig_handler(int signum){

	/* Ultra Sensor Run */

	Ultra(1,3,18);
	Ultra(2,8,7);
	Ultra(3,23,24);

	printf("\n >> RESULT : %d cm, %d cm, %d cm\n\n",Loc[1].r,Loc[2].r,Loc[3].r);





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
		if(Queue[rear]!=0){
			CallRobot(Queue[rear]);
		}
	}

	munmap(mmaped,PAGE_SIZE);
	return 0;
}

void CallRobot(int location){
	int i;
	for(i=0;i<10;i++){
		if(Robots[i].state==1){
			Robots[i].state = 2;
			Robots[i].dest.x = Loc[location].store[Loc[location].count].x;
			Robots[i].dest.y = Loc[location].store[Loc[location].count++].y;
			
			/* clear queue */
			Queue[rear++] = 0;
			break;
		}
	}
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
			Queue[front++%3] = num;
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

/*
	switch(num){
		case 1:
			r1 = pin_time*340/4/58;
			usleep(60000);
			if(r1<10)	
				dist1++;
			
			if(dist1==5){
        
        		        if(LocalA.state==0){
                        		LocalA.state=1;
					Queue[front++%3] = num;
                        		printf(" ##### Local A push\n\n");
                		}
        		}


			
			if(dist1>=5 && r1>10){
		                pop1++;
                		if(pop1==5){
                        		pop1=0;
                        		dist1=0;
                        		LocalA.state=0;
				//	Queue[rear++%3] = 0;
                        		printf(" $$$$$ Local A pop\n\n");
                		}
        		}


			break;
		case 2:
			r2 = pin_time*340/4/58;
                        usleep(60000);
                        if(r2<10)
                                dist2++;
                        
                        if(dist2==5){

                                if(LocalB.state==0){
                                        LocalB.state=1;
					Queue[front++%3] = num;
                                        printf(" ##### Local B push\n\n");
                                }
                        }


                        
                        if(dist2>=5 && r2>10){
                                pop2++;
                                if(pop2==5){
                                        pop2=0;
                                        dist2=0;
                                        LocalB.state=0;
				//	Queue[rear++%3] = 0;
                                        printf(" $$$$$ Local B pop\n\n");
                                }
                        }
		
			break;
		case 3:
			r3 = pin_time*340/4/58;
                        usleep(60000);
                        if(r3<10)
                                dist3++;
                        
                        if(dist3==5){

                                if(LocalC.state==0){
                                        LocalC.state=1;
					Queue[front++%3] = num;
                                        printf(" ##### Local C push\n\n");
                                }
                        }

                        
                        if(dist3>=5 && r3>10){
                                pop3++;
                                if(pop3==5){
                                        pop3=0;
                                        dist3=0;
                                        LocalC.state=0;
				//	Queue[rear++%3] = 0;
                                        printf(" $$$$$ Local C pop\n\n");
                                }
                        }
	
			break;	
	}
*/
}
