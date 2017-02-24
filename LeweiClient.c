#include "LeweiClient.h"
#include "dbg.h"

#define error(msg) { \
        perror(msg); \
        close(sockfd); \
        return -1; \
}


int LWCsetup(LeweiClient_t *lwc, const char *user_key, const char *gateway) {
    char *ptr;
    int head_length = 0;
    int tmp;

    DBGPRINT("--- enter LWCsetup ---\n");

    lwc->head = (char *)malloc(160);
    if (lwc->head == NULL)
        return -1;
    ptr = lwc->head;
    memset(lwc->head, 0, sizeof(lwc->head));

    lwc->user_str_length = 0;

//    lwc->user_data = NULL;
    lwc->user_data = (char *)malloc(300);
    if (lwc->user_data == NULL)
        return -1;

    // build head.
    tmp = sprintf(ptr, "POST /api/V1/Gateway/UpdateSensors/%s HTTP/1.1\r\n", gateway);
    head_length += tmp;
    ptr += tmp;

    // build userkey.
    tmp = sprintf(ptr, "userkey: %s", user_key);
    head_length += tmp;
    ptr += tmp;

    // build Host.
    tmp = sprintf(ptr, "Host: www.lewei50.com\r\n");
    head_length += tmp;
    ptr += tmp;

    // build User-Agent.
/*    tmp = sprintf(ptr, "User-Agent: BeagleBone Black\r\n");
    head_length += tmp;
    ptr += tmp;
*/

//    DBGPRINT("setup:%s\r\n", lwc->head);
    return 0;
}

int LWCappend(LeweiClient_t *lwc, const char *name, int value, char vnum) {
    int length;
    int tmp;
    char *ptr;

    DBGPRINT("---enter LWCappend---\n");
    if(vnum == 0) {
        lwc->user_data[0] = '[';
        lwc->user_data[1] = 0;
        lwc->user_str_length = 1;
    } else {
        if(lwc->user_data == NULL) return -1;
        lwc->user_data[lwc->user_str_length] = ',';
        lwc->user_str_length += 1;
        lwc->user_data[lwc->user_str_length] = 0;
    }

    if(lwc->user_data == NULL) return -1;

    ptr = lwc->user_data + lwc->user_str_length;

    length = sprintf(ptr, "{\"Name\":\"%s\",\"Value\":\"%d\"}", name, value);

    lwc->user_str_length += length;

//    DBGPRINT("append:%s\r\n", ptr);
//
    return length;
}


int LWCsend(LeweiClient_t *lwc) {
    int portno = 80;
    char *host = "open.lewei50.com";
    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;
    char message[1024], response[4096];
    char *message_fmt = "%sContent-Length: %d\r\nConnection: Close\r\n\r\n%s]\r\n";

    DBGPRINT("---enter LWCsend---\n");

    /* fill in the parameters */
    sprintf(message, message_fmt, lwc->head, strlen(lwc->user_data) + 1, lwc->user_data);
    DBGPRINT("message:\n---\n%s\n---\n", message);

    /* create the socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0 ) {
        perror("Error creating socket");
        return -1; 
    }

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) 
        error("ERROR finding host");

    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting host");

    /* send the request */
    total = strlen(message);
    sent = 0;
    do {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0)
            error("ERROR writing message to socket");
        if (bytes == 0)
            break;
        sent += bytes;
    } while (sent < total);

    /* receive the response */
    memset(response, 0, sizeof(response));
    total = sizeof(response) - 1;
    received = 0;
    DBGPRINT("total = %d\n", total);
    do {
        bytes = read(sockfd, response + received, total - received);
        if (bytes < 0)
            error("ERROR reading response from socket");
        if (bytes == 0)
            break;
        DBGPRINT("bytes = %d\n", bytes);
        received += bytes;
    } while (received < total);

    if (received == total)
        error("ERROR storing complete response from socket");

    /* close the socket */
    close(sockfd);

    /* process response */
    if (received != 302) { // Successful msg is 302 bytes
        printf("--- --- --- ---\n");
        printf("received = %d\n", received);
        printf("+++ +++ +++ +++\n");
        printf("Response:\n%s\n", response);
        printf("--- --- --- ---\n");
        printf("--->");
        return -2;
    } else {
        DBGPRINT("--- --- --- ---\n");
        DBGPRINT("Response:\n%s\n",response);
        DBGPRINT("--- --- --- ---\n");
        return 0;
    }
}

