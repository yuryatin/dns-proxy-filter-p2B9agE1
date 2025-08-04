module handlefilters;

import <cstdio>;
import <cstdlib>;
import <cstring>;
import <print>;
import <ranges>;

import configreader;
import utils;

using std::print;
using std::println;
namespace vw = std::views;

bool inNotFind(const char * domain, NotFind * notFind) {
    for (int i : vw::iota(0, notFind->n))
        if (strcasecmp(domain, notFind->domains[i].domain) == 0)
            return true;
    return false;
}

bool inRefuse(const char * domain, Refuse * refuse) {
    for (int i : vw::iota(0, refuse->n))
        if (strcasecmp(domain, refuse->domains[i].domain) == 0) 
            return true;
    return false;
}

bool inPreDefinedIPv4(const char * domain, ArrayPreDefinedIPv4 * arrayPreDefinedIPv4, char * ipv4) {
    for (int i : vw::iota(0, arrayPreDefinedIPv4->n))
        if (strcasecmp(domain, arrayPreDefinedIPv4->records[i].domain) == 0) {
            strcpy(ipv4, arrayPreDefinedIPv4->records[i].ip);
            return true;
        }
    return false;
}

bool inPreDefinedIPv6(const char * domain, ArrayPreDefinedIPv6 * arrayPreDefinedIPv6, char * ipv6) {
    for (int i : vw::iota(0, arrayPreDefinedIPv6->n))
        if (strcasecmp(domain, arrayPreDefinedIPv6->records[i].domain) == 0) {
            strcpy(ipv6, arrayPreDefinedIPv6->records[i].ip);
            return true;
        }
    return false;
}

void loadFiltersAndParams(const char * configFileName, Filter * filter, Server * server, UpStream * upStream) {
    FILE * f = fopen(configFileName, "r");
    if (f == nullptr) {
        perror("The DNS proxy filter cannot access the configuration file you may have provided when launching the proxy. This could be due to one of the following reasons:\n\t(1) DNS proxy filter is not authorized to read the file\n\t(2) the file path was not specified as the first parameter\n\t(3) the provided file path is incorrect\n\nDNS proxy filter will proceed with default settings. No domains will be filtered.");
        return;
    }
    scanConfigLines(f, false, filter, server, upStream);
    rewind(f);
    scanConfigLines(f, true, filter, server, upStream);
    println("\nThe parameters are loaded:\n\tDNS proxy filter should\n\t\tlisten at \n\t\t\tIP \t{}\n\t\t\tport \t{}\n\t\tforward DNS queries to:\n\t\t\t{}\n\t\t\t{}\n\t\t\t{}\n\n\tIt will\n\t\tnot find \t\t{} domains\n\t\trefuse to serve \t{} domains\n\n\tIt has\n\t\tpreconfigured IPv4 for \t{} domains\n\t\tpreconfigured IPv6 for \t{} domains\n", server->ip, server->port, upStream->dns[0], upStream->dns[1], upStream->dns[2], filter->notFind.n, filter->refuse.n, filter->preDefinedIPv4.n, filter->preDefinedIPv6.n);
}

void cleanFilter(Filter * filter) {
    free(filter->notFind.domains);
    filter->notFind.domains = nullptr;
    free(filter->refuse.domains);
    filter->refuse.domains = nullptr;
    free(filter->preDefinedIPv4.records);
    filter->preDefinedIPv4.records = nullptr;
    free(filter->preDefinedIPv6.records);
    filter->preDefinedIPv6.records = nullptr;
}

void printFilters(Filter * filter) {
    println("\tThe filters\nDomains not to find:");
    for (int i : vw::iota(0, filter->notFind.n)) print("{}\t", filter->notFind.domains[i].domain);
    println("\n\nDomains to refuse:");
    for (int i : vw::iota(0, filter->refuse.n)) print("{}\t", filter->refuse.domains[i].domain);
    println("\n\nDomains with pre-defined IPv4:");
    for (int i : vw::iota(0, filter->preDefinedIPv4.n)) println("{:>32}\t{}", filter->preDefinedIPv4.records[i].domain, filter->preDefinedIPv4.records[i].ip);
    println("\nDomains with pre-defined IPv6:");
    for (int i : vw::iota(0, filter->preDefinedIPv6.n)) println("{:>32}\t{}", filter->preDefinedIPv6.records[i].domain, filter->preDefinedIPv6.records[i].ip);
    println("");
}