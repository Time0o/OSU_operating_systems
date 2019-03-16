#if !defined ENC && !defined DEC
    #error "either ENC or DEC must be defined"
#endif

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "proto.h"
#include "socket.h"
#include "util.h"


/* constants */
enum { BUF_SIZE = 256, CHUNK_SIZE = 256, LISTEN_BACKLOG = 128 };

/* program name */
char *progname;


static long long receive_block(
    int sock_fd, char *buf, size_t buf_size, char **block) {

    /* receive block size */
    long long block_length, block_offs = 0;
    if (read(sock_fd, buf, sizeof(block_length)) == -1) {
        errprintf("failed to receive block size");
        return -1;
    }

    memcpy(&block_length, buf, sizeof(block_length));

    /* allocate block */
    *block = malloc(block_length);
    if (!*block) {
        errprintf("failed to allocate block");
        return -1;
    }

    /* receive block data */
    ssize_t read_size;
    while (block_offs < block_length) {
        if ((read_size = read(sock_fd, buf, buf_size)) == -1) {
            errprintf("failed to receive data");
            free(block);
            return -1;
        }

        if (block_offs + read_size > block_length)
            read_size = block_length - block_offs;

        memcpy(*block + block_offs, buf, read_size);

        block_offs += read_size;
    }

    return block_length;
}


static int send_block(
    int sock_fd, char *block, long long block_length, long long chunk_size) {

    long long block_offs = 0;

    ssize_t write_size;
    while (block_offs < block_length) {
        if (block_offs + chunk_size > block_length)
            chunk_size = block_length - block_offs;

        if ((write_size =
             write(sock_fd, block + block_offs, chunk_size)) == -1) {

            errprintf("failed to send data");
            return -1;
        }

        block_offs += write_size;
    }

    return 0;
}


static void code(char *text, char *key, long long text_length) {
    char t, k;

    long long i;
    for (i = 0; i < text_length; ++i) {
        t = text[i] - 'a';
        k = key[i] - 'a';

#if defined ENC
        text[i] = (t + k) % 26 + 'a';
#elif defined DEC
        char tmp = t - k;
        if (tmp < 0)
            tmp += 26;

        text[i] = tmp + 'a';
#endif
    }
}


int main(int argc, char **argv) {
    /* store program name */
    progname = basename(argv[0]);

    /* parse command line arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s PORT\n", progname);
        exit(EXIT_FAILURE);
    }

    int port;
    if ((port = strtoll_safe(argv[1])) == -1) {
        errprintf("failed to parse port argument");
        exit(EXIT_FAILURE);
    }

    /* create socket */
    int sock_fd;
    if ((sock_fd = create_socket(port, SOCKET_BIND)) == -1) {
        errprintf("failed to create socket");
        exit(EXIT_FAILURE);
    }

    /* listen for clients */
    if (listen(sock_fd, LISTEN_BACKLOG) == -1) {
        errprintf("listen failed\n");
        exit(EXIT_FAILURE);
    }

    /* handle client requests */
    struct sockaddr_in client_addr;
    unsigned client_addr_size = sizeof(client_addr);

    for (;;) {
        if (accept(sock_fd,
                   (struct sockaddr *) &client_addr,
                   &client_addr_size) == -1) {

            errprintf("accepting client failed");

        } else {
            /* fork of child process */
            pid_t child;
            switch (child = fork()) {
            case -1:
                errprintf("fork failed");
                break;
            case 0:
                {
                char buf[BUF_SIZE];

                /* receive protocol opcode */
                if (read(sock_fd, buf, sizeof(enum proto)) == -1) {
                    errprintf("failed to read opcode");
                    _Exit(EXIT_FAILURE);
                }

                enum proto proto;
                memcpy(&proto, buf, sizeof(proto));

#if defined ENC
                if (proto != PROTO_ENC)
#elif defined DEC
                if (proto != PROTO_DEC)
#endif
                {
                    errprintf("invalid protocol");
                    _Exit(EXIT_FAILURE);
                }

                /* receive text */
                char *text;
                long long text_length;
                if ((text_length =
                     receive_block(sock_fd, buf, BUF_SIZE, &text)) == -1) {

                    _Exit(EXIT_FAILURE);
                }

                /* receive key */
                char *key;
                long long key_length;
                if ((key_length =
                     receive_block(sock_fd, buf, BUF_SIZE, &key)) == -1) {

                    free(text);
                    _Exit(EXIT_FAILURE);
                }

                if (key_length < text_length) {
                    errprintf("key too short (%lld/%lld)",
                              key_length,
                              text_length);

                    _Exit(EXIT_FAILURE);
                }

                /* (en/de)code text */
                code(text, key, text_length);

                /* send result */
                if (send_block(sock_fd, text, text_length, CHUNK_SIZE) == -1)
                    _Exit(EXIT_FAILURE);
                }
                break;
            default:
                break;
            }
        }
    }

    exit(EXIT_SUCCESS);
}
