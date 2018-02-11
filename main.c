#include "helper.h"

int main(int argc, char **argv){
    int listenfd, connfd;
    char hostname[MAXLEN], port[MAXLEN];
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;
    /* Check command-line args */
    if (argc != 2){
        fprintf(stderr, "usage : %s <port>\n", argv[0]);
        exit(1);
    }
    /* Create listenfd */
    listenfd = Open_listenfd(argv[1]);
    while(1){
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        Getnameinfo((struct sockaddr *)&clientaddr, clientlen, hostname, MAXLEN, port, MAXLEN, 0);
        fprintf(stdout, "Accepted connection from (%s : %s)\n", hostname, port);
    }
}