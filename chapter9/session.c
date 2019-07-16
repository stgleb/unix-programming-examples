#include "../apue.h"
#include <sys/wait.h>

void check_controlling_terminal() {
    FILE *fd;

    fd = fopen("/dev/tty", "r");

    if(fd == NULL) {
        printf("process %d has no controlling terminal\n", getpid());
        return;
    }

    printf("process %d has controlling terminal\n", getpid());
}

int main(void){
	int sid;
	int session_id;
    int status;

	pid_t pid;

	printf("before fork\n");

	if((pid = fork()) < 0) {
		printf("fork error");
	} else if (pid == 0) {
		if((sid = setsid()) < 0){
		    printf("create session error %d\n", sid);
		    return -1;
		} else {
		    printf("child process %d session id %d\n", getpid(), getsid(getpid()));
		}

        check_controlling_terminal();
		return 0;
	}

    printf("parent process %d session id %d\n", getpid(), getsid(getpid()));
    check_controlling_terminal();

    if((waitpid(pid, &status, 0) < 0)) {
        printf("error wait for process %d", pid);
    } else {
        printf("child process has finished with %d", status);
    }

	exit(0);
}
