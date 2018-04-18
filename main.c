#include "helper.h"

void doit(int fd);
void clienterror(int fd, char *errnum, char *shortmes, char *longmes, char *cause);
void read_request(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void static_server(int fd, char *filename, int filesize);
void dynamic_server(int fd, char *filename, char *cgiargs);
void get_filetype(char *filename, char *filetype);
void *thread(void *vargp);

int main(int argc, char **argv){
    int listenfd, connfd, *connfdp;
    char hostname[MAXLEN], port[MAXLEN];
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;
    pthread_t tid;
    /* Check command-line args */
    if (argc != 2){
        fprintf(stderr, "usage : %s <port>\n", argv[0]);
        exit(1);
    }
    /* Create listenfd */
    listenfd = Open_listenfd(argv[1]);
    while(1){
        clientlen = sizeof(clientaddr);
        connfdp = malloc(sizeof(int));
        *connfdp = Accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        Getnameinfo((struct sockaddr *)&clientaddr, clientlen, hostname, MAXLEN, port, MAXLEN, 0);
        fprintf(stdout, "Accepted connection from (%s : %s)\n", hostname, port);
        pthread_create(&tid, NULL, thread, connfdp);
    }
    /* begin thread routine */
}

void *thread(void *vargp){
    int connfd = *((int *)vargp);
    pthread_detach(pthread_self());
    free(vargp);
    doit(connfd);
    Close(connfd);
    return NULL;
}

void doit(int fd){

    int k;
    int is_static;
    rio_t rio;
    char buf[MAXLEN], method[MAXLEN], uri[MAXLEN], version[MAXLEN];
    char filename[MAXLEN], cgiargs[MAXLEN];
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
        clienterror(fd, "501", "Not Implement", "Not implement this method ", method);
        return;
    }
    read_request(&rio);
    /* request uri */
    is_static = parse_uri(uri, filename, cgiargs);
    if(stat(filename, &sbuf) < 0){
        clienterror(fd, "404", "Not Found", "Not found the file ", filename);
        return;
    }
    /* static or dynamic server */
    if(is_static){
        if(!(S_ISREG(sbuf.st_mode)) || !(sbuf.st_mode & S_IRUSR)){
            clienterror(fd, "403", "Forbidden", "Refuse this request ", filename);
            return;
        }
        static_server(fd, filename, sbuf.st_size);
    }
    else{
        if(!(S_ISREG(sbuf.st_mode)) || !(sbuf.st_mode & S_IRUSR)){
            clienterror(fd, "403", "Forbidden", "Refuse this request ", filename);
            return;
        }
        dynamic_server(fd, filename, cgiargs);
    }
}

/* Begin get_filetype */
void get_filetype(char *filename, char *filetype){
    if(strstr(filename, ".html")){
        strcpy(filetype, "text/html");
    }
    else if(strstr(filename, ".gif")){
        strcpy(filetype, "image/gif");
    }
    else if(strstr(filename, ".png")){
        strcpy(filetype, "image/png");
    }
    else if(strstr(filename, ".jpg")){
        strcpy(filetype, "image/jpg");
    }
    else if(strstr(filename, ".pdf")){
        strcpy(filetype, "application/pdf");
    }
    else {
        strcpy(filetype, "text/plain");
    }
}
/* End get_filetype */

/* begin static server */
void static_server(int fd, char *filename, int filesize){
    
    char buf[MAXLEN], filetype[MAXLEN];
    char *srcp;
    int srcfd;
 
    /* Send response headers to client */
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.1 200 OK\r\n");
    sprintf(buf, "%sServer : xuanlan\r\n", buf);
    sprintf(buf, "%sContent-Type : %s\r\n", buf, filetype);
    sprintf(buf, "%sContent-Size : %d\r\n\r\n", buf, filesize);
    Rio_writen(fd, buf, strlen(buf));
    /* print response headers to server */
    fprintf(stdout, "Response headers : \n%s",buf);

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);  
}
/* end static server */

/* begin dynamic server */
void dynamic_server(int fd, char *filename, char *cgiargs){
    char buf[MAXLEN], *newargs[] = { NULL };
	int i;

	/* Return first part of HTTP response */
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Server : Tiny Web Server\r\n\r\n");
	Rio_writen(fd, buf, strlen(buf));

	if(fork() == 0){
	    /* Real server would set all common gateway interface vars heres */
		setenv("QUERY_STRING", cgiargs, 1);
		/* set args */
		newargs[0] = filename;
		int i = 1;
		char *p;
		while((p = strchr(cgiargs, '&')) != NULL){
		    *p = '\0';
			newargs[i] = cgiargs;
			cgiargs = p + 1;
			i++;
		}
		newargs[i] = cgiargs;
		newargs[i + 1] = NULL;
		/* set args */
		dup2(fd, STDOUT_FILENO);
		execve(filename, newargs, __environ);
	}
	wait(NULL);
}
/* end dynamic server */

/* begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs){

    char *ptr;

    /* Static content */
    if(!strstr(uri, "cgi-bin")){
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if(uri[strlen(uri) - 1] == '/'){
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
    char body[MAXLEN], buf[MAXLEN];
    /* Bulid response headers*/ 
    sprintf(buf, "HTTP/1.1 %s %s\r\n", errnum, shortmes);
    sprintf(buf, "%scontent-type : text/html\r\n\r\n", buf);
    Rio_writen(fd, buf, strlen(buf));
    /* bulid response body */
    sprintf(body, "<html><title>Server Error</title>");
    sprintf(body, "%s<body bgcolor=ffffff>\r\n", body);
    sprintf(body, "%s<h1 style=\"text-align:center\">%s : %s</h1>\r\n<hr>\r\n", body, errnum, shortmes);
    sprintf(body, "%s<h2 style=\"text-align:center\">%s : %s</h2>\r\n", body, longmes, cause);   
    Rio_writen(fd, body, strlen(body));
}
/* end clienterror */
