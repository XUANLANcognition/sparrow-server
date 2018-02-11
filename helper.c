#include "helper.h"

/* start error function */
void unix_error(char *message){
    fprintf(stderr, "%s: %s\n", message, strerror(errno));
    exit(0);
}

void gai_error(int r, char *message){
    fprintf(stderr, "%s: %s\n", message, gai_strerror(r));
    exit(0);
}
/* end error function */

/* start file descriptor */
int Close(int fd){
    int r;
    if((r = close(fd)) != 0){
        unix_error("Close fail");
    }
}
/* end file descriptor */

/* start network programming */
int Setsockopt(int socket, int level, int optname, const void *optval, socklen_t optlen){
    int r;
    if((r = setsockopt(socket, level, optname, optval, optlen)) < 0){
        unix_error("Setsockopt fail");
    }
}

int Getaddrinfo(const char *host, const char *service, const struct addrinfo *hints, struct addrinfo **result){
    int r;

    if((r = getaddrinfo(host, service, hints, result)) != 0){
        gai_error(r, "Getaddrinfo fail");
    }
}

int Getnameinfo(const struct sockaddr * sa, socklen_t salen, char *host, size_t hostlen, char *service, size_t servicelen, int flags){
    int r;
    if((r = getnameinfo(sa, salen, host, hostlen, service, servicelen, flags)) != 0){
        unix_error("Getanmeinfo fail");
    }
    return r;
}

int open_listenfd(char *port){

    struct addrinfo hints, *plist, *ptem;
    int listenfd, optval = 1;

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_socktype = SOCK_STREAM;  /* Accpet TCP connections */
    hints.ai_flags = AI_PASSIVE;  /* Accpet all IP */
    hints.ai_flags = hints.ai_flags | AI_NUMERICSERV;  /* service must be port  */
    hints.ai_flags |= AI_ADDRCONFIG;  /*  */

    Getaddrinfo(NULL, port, &hints, &plist);

    /* Walk the list for one that we can bind to*/
    for(ptem = plist; ptem; ptem = ptem->ai_next){
        /* socket */
        if((listenfd = socket(ptem->ai_family, ptem->ai_socktype, ptem->ai_protocol)) < 0){
            continue;
        }
        /* setsockopt */
        Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
        /* bind */
        if((bind(listenfd, ptem->ai_addr, ptem->ai_addrlen)) == 0){
            break;  /* successful */
        }
        /* try next */
        Close(listenfd);
    }

    if(!ptem){
        return -1;
    }

    if(listen(listenfd, 1024) < 0){
        return -1;
    }

    return listenfd;
}

int Open_listenfd(char *port){
    int r;
    if(r = open_listenfd(port) < 0){
        unix_error("Open_listenfd fail.");
        return r;
    }
}

int Accept(int listenfd, struct sockaddr *addr, int *addrlen){
    int r;
    if((r = accept(listenfd, addr, addrlen)) < 0){
        unix_error("Accept fail");
    }
    return r;
}
/* end network programming */