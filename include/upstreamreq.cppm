export module upstreamreq;

#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cctype>

import utils;

export void submitTask(void (* func)(void *), void * arg);
export void forward(void * arg);
export void forwardDNSquery(void (* func)(void *), void * arg);
