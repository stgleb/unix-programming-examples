#include "../apue.h"
#include <pthread.h>

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;


void prepare(void) {
    int err;

    printf("preparing locks...\n");
    if((err = pthread_mutex_lock(&lock1)) != 0) {
        printf("can't lock lock1 in prepare handler %d", err);
    }

    if((err = pthread_mutex_lock(&lock2)) != 0) {
        printf("can't lock lock2 in prepare handler %d", err);
    }
}

void parent(void) {
    int err;

    printf("parent unlocking locs...\n");
    if((err = pthread_mutex_unlock(&lock1)) != 0) {
        printf("can't unlock lock1 in parent handler %d", err);
    }

    if((err = pthread_mutex_unlock(&lock2)) != 0) {
        printf("can't unlock lock2 in parent handler %d", err);
    }
}

void child(void) {
    int err;

    printf("child unlocking locs...\n");
    if((err = pthread_mutex_unlock(&lock1)) != 0) {
        printf("can't unlock lock1 in child handler %d", err);
    }

    if((err = pthread_mutex_unlock(&lock2)) != 0) {
        printf("can't unlock lock2 in child handler %d", err);
    }
}

void* thread_fn(void *arg) {
    printf("thread started...\n");
    pause();
    return(0);
}

int main(void) {
    int err;
    pid_t pid;
    pthread_t tid;

    if((err = pthread_atfork(prepare, parent, child)) != 0) {
        printf("can't install fork handlers %d", err);
    }

    if((err = pthread_create(&tid, NULL, thread_fn, 0)) != 0) {
        printf("can;t create thread %d", err);
    }

    sleep(2);
    printf("parent is about to fork...\n");

    if((pid == fork()) < 0) {
        printf("fork failed %d", err);
    } else if(pid == 0) {
        printf("child returned from fork");
    } else {
        printf("parent returned from fork");
    }

    exit(0);
}