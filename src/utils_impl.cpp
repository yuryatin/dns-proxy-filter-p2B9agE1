module utils;

#include <unistd.h>
#include <arpa/inet.h>
#include <strings.h>

#include <cstring>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <print>

using std::print;
using std::println;

int isValidInteger(const char * str) {
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (str[0] == '\0') return false;
    for (int i = 0; str[i]; i++)
        if (!isdigit((unsigned char)str[i])) return false;
    int val = atoi(str);
    if (val <= 1) return false;
    if ((long) val > (nprocs * 8)) return (int)(nprocs * 8);
    return val;
}

void hexPrint(const void * data, size_t len) {
    const unsigned char * byte = (const unsigned char *) data;
    for (size_t i = 0; i < len; i += 16) {
        print("{:04x}  ", i);
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < len) print("{:02x} ", byte[i + j]);
            else print("   ");
        }
        print("\t");
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < len) {
                unsigned char c = byte[i + j];
                print("{}", isprint(c) ? c : '.');
            }
        }
        println("");
    }
    println("\n");
}

void parseDomainName(const char * buffer, char * output) {
    size_t pos = DNS_HEADER_SIZE;
    size_t end = 0;
    while (buffer[pos] != 0) {
        int label_len = buffer[pos++];
        for (int i = 0; i < label_len; i++)
            output[end++] = buffer[pos++];
        output[end++] = '.';
        if (pos >= DOMLENGTH) break;
    }
    if (end > 0) output[end - 1] = '\0';
    else output[0] = '\0';
}

const char * findEndOfQuestions(const char * startOfQuestions, int nQuestions) {
    const char * p = startOfQuestions;
    for (int i = 0; i < nQuestions; ++i) {
        while (*p != 0) p += (*p) + 1;
        p += 5;
    }
    return p;
}
