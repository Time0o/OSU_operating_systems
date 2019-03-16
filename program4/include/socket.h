#ifndef SOCKET_H
#define SOCKET_H

enum socket_mode { SOCKET_BIND, SOCKET_CONNECT };

int create_socket(int port, enum socket_mode mode);

#endif /* SOCKET_H */
