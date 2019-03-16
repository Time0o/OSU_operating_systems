#define _GNU_SOURCE

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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
