#ifndef UTILITIES_H_p2B9agE1
#define UTILITIES_H_p2B9agE1

#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>

#define DNS_HEADER_SIZE 12
#define DOMLENGTH 256
#define BUFFER_SIZE 4096

enum filterType { NFIND, RFSE, IP4C, IP6C };
enum boolean { False, True };

// DNS header of 12 bytes:
struct DnsHeader {
    uint16_t id;        // ID
    uint16_t flags;     // Flags
    uint16_t qdcount;   // QDCOUNT
    uint16_t ancount;   // ANCOUNT
    uint16_t nscount;   // NSCOUNT
    uint16_t arcount;   // ARCOUNT
} __attribute__((packed));

int isValidInteger(const char *str);
void hexPrint(const void * data, size_t len);
void parseDomainName(const char * buffer, char * output);
const char * findEndOfQuestions(const char * startOfQuestions, int nQuestions);

#endif