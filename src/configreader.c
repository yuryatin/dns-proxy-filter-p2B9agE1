#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "configreader.h"

enum boolean keyIsListenAddress(const char * key) {
    if (strncmp(key, "listen_address", 16) == 0) return True;
    else return False;
}

enum boolean keyIsListenPort(const char * key) {
    if (strncmp(key, "listen_port", 13) == 0) return True;
    else return False;
}

enum boolean keyIsDNS1(const char * key) {
    if (strncmp(key, "dns1", 6) == 0) return True;
    else return False;
}

enum boolean keyIsDNS2(const char * key) {
    if (strncmp(key, "dns2", 6) == 0) return True;
    else return False;
}

enum boolean keyIsDNS3(const char * key) {
    if (strncmp(key, "dns3", 6) == 0) return True;
    else return False;
}

enum boolean valueIsNotFind(const char * value) {
    if (strncmp(value, "notfind", 7) == 0) return True;
    else return False;
}

enum boolean valueIsRefuse(const char * value) {
    if (strncmp(value, "refuse", 6) == 0) return True;
    else return False;
}

enum boolean isValidPort(const char * port) {
    for (int i = 0; port[i] != '\0'; i++) {
        if (!isdigit((unsigned char)port[i]))
            return 0;
    }
    long intPort = strtol(port, NULL, 10);
    return intPort > 0 && intPort < 65536;
}

enum boolean isValidIPv4(const char *ip) {
    struct in_addr addr;
    return inet_pton(AF_INET, ip, &addr) == True;
}

enum boolean isValidIPv6(const char *ip) {
    struct in6_addr addr6;
    return inet_pton(AF_INET6, ip, &addr6) == True;
}

enum boolean isValidDomainChar(unsigned char c, enum boolean first, enum boolean last) {
    if (first) return isalpha(c) || isdigit(c) || c == '_';
    else if (last) return isalpha(c) || isdigit(c);
    else return isalpha(c) || isdigit(c) || c == '-';
}

enum boolean isValidDomain(char * domainStart) {
    char * domain = domainStart;
    enum boolean first = True;
    enum boolean last = False;
    while (*domain != '\0') {
        if (*domain == '.') {
            first = True;
            ++domain;
            continue;
        }
        if (*(domain+1) == '.') last = True;
        if (isValidDomainChar(*domain, first, last)) {
            first = last = False;
            ++domain;
            continue;
        } else return False;
    }
    return True;
}

