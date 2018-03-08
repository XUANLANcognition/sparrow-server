#include "helper.h"

void doit(int fd);
void clienterror(int fd, char *errnum, char *shortmes, char *longmes, char *cause);
void read_request(rio_t *rp);


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
        doit(connfd);
        Close(connfd);
    }
}

void doit(int fd){

    int k;
    int is_static;
    rio_t rio;
    char buf[MAXLEN], method[MAXLEN], uri[MAXLEN], version[MAXLEN];
    struct stat sbuf;

    /* Read request line and header */
    Rio_readinitb(&rio, fd);
    if(!Rio_readlineb(&rio, buf, MAXLEN)){
        return;
    }
    fprintf(stdout, "Request header is : \n");
    fprintf(stdout, "%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    /* request method */
    if(strcasecmp(method, "GET")){
        clienterror(fd, "501", "Not Implement", "Not Implement.", method);
        return;
    }
    read_request(&rio);
    /* request uri */
    is_static = parse_uri(uri, filename, cgiargs);
    
    
}

/* begin parse_uri */
int parse_uri(char *uri, char filename, char *cgiargs){

    char *ptr;

    /* Static content */
    if(!strstr(uri, "cgi-bin")){
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if(uri[strlen(uri) - 1] == "/"){
            strcat(filename, "home.html");
        }
        return 1;
    }
    /* Dynamic content */
    else{
        ptr = strchr(uri, '?');
        if(ptr){
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }        
        else{
            strcpy(cgiargs, "");
        }
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}
/* end parse_uri */

/* begin read_request */
void read_request(rio_t *rp){
    
    char buf[MAXLEN];
    
    Rio_readlineb(rp, buf, MAXLEN);
    while(strcmp(buf, "\r\n")){
        fprintf(stdout, "%s", buf);
        Rio_readlineb(rp, buf, MAXLEN);
    }
    return;
}
/* end read_request */

/* begin clienterror */
void clienterror(int fd, char *errnum, char *shortmes, char *longmes, char *cause){
    char body[MAXLEN];
    
    /* bulid response body */
    sprintf(body, "<html><title>Server Error</title>");
    sprintf(body, "%s<body bgcolor=ffffff>\r\n", body);
    sprintf(body, "%s%s : %s\r\n", body, errnum, shortmes);
    sprintf(body, "%s%s : %s\r\n", body, longmes, cause);   

    Rio_writen(fd, body, strlen(body));
}
/* end clienterror */
