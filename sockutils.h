// socket_utils.c
// November 9, 2000
// Lisa Frye
// from Stevens, UNIX Network Programming Vol 1 book, page 78-79
// readn, writen, and readline functions to use with sockets

#include <unistd.h>

ssize_t readn(int fd, void *vptr, size_t n);
ssize_t writen(int fd, void *vptr, size_t n);
ssize_t readline(int fd, void *vptr, size_t maxlen);
