#include "ipsender.h"
#include "configreader.h"

extern int sockfd;

void sendPreDefinedIP(struct DnsHeader * recv_header, char * forIP, ForwardArgs * senderArgs, bool isIPv6) {
    const char * endOfQuestions = findEndOfQuestions(senderArgs->buffer + DNS_HEADER_SIZE, ntohs(recv_header->qdcount));
    char response[BUFFER_SIZE];
    memset(response, 0, sizeof(response));
    memcpy(response, senderArgs->buffer, senderArgs->len);

    // I clean a space of 16 bytes between end of questions and additional section of the query
    memmove(response + (endOfQuestions - senderArgs->buffer) + 16, response + (endOfQuestions - senderArgs->buffer), endOfQuestions - senderArgs->buffer - DNS_HEADER_SIZE);

    struct DnsHeader * resp_header = (struct DnsHeader *) response;
    resp_header->id = recv_header->id;

    // Setting the flags: QR=1 (response), RCODE=0 (NO ERROR), AA=0
    // I decided to reveal the response is non-authoritative
    resp_header->flags = htons(0x8000); // 1000 0000 0000 0000
    resp_header->qdcount = recv_header->qdcount;
    resp_header->ancount = htons(1); // one query response
    resp_header->nscount = recv_header->nscount;
    resp_header->arcount = 0;

    char * p = response + (endOfQuestions - senderArgs->buffer);

    // I append the answer section
    // I am using pointer to domain name in the query at offset 12 (0xC00C)
    *p++ = 0xC0;  // pointer to domain name
    *p++ = 0x0C;  // offset 12
    // TYPE = 1 (A) or TYPE = 28 (AAAA)
    *p++ = 0x00; *p++ = isIPv6 ? 0x1C : 0x01;
    // CLASS = 1 (IN)
    *p++ = 0x00; *p++ = 0x01;
    // TTL = 60 seconds
    *p++ = 0x00; *p++ = 0x00; *p++ = 0x00; *p++ = 0x3C;
    // RDLENGTH = 4 (IPv4) or RDLENGTH = 16 (IPv6)
    *p++ = 0x00;
    *p++ = isIPv6 ? 0x10 : 0x04;
    size_t responseSize;
    if (isIPv6) {   // RDATA = IPv6 address in 16 bytes
        struct in6_addr ip6_addr;
        inet_pton(AF_INET6, forIP, &ip6_addr);
        memcpy(p, &ip6_addr, 16); //p += 16;
        responseSize = 28;
    } else {    // RDATA = IPv4 address in 4 bytes
        struct in_addr ip_addr;
        inet_pton(AF_INET, forIP, &ip_addr);
        memcpy(p, &ip_addr, 4); //p += 4;
        responseSize = 16;
    }
/*  puts("\n\tIncoming package\n");
    hexPrint(senderArgs->buffer, senderArgs->len);
    puts("\tOutcoming package\n");
    hexPrint(response, endOfQuestions - senderArgs->buffer + responseSize); */
    sendto(sockfd, response, endOfQuestions - senderArgs->buffer + responseSize, 0, (struct sockaddr *) &senderArgs->sender, senderArgs->sender_len);
}