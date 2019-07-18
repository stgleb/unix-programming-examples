#include "../apue.h"
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#define LOCKFILE "/var/run/daemon.pid"
#define LOG_FILE "/tmp/daemon.log"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

int lockfile(int fd) {
    struct flock fl;

    fl.l_type =F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    return(fcntl(fd, F_SETLK, &fl));
}

void reread() {
    syslog(LOG_INFO, "reread daemon configuration");
}

void sigterm(int signo) {
    syslog(LOG_INFO, "got SIGTERM");
    exit(0);
}

void sighup(int signo) {
    syslog(LOG_INFO, "got SIGTINT");
    reread();
}

int already_running(void) {
    int fd;
    char buf[16];

    fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);

    if(fd < 0){
        syslog(LOG_ERR, "can't open %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }

    if(lockfile(fd) < 0) {
        if(errno == EACCES || errno == EAGAIN) {
            close(fd);
            return(1);
        }
        syslog(LOG_ERR, "can't open %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }
    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);
    return(0);
}

int write_status() {
    FILE *fp;
    char *p;

    fp = fopen(LOG_FILE, "w");
    p = getlogin();

    if(fp == NULL) {
        syslog(LOG_ERR, "can't open %s to write status", LOG_FILE);
        exit(1);
    } else {
        fprintf(fp, "Process pid %d session %d\n", getpid(), getsid(getpid()));

        if(p == NULL) {
            fprintf(fp, "no login name\n");
        } else {
            fprintf(fp, "login name %s\n", p);
        }
    }

    return(0);
}

void daemonize(const char* cmd) {
    int i;
    int fd0;
    int fd1;
    int fd2;
    int sid;
    int pid;
    struct rlimit rl;
    struct sigaction sa;

    /*
        Clear file creation mask
    */
    umask(0);

    /*
        Get maximum number of files
    */
    if(getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        printf("get file limit");
        exit(1);
    }

    /*
        Fork and become session leader
    */
    if((pid = fork() < 0)) {
    } else if(pid != 0) {
    /*
        Exit from parent branch
    */
        exit(0);
    }

    /*
        Become a session leader
    */
    if((sid = setsid()) < 0){
        printf("create session error %d\n", sid);
        exit(1);
    }

    /*
        Make sure future opens won't allocate controlling tty
    */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags;

    if(sigaction(SIGHUP, &sa, NULL) < 0) {
        printf("can't ignore SIGHUP");
        exit(1);
    }

    if((pid = fork()) < 0) {
        printf("can't fork %s", cmd);
    } else if (pid != 0) {
        exit(0);
    }

    /*
        Change current working directory to the rood dir
        we won't prevent file system from beign unmounted
    */
    if(chdir("/") < 0){
        printf("can't change working dir");
        exit(1);
    }

    /*
        Close all open FDs
    */
    if(rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;

    for(i =0;i < rl.rlim_max;i++) {
        close(i);
    }

    /*
        Attach file descriptors 0,1,2 to /dev/null
    */
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    /*
        Initialize log file
    */
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if(fd0 != 0 || fd1 != 1 || fd2 != 2) {
        printf("unexpected file descriptors %d %d %d\n", fd0, fd1, fd2);
        exit(1);
    }
}

int  main(int arc, char *argv[]) {
    char *cmd;
    struct sigaction sa;

    if((cmd = strchr(argv[0], '/')) ==  NULL) {
        cmd = argv[0];
    } else {
        cmd++;
    }

    /*
        Become a daemon
    */
    daemonize(cmd);

    // write info about running daemon
    write_status();

    /*
        Make sure only one instance of daemon is running
    */
    if(already_running()){
        syslog(LOG_ERR, "daemon is already running");
        exit(1);
    }

    /*
        Handle signals of interest
    */
    sa.sa_handler = sigterm;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGHUP);
    sa.sa_flags = 0;
    if(sigaction(SIGTERM, &sa, NULL) < 0) {
        syslog(LOG_ERR, "can't catch SIGTERM: %s", strerror(errno));
        exit(1);
    }

    sa.sa_handler = sighup;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTERM);
    sa.sa_flags = 0;
    if(sigaction(SIGHUP, &sa, NULL) < 0) {
        syslog(LOG_ERR, "can't catch SIGHUP: %s", strerror(errno));
        exit(1);
    }

    pause();
}
