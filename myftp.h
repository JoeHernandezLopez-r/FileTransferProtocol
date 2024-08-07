#ifndef both_h
#define both_h

//  =======
//  headers
//  ==================
   #include <stdio.h>
   #include <stdlib.h>
   #include <errno.h>
   #include <string.h>
   #include <sys/un.h>
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <unistd.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>
   #include <netdb.h>
   #include <wait.h>
   #include <fcntl.h>
   #include <sys/stat.h>
//  ==================
 
//  ============================
   #define MY_PORT_NUMBER 49999
   #define BACKLOG 4
//  ================

//  client 
    int  cd(char* pathname);
    int  inetconnect(char* name, char* portNum);
    int  ls();
    int  controlDataConnection(int socketfd, char* host);
    void rls(int socketfd, char* host);
    int  rcd(int socketfd, char* pathname);
    int  show(int socketfd, char* pathname, char* host);
    void get(int socketfd, char* pathname, char* host);
    void put(int socketfd, char* pathname, char* host);
    void exitInet(int socketfd);

//  server
    void gloriousErrno(int listenfd);
    void rcdServer(int listenfd,char* path);
    void Lserver(int listenfd, int datasocketfd);
    void Pserver(int listenfd, int dataSocketfd, char*pathname);
    void Gserver(int listenfd, int dataSocketfd, char*pathname);
    int  datasocketServerconnection(int socketfd);

#endif 