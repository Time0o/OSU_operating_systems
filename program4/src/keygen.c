#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


/* random state array size in bytes */
enum { RANDOM_STATE_SIZE = 256 };

/* program name */
char *progname;


/* reuseable usage error handler */
static void usage_error() {
    fprintf(stderr, "Usage: %s KEY_LENGTH\n", progname);
    exit(EXIT_FAILURE);
}


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
    if (argc != 2)
        usage_error();

    char *endptr;

    errno = 0;
    long long length = strtoll(argv[1], &endptr, 10);

    if (errno == ERANGE) {
        if (length == LLONG_MIN)
            fprintf(stderr, "key length underflow\n");
        else if (length == LLONG_MAX)
            fprintf(stderr, "key length overflow\n");

        exit(EXIT_FAILURE);

    } else if (errno == EINVAL || *endptr != '\0') {
        usage_error();
    }

    /* allocate heap memory for key
       (not really necessary but makes this more reuseable) */
    char *key = malloc(length);

    if (!key) {
        fprintf(stderr, "failed to allocate memory for key array\n");
        exit(EXIT_FAILURE);
    }

    /* securely seed random number generator */
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "failed to open /dev/urandom\n");
        free(key);
        exit(EXIT_FAILURE);
    }

    unsigned seed;
    if (read(fd, &seed, sizeof(seed)) < 0) {
        fprintf(stderr, "failed to read from /dev/urandom\n");
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
