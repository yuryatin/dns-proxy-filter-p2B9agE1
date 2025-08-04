export module configreader;

import <cstdio>;

import utils;

export inline constexpr std::size_t IPv4LEN = 16uz;
export inline constexpr std::size_t IPv6LEN = 40uz;
export inline constexpr int QTYPE_A = 1;
export inline constexpr int QTYPE_AAAA = 28;

export typedef struct {
    char domain[DOMLENGTH];
} Domain;

export typedef struct {
    char domain[DOMLENGTH];
    char ip[IPv4LEN];
} PreDefinedIPv4;

export typedef struct {
    char domain[DOMLENGTH];
    char ip[IPv6LEN];
} PreDefinedIPv6;

export typedef struct {
    char ip[IPv6LEN];
    bool ipv6;
    int port;
} Server;

export typedef struct {
    char dns[N_UPSTREAM_DNS][IPv6LEN];
    bool ipv6[N_UPSTREAM_DNS];
} UpStream;

export typedef struct {
    int n;
    Domain * domains;
} NotFind;

export typedef struct {
    int n;
    Domain * domains;
} Refuse;

export typedef struct {
    int n;
    PreDefinedIPv4 * records;
} ArrayPreDefinedIPv4;

export typedef struct {
    int n;
    PreDefinedIPv6 * records;
} ArrayPreDefinedIPv6;

export typedef struct {
    NotFind notFind;
    Refuse refuse;
    ArrayPreDefinedIPv4 preDefinedIPv4;
    ArrayPreDefinedIPv6 preDefinedIPv6;
} Κάθαρσις;

export [[nodiscard]] bool keyIsListenAddress(const char * key);
export [[nodiscard]] bool keyIsListenPort(const char * key);
export [[nodiscard]] bool keyIsDNS1(const char * key);
export [[nodiscard]] bool keyIsDNS2(const char * key);
export [[nodiscard]] bool keyIsDNS3(const char * key);
export [[nodiscard]] bool valueIsNotFind(const char * value);
export [[nodiscard]] bool valueIsRefuse(const char * value);
export [[nodiscard]] bool isValidPort(const char * port);
export [[nodiscard]] bool isValidIPv4(const char *ip);
export [[nodiscard]] bool isValidIPv6(const char *ip);
export [[nodiscard]] bool isValidDomain(char * domainStart);
export void scanConfigLines(FILE * f, bool reading, Κάθαρσις * filter, Server * server, UpStream * upStream);
