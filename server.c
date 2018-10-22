#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 4096 /*max text line length*/
#define BUFSIZE 4096
#define SERV_PORT 8093 /*port*/
#define LISTENQ 8 /*maximum number of client connections */

char httpOK[] = "HTTP/1.1 200 Ok\r\n";
char contentType[] = "Content-Type: text/html; charset=UTF-8\r\n\r\n";

char header[] = "HTTP/1.1 200 Ok\r\nContent-Type: text/html\r\n\r\n";

char resp404[] =
"HTTP/1.1 404 NOT FOUND\r\n";


//converts long to string
char * toString(char * str, long num) {
    sprintf(str, "%ld", num);
    return str;
}

//builds the http header
void getHeader (char * header1, char * file, long fileSize) {
    char fileName[MAXLINE];
    char ext[10];
    char buf[MAXLINE];
    char returnHeader[MAXLINE];

    strcpy(header1, "HTTP/1.1 200 Ok\r\nContent-Type: ");
    strcpy(fileName, file);
    strtok(fileName, ".");
    strcpy(ext, strtok(NULL, "."));
    if (strcmp(ext, "html") == 0) {
        strcat(header1, "text/html");
    }
    else if (strcmp(ext, "txt") == 0) {
        strcat(header1, "text/plain");
    }
    else if (strcmp(ext, "png") == 0) {
        strcat(header1, "image/png");
    }
    else if (strcmp(ext, "gif") == 0) {
        strcat(header1, "image/gif");
    }
    else if (strcmp(ext, "jpg") == 0) {
        strcat(header1, "image/jpg");
    }
    else if (strcmp(ext, "css") == 0) {
        strcat(header1, "text/css");
    }
    else if (strcmp(ext, "js") == 0) {
        strcat(header1, "application/javascript");
    }
    else {
        printf("%s\n", "Illegal type!");
    }

    //strcat(header1, "\r\nContent-Length: ");

    //bzero(buf, BUFSIZE);
    //strcat(header1, toString(buf, fileSize));

    strcat(header1, "\r\n\r\n");
    printf("%s%s\n", "Header content: ", header1);
}

//processes a single get request
void processRequest (char * buf1, int connfd) {
    char type[10];
    char file2[MAXLINE];
    char buf[BUFSIZE];
    char file[MAXLINE];
    size_t byteCount = 0;
    long fileSize;
    FILE * file3;
    int n = 0;
    //char arg[MAXLINE];
    char * arg;
    arg = strtok(buf1, " ");
    strcpy(type, arg);
    arg = strtok(NULL, " ");
    strcpy(file2, arg);
    //puts(buf);
   //send(connfd, buf, n, 0);

   if (strlen(file2) < 2) strcpy(file2, "/index.html");

   strcpy(file, "www");
   strcat(file, file2);

   if( !(file3 = fopen(file, "r")) ) {
        send(connfd, resp404, sizeof(resp404), 0);
        printf("%s %s %s\n","Returning 404...", type, file);
        return;
    }

    fseek(file3, 0L, SEEK_END);
    fileSize = ftell(file3);

    fseek(file3, 0L, SEEK_SET);


    bzero(buf, MAXLINE);
    getHeader(buf, file2, fileSize);
    send(connfd, buf, strlen(buf), 0);
    //send(connfd, webpage, sizeof(webpage), 0);


    while ((byteCount = fread(buf, 1, BUFSIZE, file3)) > 0) {

        n = send(connfd, buf, sizeof(buf), 0);
        if (n < 0)
            error("ERROR in sendto");
        printf("Sent Chunk\nSize %i\n", byteCount);
        bzero(buf, BUFSIZE);
    }

   //send(connfd, webpage, sizeof(webpage), 0);
    printf("%s %s %s\n","sent and closed...", type, file);
}

int main (int argc, char **argv)
{
 int listenfd, connfd, n;
 socklen_t clilen;
 char buf[MAXLINE];
 struct sockaddr_in cliaddr, servaddr;

 //creation of the socket
 listenfd = socket (AF_INET, SOCK_STREAM, 0);

    //setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &o, sizeof(int));

 //preparation of the socket address
 servaddr.sin_family = AF_INET;
 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 servaddr.sin_port = htons((unsigned short)atoi(argv[1]));

 bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

 listen(listenfd, LISTENQ);

 printf("%s\n","Server running...waiting for connections.");

 for ( ; ; ) {

  clilen = sizeof(cliaddr);
  printf("%s\n","Listing for request...");
  connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
  printf("%s %i \n","Received request...", connfd);
  bzero(buf, MAXLINE);
  n = recv(connfd, buf, MAXLINE,0);

  /*
  if (strcmp(buf, "stop") == 0) {
    printf("%s\n","stoping...");
    close(connfd);
    close(listenfd);
    return 0;
  } */

  //while ( (n = recv(connfd, buf, MAXLINE,0)) > 0)  {

   if (fork() == 0) {
    processRequest(buf, connfd);
    close(connfd);
    close(listenfd);
    return 0;
   }
   close(connfd);

 }
 //close listening socket
 close(listenfd);
}
