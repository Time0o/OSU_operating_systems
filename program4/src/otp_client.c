#if !defined ENC && !defined DEC
    #error "either ENC or DEC must be defined"
#endif

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"


/* program name */
char *progname;


int main(int argc, char **argv) {
    /* store program name */
    progname = basename(argv[0]);

    /* parse command line arguments */
    if (argc != 4) {
#if defined ENC
        char *arg_fmt = "PLAINTEXT KEY PORT";
#elif defined DEC
        char *arg_fmt = "CIPHERTEXT KEY PORT";
#endif
        fprintf(stderr, "Usage: %s %s\n", progname, arg_fmt);
        exit(EXIT_FAILURE);
    }

    char *text_file = argv[1];
    char *key_file = argv[2];
    char *port = argv[2];

    FILE *text_fp = fopen(text_file, "r");
    if (!text_fp) {
        errprintf("failed to open '%s'\n", text_file);
        exit(EXIT_FAILURE);
    }

    FILE *key_fp = fopen(key_file, "r");
    if (!key_fp) {
        errprintf("failed to open '%s'\n", key_file);
        exit(EXIT_FAILURE);
    }

    return 0;
}
