/* start helper.h */
#ifndef _HELPER_
#define _HELPER_

#include <stdio.h>
#include <sys/socket.h>  /* getnameinfo */
#include <netdb.h>  /* getnameinfo */
#include <errno.h>  /* error */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLEN 8192

/* begin error handle function */
void unix_error(char *message);
void gai_error(int r, char *message);
/* end error handle function */

/* begin file descriptor */
int Close(int fd);
/* end file descriptor */

/* begin network programming */
int Setsockopt(int socket, int level, int optname, const void *optval, socklen_t optlen);
int Getaddrinfo(const char *host, const char *service, const struct addrinfo *hints, struct addrinfo **result);
int Getnameinfo(const struct sockaddr * sa, socklen_t salen, char *host, size_t hostlen, char *service, size_t servicelen, int flags);
int open_listenfd(char *port);
int Open_listenfd(char *port);
int Accept(int listenfd, struct sockaddr *addr, int *addrlen);
/* end network programming */
#endif 
/* end helper.h */