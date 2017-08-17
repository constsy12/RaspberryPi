#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PAGE_SIZE (4096)
#define GPIO_BASE (volatile unsigned int)0x3F200000
#define GPFSEL0 0x04
#define GPSET0 0x1C
#define GPCLR0 0x28

void *mmaped;

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
	volatile unsigned int *addr = get_base_addr();
	int i=0;
	int j=0;

	*(addr+1) |= (1<<9);

	printf("\naddr : %x\n",*addr);

	while(1){
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
	}

//	addr[GPSET0] |= (1<<3);
		
	munmap(mmaped,PAGE_SIZE);
	return 0;
}
