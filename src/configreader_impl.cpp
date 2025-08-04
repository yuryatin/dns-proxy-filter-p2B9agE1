module configreader;

#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

import utils;

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

bool keyIsListenAddress(const char * key) {
    if (strncmp(key, "listen_address", 16) == 0) return true;
    else return false;
}

bool keyIsListenPort(const char * key) {
    if (strncmp(key, "listen_port", 13) == 0) return true;
    else return false;
}

bool keyIsDNS1(const char * key) {
    if (strncmp(key, "dns1", 6) == 0) return true;
    else return false;
}

bool keyIsDNS2(const char * key) {
    if (strncmp(key, "dns2", 6) == 0) return true;
    else return false;
}

bool keyIsDNS3(const char * key) {
    if (strncmp(key, "dns3", 6) == 0) return true;
    else return false;
}

bool valueIsNotFind(const char * value) {
    if (strncmp(value, "notfind", 7) == 0) return true;
    else return false;
}

bool valueIsRefuse(const char * value) {
    if (strncmp(value, "refuse", 6) == 0) return true;
    else return false;
}

bool isValidPort(const char * port) {
    for (int i = 0; port[i] != '\0'; i++) {
        if (!isdigit((unsigned char)port[i]))
            return false;
    }
    long intPort = strtol(port, NULL, 10);
    return intPort > 0 && intPort < 65536;
}

bool isValidIPv4(const char *ip) {
    struct in_addr addr;
    return inet_pton(AF_INET, ip, &addr) == true;
}

bool isValidIPv6(const char *ip) {
    struct in6_addr addr6;
    return inet_pton(AF_INET6, ip, &addr6) == true;
}

bool isValidDomainChar(unsigned char c, bool first, bool last) {
    if (first) return isalpha(c) || isdigit(c) || c == '_';
    else if (last) return isalpha(c) || isdigit(c);
    else return isalpha(c) || isdigit(c) || c == '-';
}

bool isValidDomain(char * domainStart) {
    char * domain = domainStart;
    bool first = true;
    bool last = false;
    while (*domain != '\0') {
        if (*domain == '.') {
            first = true;
            ++domain;
            continue;
        }
        if (*(domain+1) == '.') last = true;
        if (isValidDomainChar(*domain, first, last)) {
            first = last = false;
            ++domain;
            continue;
        } else return false;
    }
    return true;
}

void scanConfigLines(FILE * f, bool reading, Filter * filter, Server * server, UpStream * upStream) {
    char line[BUFFER_SIZE];
    char * p = NULL;
    bool inBlackList = false;
    bool inUpstream = false;
    bool inServer = false;
    //int lineCounter = 0;

    int notFindCounter = 0;
    int refuseCounter = 0;
    int BlackListIPv4Counter = 0;
    int BlackListIPv6Counter = 0;

    if (reading) {
        filter->notFind.domains = (Domain *) malloc(filter->notFind.n * sizeof(Domain));
        filter->refuse.domains = (Domain *) malloc(filter->refuse.n * sizeof(Domain));
        filter->preDefinedIPv4.records = (PreDefinedIPv4 *) malloc(filter->preDefinedIPv4.n * sizeof(PreDefinedIPv4));
        filter->preDefinedIPv6.records = (PreDefinedIPv6 *) malloc(filter->preDefinedIPv6.n * sizeof(PreDefinedIPv6));
    }

    while (fgets(line, sizeof(line), f)) {
        //lineCounter++;
        p = line;
        while (isspace(*p)) p++;
        if (*p == '[') {
            if (strncmp(p, "[blacklist]", 11) == 0) {
                inBlackList = true;
                inUpstream = inServer = false;
            } else if (strncmp(p, "[server]", 8) == 0) {
                inServer = true;
                inBlackList = inUpstream = false;
            } else if (strncmp(p, "[upstream]", 10) == 0) {
                inUpstream = true;
                inBlackList = inServer = false;
            } else inBlackList = inUpstream = inServer = false;
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
                    server->ipv6 = false;
                } else if (keyIsListenAddress(key) && isValidIPv6(value)) {
                    strncpy(server->ip, value, IPv6LEN);
                    server->ipv6 = true;
                } else if (keyIsListenPort(key) && isValidPort(value))
                    server->port = atoi(value);
            } else if (inUpstream) {
                if (keyIsDNS1(key) && isValidIPv4(value)) {
                    strncpy(upStream->dns[0], value, IPv4LEN);
                    upStream->ipv6[0] = false;
                } else if (keyIsDNS1(key) && isValidIPv6(value)) {
                    strncpy(upStream->dns[0], value, IPv6LEN);
                    upStream->ipv6[0] = true;
                } else if (keyIsDNS2(key) && isValidIPv4(value)) {
                    strncpy(upStream->dns[1], value, IPv4LEN);
                    upStream->ipv6[1] = false;
                } else if (keyIsDNS2(key) && isValidIPv6(value)) {
                    strncpy(upStream->dns[1], value, IPv6LEN);
                    upStream->ipv6[1] = true;
                }  else if (keyIsDNS3(key) && isValidIPv4(value)) {
                    strncpy(upStream->dns[2], value, IPv4LEN);
                    upStream->ipv6[2] = false;
                } else if (keyIsDNS3(key) && isValidIPv6(value)) {
                    strncpy(upStream->dns[2], value, IPv6LEN);
                    upStream->ipv6[2] = true;
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
