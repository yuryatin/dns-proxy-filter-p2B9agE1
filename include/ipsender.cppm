export module ipsender;

#include <netinet/in.h>

import utils;
import upstreamreq;

export void sendPreDefinedIP(DnsHeader * recv_header, char * forIP, ForwardArgs * senderArgs, bool isIPv6, const int & sockfd);
