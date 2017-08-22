#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

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


typedef struct Local{
	int no;
	int state;
	int store[5];
	int count;
}Loc;

Loc LocalA = {0,0,{0,0,0,0,0},0};
Loc LocalB = {0,0,{0,0,0,0,0},0};
Loc LocalC = {0,0,{0,0,0,0,0},0};


int cnt = 0;
int pin_read = 0;
int pin_time = 0;
unsigned int *addr = 0;
int Queue[3] = {0};


unsigned int tmp = 0x00;
int r1=0,r2=0,r3=0;
int dist1=0;dist2=0,dist3=0;


void *mmaped;

void sig_handler(int signum){
	/* SEND TRIGGER */
        pin_read = 0;
        pin_time = 0;
        addr[GPSET0/4] |= (1<<3);
        usleep(20);
        addr[GPCLR0/4] |= (1<<3);

        /* READ PIN */
        while(1){
                pin_read++;
                tmp = (addr[GPLEV0/4])&(1<<18);
                if(tmp==0x40000){
                        pin_time++;
                        usleep(2);
                }
                if(pin_time!=0 && tmp==0x00)
                        break;
                if(pin_read>100000)
                        break;
        }

        r1 = pin_time*340/4/58;
	usleep(60000);

	if(r1<10){
		dist1++;
	}


	/* SEND TRIGGER */
        pin_read = 0;
        pin_time = 0;
        addr[GPSET0/4] |= (1<<8);
        usleep(20);
        addr[GPCLR0/4] |= (1<<8);

        /* READ PIN */
        while(1){
                pin_read++;
                tmp = (addr[GPLEV0/4])&(1<<7);
                if(tmp==0x80){
                        pin_time++;
                        usleep(2);
                }
                if(pin_time!=0 && tmp==0x00)
                        break;
                if(pin_read>100000)
                        break;
        }
        r2 = pin_time*340/4/58;
        usleep(60000);

	if(r2<10)
		dist2++;


        /* SEND TRIGGER */
        pin_read = 0;
        pin_time = 0;
        addr[GPSET0/4] |= (1<<23);
        usleep(20);
        addr[GPCLR0/4] |= (1<<23);
        /* READ PIN */
        while(1){
                pin_read++;
                tmp = (addr[GPLEV0/4])&(1<<24);
                if(tmp==0x1000000){
                        pin_time++;
                        usleep(2);
                }
                if(pin_time!=0 && tmp==0x00)
                        break;
                if(pin_read>100000)
                        break;
        }
        r3 = pin_time*340/4/58;
        usleep(60000);

	if(r3<10)
		dist3++;

	if(dist1==10){
		dist1 = 0;
		if(LocalA.state==0)
			LocalA.state=1;
	}


	printf("\n >> RESULT : %d cm, %d cm, %d cm\n\n",r1,r2,r3);

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

	signal(SIGALRM,sig_handler);
	alarm(1);

	while(1);

/*

	addr[GPFSEL0/4] |= (1<<9);	//GPIO_PIN_3  FSEL0 001 >> output
	addr[GPFSEL0/4] |= (1<<24);	//GPIO_PIN_8  FSEL0 001 >> output
	addr[GPFSEL2/4] |= (1<<9);	//GPIO_PIN_23 FSEL2 001 >> output
	

	addr[GPPUD/4] |= 0x01;		//GPPUD set Pull-Down Control
	addr[GPPUDCLK0/4] |= (1<<18);	//GPIO_PIN_18 CLK
	addr[GPPUDCLK0/4] |= (1<<7);	//GPIO_PIN_7  CLK
	addr[GPPUDCLK0/4] |= (1<<24);	//GPIO_PIN_23 CLK

	printf("\n addr = %x\n",*addr);

	while(1){

		pin_read = 0;
		pin_time = 0;
		addr[GPSET0/4] |= (1<<3);
		usleep(20);
		addr[GPCLR0/4] |= (1<<3);

		
		while(1){
			pin_read++;
			tmp = (addr[GPLEV0/4])&(1<<18);
			if(tmp==0x40000){
				pin_time++;
				usleep(2);
			}
			if(pin_time!=0 && tmp==0x00)
				break;
			if(pin_read>100000)
				break;
		}

		r1 = pin_time*340/4/58;
		usleep(60000);

		
		
                pin_read = 0;
                pin_time = 0;
                addr[GPSET0/4] |= (1<<8);
                usleep(20);
                addr[GPCLR0/4] |= (1<<8);

                
                while(1){
                        pin_read++;
                        tmp = (addr[GPLEV0/4])&(1<<7);
                        if(tmp==0x80){
                                pin_time++;
                                usleep(2);
                        }
                        if(pin_time!=0 && tmp==0x00)
                                break;
                        if(pin_read>100000)
                                break;
                }

                r2 = pin_time*340/4/58;
                usleep(60000);

		
                pin_read = 0;
                pin_time = 0;
                addr[GPSET0/4] |= (1<<23);
                usleep(20);
                addr[GPCLR0/4] |= (1<<23);

                
                while(1){
                        pin_read++;
                        tmp = (addr[GPLEV0/4])&(1<<24);
                        if(tmp==0x1000000){
                                pin_time++;
                                usleep(2);
                        }
                        if(pin_time!=0 && tmp==0x00)
                                break;
                        if(pin_read>100000)
                                break;
                }

                r3 = pin_time*340/4/58;
                usleep(60000);

		printf("\n >> RESULT : %d cm, %d cm, %d cm\n\n",r1,r2,r3);
	}	
*/
	munmap(mmaped,PAGE_SIZE);
	return 0;
}
