#include "handlefilters.h"
#include "upstreamreq.h"
#include "utils.h"
#include "configreader.h"
#include "ipsender.h"

const char * configFileName = NULL;
int n_threads = 0; //1

int sockfd;
struct sockaddr_in addr;
struct sockaddr_in upstream_addr[N_UPSTREAM_DNS];

int taskCount = 0;
int totalTaskCount = 0;
int upstream_socks[THREAD_POOL_SIZE];
task_t task_queue[MAX_TASKS];
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t taskAvailable = PTHREAD_COND_INITIALIZER;

int main(int argc, char *argv[]) {

    if (argc > 1) configFileName = argv[1];
    else puts("No path to a configuration file was provided. Proceeding with Default parameters.");

    Filter filter = {
        { .n=0, .domains=NULL },
        { .n=0, .domains=NULL },
        { .n=0, .records=NULL },
        { .n=0, .records=NULL }
    };
    Server server = {
        .ip = "127.0.0.1",
        .ipv6 = False,
        .port = 1053
    };
    UpStream upStream = {
        .dns = {"1.1.1.1", "8.8.8.8", "8.8.4.4"},
        .ipv6 = {False, False, False}
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

    for (int i = 0; i < N_UPSTREAM_DNS; i++) {
        upstream_addr[i].sin_family = AF_INET;
        upstream_addr[i].sin_port = htons(53);
        if (inet_pton(AF_INET, upStream.dns[i], &upstream_addr[i].sin_addr) <= 0) {
            fprintf(stderr, "inet_pton for upstream dns #%d", i+1);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }

    pthread_t threads[THREAD_POOL_SIZE];
    for (n_threads = 0; n_threads < THREAD_POOL_SIZE; n_threads++) {
        upstream_socks[n_threads] = socket(AF_INET, SOCK_DGRAM, 0);
        if (upstream_socks[n_threads] < 0) {
            if (!n_threads) {
                perror("socket");
                close(sockfd);
                exit(EXIT_FAILURE);
            } else break;
        }
        pthread_create(&threads[n_threads], NULL, workerThread, (void *) (intptr_t) upstream_socks[n_threads]);
    }

    printf("Listening for UDP packets on %s:%d...\n", server.ip, server.port);
    
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
        printf("Query ID: %u\n", ntohs(recv_header->id));

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
            ForwardArgs * forwardArgs = malloc(sizeof(ForwardArgs));
            if (!forwardArgs) {
                perror("malloc for forwardArgs failed");
                exit(EXIT_FAILURE);
            }
            forwardArgs->sender_len = sender_len;
            memcpy(&forwardArgs->sender, &sender, sizeof(struct sockaddr_in));
            forwardArgs->buffer = malloc(len);
            memcpy(forwardArgs->buffer, buffer, len);
            forwardArgs->len = len;
            
            uint8_t * p0 = (uint8_t *) buffer + DNS_HEADER_SIZE;
            while (*p0 != 0) p0 += (*p0) + 1; p0 += 1;
            uint16_t qtype = ntohs(*(uint16_t *)p0);
            if(qtype == QTYPE_A && inPreDefinedIPv4(readDomain, &(filter.preDefinedIPv4), forIPv4)) {
                sendPreDefinedIP(recv_header, forIPv4, forwardArgs, False);
            } else if(qtype == QTYPE_AAAA && inPreDefinedIPv6(readDomain, &(filter.preDefinedIPv6), forIPv6)) {
                sendPreDefinedIP(recv_header, forIPv6, forwardArgs, True);
            } else {
                forwardArgs->taskCount = totalTaskCount;
                forwardDNSquery(forward, forwardArgs);
                ++totalTaskCount;
            }
        }
        printf("Received %zd bytes from %s:%d\n", len,
               inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));
    }

    cleanFilter(&filter);
    close(sockfd);
    for (int i = 0; i < n_threads; i++)
        close(upstream_socks[i]);
    return 0;
}