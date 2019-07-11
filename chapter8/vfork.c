#include "apue.h"

int globalVar = 6;

int main(void){
	int localVar = 88;
	pid_t pid;

	printf("before fork\n");

	if((pid = vfork()) < 0) {
		printf("fork error");
	} else if (pid == 0) {
		localVar++;
		globalVar++;
		_exit(0);
	}

	printf("pid = %d, glob = %d, local = %d\n", getpid(), globalVar, localVar);
	exit(0);
}
