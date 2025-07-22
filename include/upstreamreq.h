#ifndef UPSTREAMREQ_H_p2B9agE1
#define UPSTREAMREQ_H_p2B9agE1

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_TASKS 100

typedef struct {
    void (* function)(void *);
    void * arg;
} task_t;

extern int taskCount;
extern task_t task_queue[MAX_TASKS];
extern pthread_mutex_t queueMutex;
extern pthread_cond_t taskAvailable;

typedef struct {
    int taskCount;
    struct sockaddr_in sender;
    socklen_t sender_len;
    char * buffer;
    ssize_t len;
    int upstream_sock;
} ForwardArgs;

void submitTask(void (* func)(void *), void * arg);
void * workerThread(void *arg);
void forward(void * arg);
void forwardDNSquery(void (* func)(void *), void * arg);

#endif