#include "handlefilters.h"
#include "upstreamreq.h"
#include "utils.h"
#include "configreader.h"
#include "ipsender.h"

const char * configFileName = NULL;

int taskCount = 0;
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
        .dns1 = "1.1.1.1",
        .dns2 = "8.8.8.8",
        .dns3 = "8.8.4.4",
        .dns1ipv6 = False,
        .dns2ipv6 = False,
        .dns3ipv6 = False
    };

    if (configFileName) loadFiltersAndParams(configFileName, &filter, &server, &upStream);
    printFilters(&filter);

    int sockfd;
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int upstream_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (upstream_sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in upstream_addr = {0};
    upstream_addr.sin_family = AF_INET;
    upstream_addr.sin_port = htons(53);
    if (inet_pton(AF_INET, upStream.dns1, &upstream_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(upstream_sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server.port);
    addr.sin_addr.s_addr = inet_addr(server.ip);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    pthread_t threads[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&threads[i], NULL, workerThread, NULL);
    }

    printf("Listening for UDP packets on %s:%d...\n", server.ip, server.port);
    
    char readDomain[DOMLENGTH];
    char forIPv4[IPv4LEN];
    char forIPv6[IPv6LEN];
    memset(readDomain, 0, sizeof(readDomain));
    memset(readDomain, 0, sizeof(forIPv4));
    memset(readDomain, 0, sizeof(forIPv6));
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
            unsigned char response[512];
            memset(response, 0, sizeof(response));
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
            unsigned char response[512];
            memset(response, 0, sizeof(response));
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
            forwardArgs->sockfd = sockfd;
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
                forwardArgs->upstream_sock = upstream_sock;
                memset(&forwardArgs->upstream_addr, 0, sizeof(forwardArgs->upstream_addr));
                forwardArgs->upstream_addr.sin_family = AF_INET;
                forwardArgs->upstream_addr.sin_port = htons(53);
                inet_pton(AF_INET, "8.8.8.8", &forwardArgs->upstream_addr.sin_addr);

                forwardDNSquery(forward, forwardArgs);
            }
        }
        printf("Received %zd bytes from %s:%d\n", len,
               inet_ntoa(sender.sin_addr), ntohs(sender.sin_port));
    }

    cleanFilter(&filter);
    close(sockfd);
    close(upstream_sock);
    return 0;
}