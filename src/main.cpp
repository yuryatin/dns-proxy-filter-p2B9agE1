import <print>;
import <iostream>;
import <array>;
import <vector>;
import <ranges>;
import <cstring>;
import <cstdio>;
import <cctype>;
import <cstdlib>;

#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

import handlefilters;
import upstreamreq;
import utils;
import configreader;
import ipsender;

using std::print;
using std::println;
using std::cerr;
using std::array;
namespace vw = std::views;

const char * configFileName = NULL;
int n_threads = 0;

int sockfd;
struct sockaddr_in addr;
array<sockaddr_in, N_UPSTREAM_DNS> upstream_addr;

int taskCount = 0;
int totalTaskCount = 0;
int thread_pool_size = 24;
task_t task_queue[MAX_TASKS];
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t taskAvailable = PTHREAD_COND_INITIALIZER;

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

int main(int argc, char *argv[]) {

    if (argc > 1) configFileName = argv[1];
    else println("No path to a configuration file was provided. Proceeding with Default parameters.");

    if (argc > 2) {
        int core_n = isValidInteger(argv[2]);
        if (core_n) thread_pool_size = core_n;
    }

    Filter filter = {
        { .n=0, .domains=NULL },
        { .n=0, .domains=NULL },
        { .n=0, .records=NULL },
        { .n=0, .records=NULL }
    };
    Server server = {
        .ip = "127.0.0.1",
        .ipv6 = false,
        .port = 1053
    };
    UpStream upStream = {
        .dns = {"1.1.1.1", "8.8.8.8", "8.8.4.4"},
        .ipv6 = {false, false, false}
    };

    if (configFileName) loadFiltersAndParams(configFileName, &filter, &server, &upStream);
    printFilters(&filter);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(server.port);
    addr.sin_addr.s_addr = inet_addr(server.ip);
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    for (int i : vw::iota(0, N_UPSTREAM_DNS)) {
        upstream_addr.at(i).sin_family = AF_INET;
        upstream_addr.at(i).sin_port = htons(53);
        if (inet_pton(AF_INET, upStream.dns[i], &upstream_addr.at(i).sin_addr) <= 0) {
            print(cerr, "inet_pton for upstream dns #{}", i+1);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }

    pthread_t * threads = (pthread_t *) malloc(sizeof(pthread_t) * thread_pool_size);
    if (!threads) {
        perror("malloc for threads failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    std::vector<int> upstream_socks(thread_pool_size);

    for (auto n_thread : vw::iota(0, thread_pool_size) ) {
        upstream_socks.at(n_thread) = socket(AF_INET, SOCK_DGRAM, 0);
        if (upstream_socks[n_thread] < 0) {
            if (!n_thread) {
                perror("socket");
                free(threads);
                close(sockfd);
                exit(EXIT_FAILURE);
            } else break;
        }
        pthread_create(&threads[n_thread], NULL, workerThread, (void *) (intptr_t) upstream_socks.at(n_thread));
    }

    println("Using {} threads. Listening for UDP packets on {}:{}...", n_threads, server.ip, server.port);
    
    char readDomain[DOMLENGTH];
    char forIPv4[IPv4LEN];
    char forIPv6[IPv6LEN];
    memset(readDomain, 0, sizeof(readDomain));
    memset(readDomain, 0, sizeof(forIPv4));
    memset(readDomain, 0, sizeof(forIPv6));

    unsigned char response[BUFFER_SIZE];
    memset(response, 0, sizeof(response));
    while (1) {
        struct sockaddr_in sender;
        socklen_t sender_len = sizeof(sender);
        ssize_t len = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *) & sender, & sender_len);
        if (len < 0) {
            perror("recvfrom");
            break;
        }

        buffer[len] = '\0';

        struct DnsHeader * recv_header = (struct DnsHeader *) buffer;
        println("Query ID: {}", ntohs(recv_header->id));

        parseDomainName(buffer, readDomain);
        if(inNotFind(readDomain, &(filter.notFind))) {
            struct DnsHeader * resp_header = (struct DnsHeader *) response;
            resp_header->id = recv_header->id;

            // Setting the flags: QR=1 (response), RCODE=3 (NXDOMAIN), AA=0
            // I decided to reveal the response is non-authoritative
            resp_header->flags = htons(0x8003); // 1000 0100 0000 0011
            resp_header->qdcount = recv_header->qdcount;
            resp_header->ancount = recv_header->ancount;
            resp_header->nscount = recv_header->nscount;
            resp_header->arcount = recv_header->arcount;
            memcpy(response + DNS_HEADER_SIZE, buffer + DNS_HEADER_SIZE, len - DNS_HEADER_SIZE);
            sendto(sockfd, response, len, 0, (struct sockaddr *) &sender, sender_len);
        } else if(inRefuse(readDomain, &(filter.refuse))) {
            struct DnsHeader * resp_header = (struct DnsHeader *) response;
            resp_header->id = recv_header->id;

            // Setting the flags: QR=1 (response), RCODE=5 (REFUSED), AA=0
            // I decided to reveal the response is non-authoritative
            resp_header->flags = htons(0x8005); // 1000 0000 0000 0101
            resp_header->qdcount = recv_header->qdcount;
            resp_header->ancount = recv_header->ancount;
            resp_header->nscount = recv_header->nscount;
            resp_header->arcount = recv_header->nscount;
            memcpy(response + DNS_HEADER_SIZE, buffer + DNS_HEADER_SIZE, len - DNS_HEADER_SIZE);
            sendto(sockfd, response, len, 0, (struct sockaddr *) &sender, sender_len);
        } else {
            ForwardArgs * forwardArgs = new ForwardArgs(sockfd,
                                                        &taskCount,
                                                        upstream_addr,
                                                        task_queue,
                                                        &queueMutex,
                                                        &taskAvailable);
            
            for (auto dns : vw::iota(0, N_UPSTREAM_DNS))
                forwardArgs->upstream_addr.at(dns) = upstream_addr.at(dns);
            for (auto ta : vw::iota(0, MAX_TASKS))
                forwardArgs->task_queue[ta] = task_queue[ta];

            forwardArgs->sender_len = sender_len;
            memcpy(&forwardArgs->sender, &sender, sizeof(struct sockaddr_in));
            forwardArgs->buffer = (char *) malloc(len);
            memcpy(forwardArgs->buffer, buffer, len);
            forwardArgs->len = len;
            
            uint8_t * p0 = (uint8_t *) buffer + DNS_HEADER_SIZE;
            while (*p0 != 0) p0 += (*p0) + 1; p0 += 1;
            uint16_t qtype = ntohs(*(uint16_t *)p0);
            if(qtype == QTYPE_A && inPreDefinedIPv4(readDomain, &(filter.preDefinedIPv4), forIPv4)) {
                sendPreDefinedIP(recv_header, forIPv4, forwardArgs, false, sockfd);
            } else if(qtype == QTYPE_AAAA && inPreDefinedIPv6(readDomain, &(filter.preDefinedIPv6), forIPv6)) {
                sendPreDefinedIP(recv_header, forIPv6, forwardArgs, true, sockfd);
            } else {
                forwardArgs->taskCount = totalTaskCount;
                forwardDNSquery(forward, forwardArgs);
                ++totalTaskCount;
            }
        }
        println("Received {} bytes from {}:{}", len,
               inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));
    }

    cleanFilter(&filter);
    close(sockfd);
    for (int i : vw::iota(0, n_threads))
        pthread_cancel(threads[i]);
    free(threads);
    pthread_mutex_destroy(&queueMutex);
    for (int i : vw::iota(0, n_threads))
        close(upstream_socks[i]);
    return 0;
}