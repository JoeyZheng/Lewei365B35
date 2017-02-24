#ifndef __LEWEICLIENT_H__
#define __LEWEICLIENT_H__

#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */

typedef struct LeweiClient_s {
    char *head;
    char *user_data;
    int user_str_length;
    char begin;
} LeweiClient_t;

int LWCsetup(LeweiClient_t *lwc, const char * user_key, const char *gateway);
int LWCappend(LeweiClient_t *lwc, const char * name, int value, char vnum);
int LWCsend(LeweiClient_t *lwc);

#endif /* __LEWEICLIENT_H__ */
