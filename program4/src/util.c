#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "util.h"


/* program name */
extern char *progname;


/* print formatted error message to stderr */
int errprintf(char const *msg, ...) {
    va_list va;

    va_start(va, msg);

    char *buf;
    if (vasprintf(&buf, msg, va) == -1) {
        va_end(va);
        return -1;
    }

    fprintf(stderr, "%s: error: %s\n", progname, buf);
    free(buf);

    va_end(va);
    return 0;
}


/* strtol wrapper with error handling */
long long strtoll_safe(char *str) {
    char *endptr;

    errno = 0;
    long long n = strtoll(str, &endptr, 10);

    if (errno != 0 || *endptr != '\0')
        return -1;

    return n;
}


/* bind socket */
int bind_socket(int port) {
    /* create socket */
    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        errprintf("failed to create socket");
        return -1;
    }

    /* bind socket */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    /* bind to localhost for now */
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(sock_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        errprintf("binding to socket failed");
        return -1;
    }

    return sock_fd;
}
