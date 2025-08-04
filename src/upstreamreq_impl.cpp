module upstreamreq;

#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <array>

import configreader;

extern task_t task_queue[MAX_TASKS];
extern pthread_mutex_t queueMutex;
extern pthread_cond_t taskAvailable;

void forwardDNSquery(void (* func)(void *), void * arg) {
    ForwardArgs * args = (ForwardArgs *) arg;
    pthread_mutex_lock(args->queueMutex);
    if (*(args->taskCountOverall) < MAX_TASKS) {
        args->task_queue[*(args->taskCountOverall)].function = func;
        args->task_queue[*(args->taskCountOverall)].arg = arg;
        (*(args->taskCountOverall))++;
        pthread_cond_signal(args->taskAvailable);
    }
    pthread_mutex_unlock(args->queueMutex);
}

void forward(void * arg) {
    [[assume(arg != nullptr)]];
    ForwardArgs * args = (ForwardArgs *) arg;
    if (sendto(args->upstream_sock, args->buffer, args->len, 0,  (struct sockaddr *) &(args->upstream_addr.at(args->taskCount % 3)), sizeof(args->upstream_addr.at(args->taskCount % 3))) < 0) {
        perror("sendto upstream");
        close(args->upstream_sock);
        exit(EXIT_FAILURE);
    }
    char resp_buffer[BUFFER_SIZE];
    struct sockaddr_in resp_addr;
    socklen_t resp_addr_len = sizeof(resp_addr);
    ssize_t resp_len = recvfrom(args->upstream_sock, resp_buffer, sizeof(resp_buffer), 0, (struct sockaddr *) & resp_addr, & resp_addr_len);
    
    if (resp_len < 0) {
        perror("recvfrom upstream");
        close(args->upstream_sock);
        exit(EXIT_FAILURE);;
    }

    if (sendto(args->sockfd, resp_buffer, resp_len, 0, (struct sockaddr *) & args->sender, args->sender_len) < 0) {
        perror("sendto client");
        close(args->upstream_sock);
        exit(EXIT_FAILURE);;
    }

    free(args->buffer);
    delete args;
}