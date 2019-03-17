#ifndef SOCKET_H
#define SOCKET_H

enum socket_mode { SOCKET_BIND, SOCKET_CONNECT };

enum { BUF_SIZE = 256, CHUNK_SIZE = 256 };

int create_socket(int port, enum socket_mode mode);
int send_block(int sock_fd, char *block, long long block_length);
int receive_block(int sock_fd, char **block, long long *block_length);

#endif /* SOCKET_H */
