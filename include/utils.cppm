export module utils;

import <cstring>;
import <cstdio>;
import <cctype>;
import <cstdlib>;
import <array>;

#include <unistd.h>
#include <arpa/inet.h>
#include <strings.h>

using std::array;

export inline constexpr std::size_t DNS_HEADER_SIZE = 12;
export inline constexpr std::size_t DOMLENGTH = 256;
export inline constexpr std::size_t BUFFER_SIZE = 4096;
export inline constexpr int N_UPSTREAM_DNS = 3;
export inline constexpr int MAX_TASKS = 100;

export enum filterType { NFIND, RFSE, IP4C, IP6C };

export struct task_t {
    void (* function)(void *);
    void * arg;
};

// DNS header of 12 bytes:
export struct DnsHeader {
    uint16_t id;        // ID
    uint16_t flags;     // Flags
    uint16_t qdcount;   // QDCOUNT
    uint16_t ancount;   // ANCOUNT
    uint16_t nscount;   // NSCOUNT
    uint16_t arcount;   // ARCOUNT
} __attribute__((packed));

export struct ForwardArgs {
    int taskCount;
    sockaddr_in sender;
    socklen_t sender_len;
    char * buffer;
    ssize_t len;
    int upstream_sock;
    const int sockfd;
    int * taskCountOverall;
    array<sockaddr_in, N_UPSTREAM_DNS> upstream_addr;
    task_t * task_queue;
    pthread_mutex_t * queueMutex;
    pthread_cond_t * taskAvailable;
    ForwardArgs(int &_sockfd, 
                int * _taskCountOverall,
                const array<sockaddr_in, N_UPSTREAM_DNS> &_upstream_addr,
                task_t * _task_queue,
                pthread_mutex_t * _queueMutex,
                pthread_cond_t * _taskAvailable) : sockfd(_sockfd),
                                                  taskCountOverall(_taskCountOverall), 
                                                  upstream_addr(_upstream_addr),
                                                  task_queue(_task_queue),
                                                  queueMutex(_queueMutex),
                                                  taskAvailable(_taskAvailable) {}
};

export int isValidInteger(const char *str);
export void hexPrint(const void * data, std::size_t len);
export void parseDomainName(const char * buffer, char * output);
export const char * findEndOfQuestions(const char * startOfQuestions, int nQuestions);
