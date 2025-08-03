#ifndef HANDLE_FILTERS_H_p2B9agE1
#define HANDLE_FILTERS_H_p2B9agE1

#include "configreader.h"

bool inNotFind(const char * domain, NotFind * notFind);
bool inRefuse(const char * domain, Refuse * refuse);
bool inPreDefinedIPv4(const char * domain, ArrayPreDefinedIPv4 * arrayPreDefinedIPv4, char * ipv4);
bool inPreDefinedIPv6(const char * domain, ArrayPreDefinedIPv6 * arrayPreDefinedIPv6, char * ipv6);
void loadFiltersAndParams(const char * configFileName, Filter * filter, Server * server, UpStream * upStream);
void cleanFilter(Filter * filter);
void printFilters(Filter * filter);

#endif
