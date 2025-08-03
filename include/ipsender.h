#ifndef IP_SENDER_H_p2B9agE1
#define IP_SENDER_H_p2B9agE1

#include <netinet/in.h>
#include "utils.h"
#include "upstreamreq.h"

void sendPreDefinedIP(struct DnsHeader * recv_header, char * forIP, ForwardArgs * senderArgs, bool isIPv6);

#endif