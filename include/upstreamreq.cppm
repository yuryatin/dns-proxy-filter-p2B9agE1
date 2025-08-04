export module upstreamreq;

import <cstdio>;
import <cstdlib>;
import <cctype>;

#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>

import utils;

export void submitTask(void (* func)(void *), void * arg);
export void forward(void * arg);
export void forwardDNSquery(void (* func)(void *), void * arg);
