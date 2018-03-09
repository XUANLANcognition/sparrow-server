#include "helper.h"

/* start error function */
void unix_error(char *message){
    fprintf(stderr, "%s : %s\n", message, strerror(errno));
    exit(0);
}

void gai_error(int r, char *message){
    fprintf(stderr, "%s : %s\n", message, gai_strerror(r));
    exit(0);
}
/* end error function */

/* start RIO */
int Open(const char *pathname, int flags, mode_t mode){
    int r;
    if((r = open(pathname, flags, mode)) < 0){
        unix_error("Open error");
    } 
    return r;  
}

int Close(int fd){
    int r;
    if((r = close(fd)) != 0){
        unix_error("Close fail");
    }
}

void *Mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset){

    void *r;

    if((r = mmap(addr, length, prot, flags, fd, offset)) == ((void *) - 1)){
        unix_error("Mmap error");
    }
    return r;
}

int Munmap(void *addr, size_t length){

    int r;

    if((r = munmap(addr, length)) < 0){
        unix_error("Munmap error");
    }
    return r;
}

void rio_readinitb(rio_t *rp, int fd){
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

void Rio_readinitb(rio_t *rp, int fd){
    rio_readinitb(rp, fd);
}

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n){
    int cnt;
    
    while(rp->rio_cnt <= 0){
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if(rp->rio_cnt < 0){
            if(errno != EINTR){
                return -1;
            }
        }
        else if(rp->rio_cnt == 0){
            return 0;
        }
        else{
            rp->rio_bufptr = rp->rio_buf;
        }
    }
    cnt = n;
    if(rp->rio_cnt < n){
        cnt = rp->rio_cnt;
    }
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr = rp->rio_bufptr + cnt;
    rp->rio_cnt = rp->rio_cnt - cnt;
    return cnt;
}

/* begin rio_writen */
ssize_t rio_writen(int fd, void *usrbuf, size_t n){
    char *pbuf = usrbuf;
    ssize_t nwritten;
    size_t nleft = n;

    while(nleft > 0){
        if((nwritten = write(fd, pbuf, nleft)) < 0){
            if(errno == EINTR){
                nwritten = 0;
            }
            else{
                return -1;
            }
        }
        nleft = nleft - nwritten;
        pbuf = pbuf + nwritten;
    }

    return n;
}
/* end rio_writen*/

/* begin Rio_writen */
void Rio_writen(int fd, void *usrbuf, size_t n){
    if(rio_writen(fd, usrbuf, n) != n){
        unix_error("Rio_writen");
    }
}
/* end Rio_writen*/

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen){
    int n, rc;
    char c, *pbuf = usrbuf;
    for(n = 1; n < maxlen; n++){
        if((rc = rio_read(rp, &c, 1)) == 1){
            *pbuf++ = c;
            if(c == '\n'){
                n++;
                break;
            }
        }
        else if(rc == 0){
            if(n == 1){
                return 0;
            }
            else{
                break;
            }
        }
        else{
            return -1;
        }
    }
    *pbuf = '\0';
    return n-1;
}

ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen){
    ssize_t r;
    if((r = rio_readlineb(rp, usrbuf, maxlen)) < 0){
        unix_error("Rio_readlineb");
    }
    return r;
}
/* end RIO */

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
    if((r = open_listenfd(port)) < 0){
        unix_error("Open_listenfd fail");
    }
    return r;
}

int Accept(int listenfd, struct sockaddr *addr, int *addrlen){
    int r;
    if((r = accept(listenfd, addr, addrlen)) < 0){
        unix_error("Accept fail");
    }
    return r;
}
/* end network programming */
