#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "socket.h"
#include "util.h"


int create_socket(int port, enum socket_mode mode) {
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

    /* use localhost for now */
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (mode == SOCKET_BIND) {
        if (bind(sock_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
            errprintf("binding to socket failed");
            return -1;
        }
    } else if (mode == SOCKET_CONNECT) {
        if (connect(sock_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
            errprintf("connecting to socket failed");
            return -1;
        }
    }

    return sock_fd;
}


int send_block(int sock_fd, char *block, long long block_length) {
    long long block_offs = 0, chunk_size;

    ssize_t write_size;
    while (block_offs < block_length) {
        chunk_size = CHUNK_SIZE;
        if (block_offs + chunk_size > block_length)
            chunk_size = block_length - block_offs;

        if ((write_size =
             write(sock_fd, block + block_offs, chunk_size)) == -1) {

            errprintf("failed to send data (%s)", strerror(errno));
            return -1;
        }

        block_offs += write_size;
    }

    return 0;
}


int receive_block(int sock_fd, char **block, long long *block_length) {
    char buf[BUF_SIZE];

    /* receive block size */
    if (read(sock_fd, buf, sizeof(*block_length)) == -1) {
        errprintf("failed to receive block size (%s)", strerror(errno));
        return -1;
    }

    memcpy(block_length, buf, sizeof(*block_length));

    /* allocate block */
    *block = malloc(*block_length);
    if (!*block) {
        errprintf("failed to allocate block");
        return -1;
    }

    /* receive block data */
    ssize_t read_size;

    long long block_offs = 0;

    while (block_offs < *block_length) {
        if ((read_size = read(sock_fd, buf, BUF_SIZE)) == -1) {
            errprintf("failed to receive data (%s)", strerror(errno));
            free(block);
            return -1;
        }

        if (block_offs + read_size > *block_length)
            read_size = *block_length - block_offs;

        memcpy(*block + block_offs, buf, read_size);

        block_offs += read_size;
    }

    return 0;
}
