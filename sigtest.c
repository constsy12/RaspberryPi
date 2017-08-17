#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int cnt = 0;

void sig_handler(int signum){
	printf("Get Signal %d\n",cnt++);
	alarm(1);
}

int main(){
	signal(SIGALRM,sig_handler);
	alarm(1);

	while(cnt!=10){
		printf("##########while#########\n");
		usleep(500000);
	}
}
