#if !defined ENC && !defined DEC
    #error "either ENC or DEC must be defined"
#endif

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "util.h"


/* constants */
enum { CHUNK_SIZE = 256 };

/* program name */
char *progname;


static long long read_block(char *file, char **block) {
    FILE *fp = fopen(file, "r");
    if (!fp) {
        errprintf("failed to open '%s'\n", file);
        goto error;
    }

    /* determine size of block */
    long long block_length = 0;

    int c;
    for (;;) {
        c = fgetc(fp);

        if (c == EOF) {
            errprintf("failed to read '%s'\n", file);
            goto error;
        }

        if (c == '\n')
            break;

        ++block_length;
    }

    rewind(fp);

    /* allocate block */
    *block = malloc(block_length);
    if (!*block) {
        errprintf("failed to allocated block");
        goto error;
    }

    /* read block */
    long long block_offs = 0, chunk_size;
    while (block_offs < block_length) {
        chunk_size = CHUNK_SIZE;
        if (block_offs + chunk_size > block_length)
            chunk_size = block_length - block_offs;

        if (fread(*block + block_offs, 1, chunk_size, fp) != chunk_size) {
            errprintf("failed to read '%s'\n", file);
            free(block);
            goto error;
        }

        block_offs += chunk_size;
    }

    return block_length;

error:
    fclose(fp);
    return -1;
}


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

    int port = strtoll_safe(argv[3]);
    if (port == -1) {
        errprintf("failed to parse port parameter");
        exit(EXIT_FAILURE);
    }

    /* read text and key from file */
    char *text, *key;
    long long text_length, key_length;

    if ((text_length = read_block(argv[1], &text)) == -1)
        exit(EXIT_FAILURE);

    if ((key_length = read_block(argv[2], &key)) == -1) {
        free(text);
        exit(EXIT_FAILURE);
    }

    /* TODO */

    free(text);
    free(key);

    exit(EXIT_SUCCESS);
}
