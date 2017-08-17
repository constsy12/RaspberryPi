#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define PAGE_SIZE (4096)
#define GPIO_BASE (volatile unsigned int)0x3F200000
#define GPFSEL0 0x00
#define GPSET0 0x1C
#define GPCLR0 0x28
#define GPREN0 0x4C	// rising edge detect enable 0
#define GPFEN0 0x58	// falling edge detect enable 0

int cnt = 0;
int pin_read = 0;
int pin_time = 0;
unsigned int *addr = 0;

void *mmaped;

void sig_handler(int signum){
	//printf("Get Signal %d\n",cnt++);
	if(*(addr+16)&(1<<17)==1){
		printf("Event!! pin = %d\n",pin_read++);
	}else{
//		printf("no pin input?\n");
	}	
	ualarm(100,1);
}

unsigned int *get_base_addr(){
	int fd=open("/dev/mem",O_RDWR|O_SYNC);
	if(fd<0){
		printf("file open fail...\n\n");
		exit(-1);
	}

	mmaped = mmap(NULL,PAGE_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,GPIO_BASE);

	if(mmaped<0){
		printf("mmap fail...\n\n");
		exit(-1);
	}

	close(fd);
	return (unsigned int*)mmaped;
}

int main(){
	unsigned int tmp = 0x00;
	addr = get_base_addr();
	int i=0;
	int j=0;

	signal(SIGALRM,sig_handler);
	

//	*(addr) &= 0xFFFFFE3F;	// gpio pin 2  FSEL 000 >> input
	*(addr) |= (1<<9);	// gpio pin 3  FSEL 001 >> output

//	*(addr+25) |= (1<<18);	// gpio pin 2  GPHEN0 1 >> high detect en
//	*(addr+19) |= (1<<17);	// gpio pin 2  GPREN0 1 >> rising edge detect en
//	*(addr+31) |= (1<<18);	// gpio pin 2  GPAREN01 >> async.RED en
	
	*(addr+37) |= 0x01;	// GPPUD set Pull-down control
	*(addr+38) |= 1<<18;	// GPPUDCLK0
	
	printf("\naddr : %x\n",*addr);

	while(1){
		/* Send Trigger */
		pin_read = 0;
		pin_time = 0;
		*(addr+7) |= (1<<3);
		usleep(20);
		*(addr+10) |= (1<<3);
//		printf("SEND TRIGGER\n");
//########################################################################
//		printf("pin 18 : 0x%x\n",(*(addr+13))&(1<<18));
		while(1){
			pin_read++;
			tmp = (*(addr+13))&(1<<18);
//			printf("pin 18 : 0x%x\n",(*(addr+13))&(1<<18));
			if(tmp==0x40000){
       		         //	printf("Event!! pin = %d\n",pin_time++);
				pin_time++;
				usleep(2);
				//*(addr+13) &= 0x0;
			}
			if(pin_time!=0 && tmp==0x00)
				break;
		}

//		printf("pin 18 : 0x%x\n",(*(addr+13))&(1<<18));
//########################################################################*/
		printf("\n >> Result : %d cm\n\n",pin_time*340/4/58);
		usleep(60000);
	}
//		ualarm(100,i1);

//		sleep(1);
/*
		for(i=0;i<20;i++){
			*(addr+7) |= (1<<3);
			usleep(1500);
			*(addr+10) |= (1<<3);
			usleep(18500);
		}

		for(i=0;i<20;i++){
			*(addr+7) |= (1<<3);
			usleep(2000);
	
			*(addr+10) |= (1<<3);
			usleep(18000);
		}	
*/
//	}
//	while(1);		
	munmap(mmaped,PAGE_SIZE);
	return 0;
}
