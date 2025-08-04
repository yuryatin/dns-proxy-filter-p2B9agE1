export module handlefilters;

import <cstdio>;
import <cstdlib>;
import <cstring>;

import configreader;

export bool inNotFind(const char * domain, NotFind * notFind);
export bool inRefuse(const char * domain, Refuse * refuse);
export bool inPreDefinedIPv4(const char * domain, ArrayPreDefinedIPv4 * arrayPreDefinedIPv4, char * ipv4);
export bool inPreDefinedIPv6(const char * domain, ArrayPreDefinedIPv6 * arrayPreDefinedIPv6, char * ipv6);
export void loadFiltersAndParams(const char * configFileName, Filter * filter, Server * server, UpStream * upStream);
export void cleanFilter(Filter * filter);
export void printFilters(Filter * filter);
