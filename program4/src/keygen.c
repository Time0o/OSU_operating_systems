#define _GNU_SOURCE

#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"


/* random state array size in bytes */
enum { RANDOM_STATE_SIZE = 256 };

/* program name */
char *progname;


/* securely (I hope) generate a random lowercase letter or space
   (see OpenBSDs rc4random_uniform for details) */
static char random_character() {
    static unsigned long thresh = -27 % 27;

    unsigned long r;
    for (;;) {
        r = random();
        if (r >= thresh) {
            r %= 27;
            break;
        }
    }

    if (r == 26)
        return ' ';
    else
        return (char) r + 'a';
}


int main(int argc, char **argv) {
    /* store program name */
    progname = basename(argv[0]);

    /* parse key length argument */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s KEY_LENGTH\n", progname);
        exit(EXIT_FAILURE);
    }

    long long length = strtoll_safe(argv[1]);
    if (length == -1) {
        errprintf("failed to parse key length argument");
        exit(EXIT_FAILURE);
    }

    /* allocate heap memory for key
       (not really necessary but makes this more reuseable) */
    char *key = malloc(length);

    if (!key) {
        errprintf("failed to allocate memory for key array");
        exit(EXIT_FAILURE);
    }

    /* securely seed random number generator */
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        errprintf("failed to open /dev/urandom");
        free(key);
        exit(EXIT_FAILURE);
    }

    unsigned seed;
    if (read(fd, &seed, sizeof(seed)) < 0) {
        errprintf("failed to read from /dev/urandom");
        free(key);
        exit(EXIT_FAILURE);
    }

    char state[RANDOM_STATE_SIZE];
    initstate(seed, state, sizeof(state));
    setstate(state);

    /* generate key */
    int i;
    for (i = 0; i < length; ++i)
        key[i] = random_character();

    /* dump key */
    long long int j;
    for (j = 0; j < length; j += sizeof(int))
        printf("%.*s", (int) sizeof(int), key + j);

    putchar('\n');

    free(key);

    return 0;
}
