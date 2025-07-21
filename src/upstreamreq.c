#include "upstreamreq.h"
#include "configreader.h"

extern int sockfd;
extern struct sockaddr_in upstream_addr[N_UPSTREAM_DNS];

void forwardDNSquery(void (* func)(void *), void * arg) {
    pthread_mutex_lock(&queueMutex);
    if (taskCount < MAX_TASKS) {
        task_queue[taskCount].function = func;
        task_queue[taskCount].arg = arg;
        taskCount++;
        pthread_cond_signal(&taskAvailable);
    }
    pthread_mutex_unlock(&queueMutex);
}

void * workerThread(void *arg) {
    int upstream_sock = (int) (intptr_t) arg;
    while (1) {
        pthread_mutex_lock(&queueMutex);
        while (taskCount == 0) {
            pthread_cond_wait(&taskAvailable, &queueMutex);
        }
        task_t task = task_queue[--taskCount];
        pthread_mutex_unlock(&queueMutex);
        ((ForwardArgs *)task.arg)->upstream_sock = upstream_sock;
        task.function(task.arg);
    }
    return NULL;
}

void forward(void * arg) {
    ForwardArgs * args = (ForwardArgs *) arg;
    if (sendto(args->upstream_sock, args->buffer, args->len, 0,  (struct sockaddr *) &upstream_addr[args->taskCount % 3], sizeof(upstream_addr[args->taskCount % 3])) < 0) {
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

    if (sendto(sockfd, resp_buffer, resp_len, 0, (struct sockaddr *) & args->sender, args->sender_len) < 0) {
        perror("sendto client");
        close(args->upstream_sock);
        exit(EXIT_FAILURE);;
    }

    free(args->buffer);
    free(args);
}