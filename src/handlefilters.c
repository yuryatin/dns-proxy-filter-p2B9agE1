#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "configreader.h"
#include "utils.h"

enum boolean inNotFind(const char * domain, NotFind * notFind) {
    for (int i = 0; i < notFind->n; ++i)
        if (strcasecmp(domain, notFind->domains[i].domain) == 0)
            return True;
    return False;
}

enum boolean inRefuse(const char * domain, Refuse * refuse) {
    for (int i = 0; i < refuse->n; ++i)
        if (strcasecmp(domain, refuse->domains[i].domain) == 0) 
            return True;
    return False;
}

enum boolean inPreDefinedIPv4(const char * domain, ArrayPreDefinedIPv4 * arrayPreDefinedIPv4, char * ipv4) {
    for (int i = 0; i < arrayPreDefinedIPv4->n; ++i)
        if (strcasecmp(domain, arrayPreDefinedIPv4->records[i].domain) == 0) {
            strcpy(ipv4, arrayPreDefinedIPv4->records[i].ip);
            return True;
        }
    return False;
}

enum boolean inPreDefinedIPv6(const char * domain, ArrayPreDefinedIPv6 * arrayPreDefinedIPv6, char * ipv6) {
    for (int i = 0; i < arrayPreDefinedIPv6->n; ++i)
        if (strcasecmp(domain, arrayPreDefinedIPv6->records[i].domain) == 0) {
            strcpy(ipv6, arrayPreDefinedIPv6->records[i].ip);
            return True;
        }
    return False;
}

void loadFiltersAndParams(const char * configFileName, Filter * filter, Server * server, UpStream * upStream) {
    FILE * f = fopen(configFileName, "r");
    if (f == NULL) {
        perror("The DNS proxy filter cannot access the configuration file you may have provided when launching the proxy. This could be due to one of the following reasons:\n\t(1) DNS proxy filter is not authorized to read the file\n\t(2) the file path was not specified as the first parameter\n\t(3) the provided file path is incorrect\n\nDNS proxy filter will proceed with default settings. No domains will be filtered.");
        return;
    }
    scanConfigLines(f, False, filter, server, upStream);
    rewind(f);
    scanConfigLines(f, True, filter, server, upStream);
    printf("\nThe parameters are loaded:\n\tDNS proxy filter should\n\t\tlisten at \n\t\t\tIP \t%s\n\t\t\tport \t%d\n\t\tforward DNS queries to:\n\t\t\t%s\n\t\t\t%s\n\t\t\t%s\n\n\tIt will\n\t\tnot find \t\t%d domains\n\t\trefuse to serve \t%d domains\n\n\tIt has\n\t\tpreconfigured IPv4 for \t%d domains\n\t\tpreconfigured IPv6 for \t%d domains\n\n", server->ip, server->port, upStream->dns[0], upStream->dns[1], upStream->dns[2], filter->notFind.n, filter->refuse.n, filter->preDefinedIPv4.n, filter->preDefinedIPv6.n);
}

void cleanFilter(Filter * filter) {
    free(filter->notFind.domains);
    filter->notFind.domains = NULL;
    free(filter->refuse.domains);
    filter->refuse.domains = NULL;
    free(filter->preDefinedIPv4.records);
    filter->preDefinedIPv4.records = NULL;
    free(filter->preDefinedIPv6.records);
    filter->preDefinedIPv6.records = NULL;
}

void printFilters(Filter * filter) {
    puts("\tThe filters\nDomains not to find:");
    for (int i = 0; i < filter->notFind.n; ++i ) printf("%s\t", filter->notFind.domains[i].domain);
    puts("\n\nDomains to refuse:");
    for (int i = 0; i < filter->refuse.n; ++i ) printf("%s\t", filter->refuse.domains[i].domain);
    puts("\n\nDomains with pre-defined IPv4:");
    for (int i = 0; i < filter->preDefinedIPv4.n; ++i ) printf("%32s\t%s\n", filter->preDefinedIPv4.records[i].domain, filter->preDefinedIPv4.records[i].ip);
    puts("\nDomains with pre-defined IPv6:");
    for (int i = 0; i < filter->preDefinedIPv6.n; ++i ) printf("%32s\t%s\n", filter->preDefinedIPv6.records[i].domain, filter->preDefinedIPv6.records[i].ip);
    puts("");
}