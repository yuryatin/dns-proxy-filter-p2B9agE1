#ifndef CONFIG_READER_H_p2B9agE1
#define CONFIG_READER_H_p2B9agE1

#include <stdio.h>
#include "utils.h"

#define IPv4LEN 16
#define IPv6LEN 40
#define DOMLENGTH 256
#define DNS_HEADER_SIZE 12
#define QTYPE_A 1
#define QTYPE_AAAA 28
#define N_UPSTREAM_DNS 3

typedef struct {
    char domain[DOMLENGTH];
} Domain;

typedef struct {
    char domain[DOMLENGTH];
    char ip[IPv4LEN];
} PreDefinedIPv4;

typedef struct {
    char domain[DOMLENGTH];
    char ip[IPv6LEN];
} PreDefinedIPv6;

typedef struct {
    char ip[IPv6LEN];
    bool ipv6;
    int port;
} Server;

typedef struct {
    char dns[N_UPSTREAM_DNS][IPv6LEN];
    bool ipv6[N_UPSTREAM_DNS];
} UpStream;

typedef struct {
    int n;
    Domain * domains;
} NotFind;

typedef struct {
    int n;
    Domain * domains;
} Refuse;

typedef struct {
    int n;
    PreDefinedIPv4 * records;
} ArrayPreDefinedIPv4;

typedef struct {
    int n;
    PreDefinedIPv6 * records;
} ArrayPreDefinedIPv6;

typedef struct {
    NotFind notFind;
    Refuse refuse;
    ArrayPreDefinedIPv4 preDefinedIPv4;
    ArrayPreDefinedIPv6 preDefinedIPv6;
} Filter;

bool keyIsListenAddress(const char * key);
bool keyIsListenPort(const char * key);
bool keyIsDNS1(const char * key);
bool keyIsDNS2(const char * key);
bool keyIsDNS3(const char * key);
bool valueIsNotFind(const char * value);
bool valueIsRefuse(const char * value);
bool isValidPort(const char * port);
bool isValidIPv4(const char *ip);
bool isValidIPv6(const char *ip);
bool isValidDomain(char * domainStart);
void scanConfigLines(FILE * f, bool reading, Filter * filter,   Server * server, UpStream * upStream);

#endif