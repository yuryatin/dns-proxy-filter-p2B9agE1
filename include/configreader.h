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
    enum boolean ipv6;
    int port;
} Server;

typedef struct {
    char dns[N_UPSTREAM_DNS][IPv6LEN];
    enum boolean ipv6[N_UPSTREAM_DNS];
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

enum boolean keyIsListenAddress(const char * key);
enum boolean keyIsListenPort(const char * key);
enum boolean keyIsDNS1(const char * key);
enum boolean keyIsDNS2(const char * key);
enum boolean keyIsDNS3(const char * key);
enum boolean valueIsNotFind(const char * value);
enum boolean valueIsRefuse(const char * value);
enum boolean isValidPort(const char * port);
enum boolean isValidIPv4(const char *ip);
enum boolean isValidIPv6(const char *ip);
enum boolean isValidDomain(char * domainStart);
void scanConfigLines(FILE * f, enum boolean reading, Filter * filter,   Server * server, UpStream * upStream);

#endif