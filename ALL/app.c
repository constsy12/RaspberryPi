#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>

#define MAX	10

static int m;
pid_t mypid=0;
int fd;
int front = 0;
char key = 0x00;
int page_mode = 0;


typedef struct QUEUE{
	int index;
	char str[30];
	char ex_t[30];
}Q;

Q Circular_Q[MAX] = {0};

void calculator(void);


/////////////////////////////////////////
typedef struct SUBMENU{
	int mode;
	char* device_menu[5];	
}SUB_M;

typedef struct MAINMENU{
	int mode;
	char* main_menu[4];
	SUB_M sub_menu;
}MAIN_M;
////////////////////////////////////////


MAIN_M menu_tree = {0,{"1. CARCULATOR","2. DEVICE CONTROL","3. COMMAND_HISTORY",
		       "4. EXIT"},
		    {1, {"1. LED_CONTROL","2. FND_CONTROL","3. CLCD_CONTROL",
		  	 "4. DOT_CONTROL","5. BACK TO MAIN"}}};

int Menu(){
	int loop=0;

	printf("\n **************  MENU  **************\n");
	switch(page_mode){
		case 0:
			for(loop=0;loop<4;loop++){
				printf("  %s\n",menu_tree.main_menu[loop]);
			}
			break;
		case 1:
			for(loop=0;loop<5;loop++){
				printf("  %s\n",menu_tree.sub_menu.device_menu[loop]);
			}
			break;
	}
	printf("\n ************************************\n");
	printf("\n Prompt >> ");
	scanf("%d",&m);

	return m;
}

void sig_handle(int signum){
	read(fd,&key,1);
//	printf("\nApp %d\n",key);
}

//(void) signal(SIGUSR1,sig_handle);//?

void SendQueue(int comm,int fd){
        Q tmp = {0};
        struct tm *ptime;
        time_t excute_time;
        char time_str[30];

        switch(comm){
                case 1:
                        tmp.index = 1;
                        strcpy(tmp.str,"CALCULATOR");
                        break;
                case 2:
                        tmp.index = 2;
                        strcpy(tmp.str,"LED_CONTROL");
                        break;
                case 3:
                        tmp.index = 3;
                        strcpy(tmp.str,"FND_CONTROL");
                        break;
                case 4:
                        tmp.index = 4;
                        strcpy(tmp.str,"CLCD_CONTROL");
                        break;
		case 5:
			tmp.index = 5;
			strcpy(tmp.str,"DOT_CONTROL");
			break;
		case 6:
			tmp.index = 6;
			strcpy(tmp.str,"DOT_PRINT");
			break;
		case 7:
			tmp.index = 7;
			strcpy(tmp.str,"CLCD_PRINT");
			break;
		case 8:
			tmp.index = 8;
			strcpy(tmp.str,"CLCD_NAME");
			break;
		case 9:
			tmp.index = 9;
			strcpy(tmp.str,"CALCULATOR");
			break;
        }

        time(&excute_time);
        ptime = localtime(&excute_time);
        sprintf(time_str,"%d/%d/%d %2d:%2d:%2d",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday,ptime->tm_hour,ptime->tm_min,ptime->tm_sec);
        strcpy(tmp.ex_t,time_str);
        write(fd,&tmp,sizeof(Q));
}


int main(){
	
	int i=0,j=0;
	char fnd_tmp[4];
	int fnd_send[4];

	fd = open("/dev/mydrv", O_RDWR);// use mknod to make /dev/test before
	mypid = getpid();
	write(fd,&mypid,sizeof(pid_t));

	(void) signal(SIGUSR1,sig_handle);

	while(Menu()!=0){
		switch(page_mode){
		case 0:
			switch(m){
				case 1:
					SendQueue(1,fd);
					printf("\n  CALCULATOR Running\n");
					calculator();
					break;	
				case 2:
					page_mode = 1;
					break;
				case 3:
					read(fd,&Circular_Q,sizeof(Q)*MAX);
					read(fd,&front,sizeof(int));
					printf("\n********** COMMAND HISTORY **********\n\n");
					for(i=MAX-1;i>=0;i--){
						if(Circular_Q[(i+front)%MAX].index!=0){
							printf("  [%s] %s \n",Circular_Q[(i+front)%MAX].ex_t,Circular_Q[(i+front)%MAX].str);
						}
					}
					printf("\n*************************************\n");
					break;
				case 4:
					printf("\n\n  GOOD BYE -\n\n");
					m = -1;		
					break;
				default:
					break;
			}
			break;
		case 1:
			switch(m){
				case 1:
					SendQueue(2,fd);
					ioctl(fd,0,3);
					break;
				case 2:
					SendQueue(3,fd);
					printf(" FND_INPUT Run...\n");
					printf(" >> ");	
					scanf("%s",fnd_tmp);
					memset(fnd_send,0,sizeof(fnd_send));
					j = 0;
					for(i=4-strlen(fnd_tmp);i<4;i++){
						fnd_send[i] = fnd_tmp[j++]-48;
					}
					write(fd,fnd_send,sizeof(int)*4);
					//ioctl(fd,0,4);
					break;
				case 3:
					SendQueue(4,fd);
					ioctl(fd,0,7);
					break;
				case 4:
					SendQueue(5,fd);
					ioctl(fd,0,5);
					break;
				case 5:
					page_mode = 0;
					break;
				default:
					break;
			}
			break;
		}
		if(m==-1)
			break;
	}

	close(fd);
	
	return 0;
}

void calculator(void){

	char tmp = 0x00;	
	char bu = 0x00;
	char store[4] = {0};
	int func[10] = {0};
	int s_idx=0;
	int f_idx=0;	
	int i,j;
	int val = 0;
	int mul = 1;
	int rst = 0;

	printf("\n\n  ");
	

	while(1){
//		read(fd,&key,1);
		
		if(bu==key)
			continue;
				
		if(key!=0x00){
			bu = key;
			if(key<0x0A){
				store[s_idx++] = (int)key;
				printf("%d",key);
				tmp = 0x30 | key;
//				write(fd,&key,1);	
			}else if(key==0x0C){
				val = 0;
				for(i=0;i<s_idx;i++){
					mul = 1;
					for(j=i+1;j<s_idx;j++){
						mul *= 10;
					}
					val += store[i]*mul;
				}
				func[f_idx++] = val;

				rst = func[0];
				for(i=1;i<f_idx;i+=2){
					if(func[i]==43){
						rst += func[i+1];
					}
					if(func[i]==45){
						rst -= func[i+1];
					}
				}
				printf("= %d",rst);

				tmp = 61;
//				write(fd,&tmp,1);
				
				memset(func,0,sizeof(func));
				memset(store,0,sizeof(store));
				
				f_idx = 0;
				s_idx = 0;

				break;
			
			}else{
				val = 0;
				for(i=0;i<s_idx;i++){
                                        mul = 1;
                                        for(j=i+1;j<s_idx;j++){
                                                mul *= 10;
                                        }
                                        val += store[i]*mul;
                                }

				func[f_idx++] = val;
				if(key==0x0A){
					func[f_idx++] = 43;
					tmp = 43;
					printf("+");
				}
				if(key==0x0B){
					func[f_idx++] = 45;
					tmp = 45;
					printf("-");
				}
				s_idx = 0;
				memset(store,0,sizeof(store));
		
//				write(fd,&tmp,1);
			}
		}else{
			bu = 0x00;
		}
	}
	printf("\n");
}
