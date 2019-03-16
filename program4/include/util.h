#ifndef UTIL_H
#define UTIL_H

int errprintf(char const *msg, ...);

long long strtoll_safe(char *str);

int bind_socket(int port);

#endif /* UTIL_H */