void scanConfigLines(FILE * f, enum boolean reading, Filter * filter, Server * server, UpStream * upStream) {
    char line[BUFFER_SIZE];
    char * p = NULL;
    enum boolean inBlackList = False;
    enum boolean inUpstream = False;
    enum boolean inServer = False;
    //int lineCounter = 0;

    int notFindCounter = 0;
    int refuseCounter = 0;
    int BlackListIPv4Counter = 0;
    int BlackListIPv6Counter = 0;

    if (reading) {
        filter->notFind.domains = malloc(filter->notFind.n * sizeof(Domain));
        filter->refuse.domains = malloc(filter->refuse.n * sizeof(Domain));
        filter->preDefinedIPv4.records = malloc(filter->preDefinedIPv4.n * sizeof(PreDefinedIPv4));
        filter->preDefinedIPv6.records = malloc(filter->preDefinedIPv6.n * sizeof(PreDefinedIPv6));
    }

    while (fgets(line, sizeof(line), f)) {
        //lineCounter++;
        p = line;
        while (isspace(*p)) p++;
        if (*p == '[') {
            if (strncmp(p, "[blacklist]", 11) == 0) {
                inBlackList = True;
                inUpstream = inServer = False;
            } else if (strncmp(p, "[server]", 8) == 0) {
                inServer = True;
                inBlackList = inUpstream = False;
            } else if (strncmp(p, "[upstream]", 10) == 0) {
                inUpstream = True;
                inBlackList = inServer = False;
            } else inBlackList = inUpstream = inServer = False;
            continue;
        }
        if ((!inBlackList && !inUpstream && !inServer)|| *p == '\0' || *p == '#')
            continue;

        char * eq = strchr(p, '=');
        if (!eq) continue;

        * eq = '\0';
        char * key = p;
        char * value = eq + 1;
        char * key_end = key + strlen(key) - 1;
        char * value_end = value + strlen(value) - 1;

        while (isspace(* key)) key++;
        while (isspace(* value)) value++;
        while (key_end > key && isspace(*key_end)) *key_end-- = '\0';
        while (value_end > value && isspace(*value_end)) *value_end-- = '\0';

        //printf("Key: %s, value: %s at line %d\n", key, value, lineCounter);

        if (reading) {
            if (inServer) {
                if (keyIsListenAddress(key) && isValidIPv4(value)) {
                    strncpy(server->ip, value, IPv4LEN);
                    server->ipv6 = False;
                } else if (keyIsListenAddress(key) && isValidIPv6(value)) {
                    strncpy(server->ip, value, IPv6LEN);
                    server->ipv6 = True;
                } else if (keyIsListenPort(key) && isValidPort(value))
                    server->port = atoi(value);
            } else if (inUpstream) {
                if (keyIsDNS1(key) && isValidIPv4(value)) {
                    strncpy(upStream->dns[0], value, IPv4LEN);
                    upStream->ipv6[0] = False;
                } else if (keyIsDNS1(key) && isValidIPv6(value)) {
                    strncpy(upStream->dns[0], value, IPv6LEN);
                    upStream->ipv6[0] = True;
                } else if (keyIsDNS2(key) && isValidIPv4(value)) {
                    strncpy(upStream->dns[1], value, IPv4LEN);
                    upStream->ipv6[1] = False;
                } else if (keyIsDNS2(key) && isValidIPv6(value)) {
                    strncpy(upStream->dns[1], value, IPv6LEN);
                    upStream->ipv6[1] = True;
                }  else if (keyIsDNS3(key) && isValidIPv4(value)) {
                    strncpy(upStream->dns[2], value, IPv4LEN);
                    upStream->ipv6[2] = False;
                } else if (keyIsDNS3(key) && isValidIPv6(value)) {
                    strncpy(upStream->dns[2], value, IPv6LEN);
                    upStream->ipv6[2] = True;
                } 
            } else if (inBlackList) {
                if (isValidDomain(key) && isValidIPv4(value)) {
                    strncpy(filter->preDefinedIPv4.records[BlackListIPv4Counter].domain, key, DOMLENGTH);
                    strncpy(filter->preDefinedIPv4.records[BlackListIPv4Counter++].ip, value, IPv4LEN);
                } else if (isValidDomain(key) && isValidIPv6(value)) {
                    strncpy(filter->preDefinedIPv6.records[BlackListIPv6Counter].domain, key, DOMLENGTH);
                    strncpy(filter->preDefinedIPv6.records[BlackListIPv6Counter++].ip, value, IPv6LEN);
                } else if (isValidDomain(key) && valueIsNotFind(value)) {
                    strncpy(filter->notFind.domains[notFindCounter++].domain, key, DOMLENGTH);
                } else if (isValidDomain(key) && valueIsRefuse(value)) {
                    strncpy(filter->refuse.domains[refuseCounter++].domain, key, DOMLENGTH);
                } 
            }
        } else if (inBlackList) {
            if (isValidDomain(key) && isValidIPv4(value)) filter->preDefinedIPv4.n++;
            else if (isValidDomain(key) && isValidIPv6(value)) filter->preDefinedIPv6.n++;
            else if (isValidDomain(key) && valueIsNotFind(value)) filter->notFind.n++;
            else if (isValidDomain(key) && valueIsRefuse(value)) filter->refuse.n++;
        }
    }
}
