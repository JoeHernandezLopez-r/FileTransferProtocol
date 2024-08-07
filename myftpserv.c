// =================
//    description:    
//        runs server code
//        creates data connection with reading D
//        gets files with reading G<>
//        puts files with reading P<>
//        exit with reading Q
//        shows ls with reading L
//        changes directoryu with reading C<>
// =============================================


//  =======
//  headers
//  =================
   #include "myftp.h"

/*
    Notes: gloriosErrno 
    this simply concats E and prints to buffer of the client error

*/

//  ==================
//  void gloriousErrno
//  =================================
    void gloriousErrno(int listenfd){
//      send error 
//      convert to readable for client 
//      vars only needed here so declared here

        char errorbuffer[30] = "E";
        strcat(errorbuffer, strerror(errno));
        strcat(errorbuffer, "\n");
        printf("Child %d: Error: %s\n", getpid(), strerror(errno));
        

//      write error to buffer
        write(listenfd, errorbuffer, strlen(errorbuffer));
    
    }//  void gloriousErrno
//  =======================

/*
    Notes: rcdServer 
    just change directory

*/

//  ==============
//  void rcdServer
//  ========================================
    void rcdServer(int listenfd,char* path){

        errno =0;

//      change to path 
        chdir(path);

//      error        
        if(errno != 0){
            gloriousErrno(listenfd);
        } 

//      else see cur path
        else{
            write(listenfd, "A\n", 2);
            
            char getpath[100];
            getcwd(getpath, 100);
            printf("Child %d: changed working directory too %s\n", getpid(), getpath);
        }

    }// rcdServer
//  =============

/*
    Notes: Lserver 
    send ls output to client 
    errors dealt with bash probably
*/

//  ============
//  void Lserver
//  =============================================
    void Lserver(int listenfd, int datasocketfd){
        
//      vars
        int numRead =0;
        errno=0;
        
//      exec and reddirect
//      parent
        if(fork()){
            close(datasocketfd);
            wait(NULL); 
        }

//      child
        else{
//          close read
            dup2(datasocketfd, 1);
            close(datasocketfd);
            execlp("ls", "ls", "-l", (char*)NULL);
            exit(-1);//fail
        }

//      ack        
        if(errno != 0){
            gloriousErrno(listenfd);
        } 

        else{
            write(listenfd, "A\n", 2);
        }
    }// rlsServer
//  =============

/*
    Notes: PServer
    just opens a file and writes to datasock
    should fail with file exist or cant open it 
*/

//  ============
//  void Pserver
//  ============================================================
    void Pserver(int listenfd, int dataSocketfd, char*pathname){

//      vars
        int numRead =0; 
        errno=0;

        /*  read from socket add \n
        then write to filediscriptor   */ 
        char filebuffer[256];
        int fd;

//      open write to and close
        fd = open(pathname, O_CREAT|O_WRONLY| O_EXCL, 0744);
    
        if(errno!=0){
            gloriousErrno(listenfd);
            close(dataSocketfd);
            return;
        }

        write(listenfd, "A\n", 2);

//      write and close discriptors
        while( (numRead=read(dataSocketfd, filebuffer, 256)) != 0 ){
            write(fd, filebuffer, numRead);
        } 

        close(dataSocketfd);
        close(fd);
    }//  Pserver
//  ============

/*
    Notes: Gserver
    just opens a file and send it to client
    should fail with file doesnt exist or cant open it 
*/

//  ============
//  void Gserver
//  ============================================================
    void Gserver(int listenfd, int dataSocketfd, char*pathname){

//      vars
        char filebuffer[256];
        int fd;
        int numRead=0;
        errno =0;
        struct stat statEnt;

//      check it 
        if (lstat(pathname, &statEnt) == -1) {
            gloriousErrno(listenfd);
            return;
        }

        if( !(S_ISREG(statEnt.st_mode)) ){

//          write error to buffer
            write(listenfd, "ENot a regular file\n", 20);
            return;

        }

//      open write to and close
        errno =0;
        fd = open(pathname, O_RDONLY);

        if(errno!=0){
            gloriousErrno(listenfd);
            return;
        }


	write(listenfd, "A\n", 2);
	  
        /*  read from fd 
        then write to datasocket  */  
        while( (numRead=read(fd, filebuffer, 256)) != 0 ){
            write(dataSocketfd, filebuffer, numRead);
        } 

        close(dataSocketfd);
        close(fd);

    }// Gserver
//  ===========

/*
    Notes: datasocketServerconnection
    sets up data connection with acknowledgments 
*/

//  ==============================
//  int datasocketServerconnection
//  =============================================
    int datasocketServerconnection(int socketfd){
//      vars 
        struct sockaddr_in servAddr;// a strct for serveradd
        int datasocketfd;// for server
        char portbuffer[10];// port array num
        int sockaddrSize =  sizeof(struct sockaddr_in);// self exp 
        errno =0;

//      set up socket
        datasocketfd = socket(AF_INET, SOCK_STREAM, 0);
        if(errno!= 0){
            gloriousErrno(socketfd);
            return -1;
        }

        int check = setsockopt(datasocketfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof (int));
        if(errno!= 0){
            gloriousErrno(socketfd);
            return -1;
        }

//      fill socket
        memset(&servAddr, 0, sizeof(struct sockaddr_in));
        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons(0);// choose random port
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY);// same as the other one but with htonl

//      bind socket
        bind(datasocketfd, (struct sockaddr*) &servAddr, sizeof(servAddr)); 
        if(errno!= 0){
            gloriousErrno(socketfd);
            return -1;
        }
//      get info 
        getsockname(datasocketfd, (struct sockaddr*) &servAddr, &sockaddrSize); 
        if(errno != 0){
            gloriousErrno(socketfd);
            close(datasocketfd);
            return -1;
        }

//      listen with backlog of 1
        listen(datasocketfd, 1); 
        if(errno!= 0){
            gloriousErrno(socketfd);
            close(datasocketfd);
            return -1;
        }

//      so get port to a string then send with a to conhtrol socket with \n at end
        char ackbuffer[10] = "A";
        int portnum = ntohs(servAddr.sin_port);

        sprintf(portbuffer, "%d", portnum);
        strcat(ackbuffer, portbuffer);
        strcat(ackbuffer, "\n");
        write(socketfd, ackbuffer, strlen(ackbuffer));

//      vars for accept
        int length = sizeof(struct sockaddr_in);// for length accept arg
        struct sockaddr_in clientAddr;//  accept arg

//      accept                    
        int datafd = accept(datasocketfd,(struct sockaddr *) &clientAddr,&length);
        if(errno!=0){
            gloriousErrno(socketfd);
            close(datasocketfd);
        } 

        return datafd;

    }// int datasocketServerconnection
//  ==================================

//  ========
//  int main
//  ===========
    int main(){

//      vars 
        struct sockaddr_in servAddr;// a strct for serveradd
        int socketfd;// for server
        int listenfd;// read and write to it

//      set up socket
        socketfd = socket(AF_INET, SOCK_STREAM, 0);
        if( errno!= 0 ){
            fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(-1); 
        }

        int check = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof (int));
        if( errno!= 0 ){
            fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(-1); 
        }

//      fill socket
        memset(&servAddr, 0, sizeof(struct sockaddr_in));
        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons(MY_PORT_NUMBER);// got to make it readable so htons
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY);// same as the other one but with htonl

//      bind socket
        bind(socketfd, (struct sockaddr*) &servAddr, sizeof(servAddr)); 
        if(errno!= 0){
            fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(-1); 
        }

//      listen with backlog of 4
        listen(socketfd, BACKLOG);         
        if(errno!= 0){
            fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(-1); 
       }

//      vars for daemon
        int length = sizeof(struct sockaddr_in);// for length accept arg
        struct sockaddr_in clientAddr;//  accept arg
        char buffer[128] = {'\n'};
        int numRead=0;

//      ==============
//      inifite daemon
        while(1){

//          accept                    
            listenfd = accept(socketfd,(struct sockaddr *) &clientAddr,&length); 
            if(listenfd < 0){
                fprintf(stderr, "Error: %s\n", strerror(errno)); 
            }

//          for connectio displayb and spliting 
            int status;
            int pid = fork();
            char *ipaddress = inet_ntoa(clientAddr.sin_addr);

//          host name 
            char hostName[NI_MAXHOST];  
            getnameinfo((struct sockaddr*)&clientAddr,sizeof(clientAddr),hostName,sizeof(hostName),NULL,0,NI_NUMERICSERV);          
            if(errno!=0){
                hostName[0] = '\0';
                strcpy(hostName, "Erro getnameinfo failed"); 
            }

//          ====
//          prnt  
            if(pid != 0){
            }

//          =================================
//          child runs connection of a person
            else{
                
                printf("Child %d: Client of IP address -> %s\n",getpid(),ipaddress);
                printf("Child %d: Connection accepted from host %s\n",getpid(),hostName);
                memset(buffer,'\n',128);

//              ===========
//              CLIENT LOOP
                while(read(listenfd,buffer, 1)!=0){

//                  Quit
                    if(buffer[0] == 'Q'){ 
                        printf("CHILD %d: Quitting \n", getpid());
                        write(listenfd, "A\n", 2); 
                        close(listenfd);
                        exit(0);
                    }

//                  rcd 
                    else if(buffer[0] == 'C'){
//                      check input from client send to output 
                        numRead = read(listenfd,buffer, 128);
                        buffer[numRead-1] = '\0';
                        printf("Child %d: attempting to change directory too %s\n", getpid(), buffer);
                        rcdServer(listenfd, buffer);
                    } 

//                  data connection required
                    else if(buffer[0] == 'D'){

//                      self exp
                        read(listenfd,buffer, 1);

                        int datasocketfd = datasocketServerconnection(listenfd);
                        if( datasocketfd != -1){

                            read(listenfd,buffer, 1);

//                          rls  
                            if(buffer[0] == 'L' ){
//                              get L
                                read(listenfd,buffer, 2);
                                Lserver(listenfd, datasocketfd);
                            }

//                          get 
                            else if(buffer[0] == 'G'){
//                              get path from client 
                                numRead = read(listenfd,buffer, 128);
                                buffer[numRead-1] = '\0';
                                printf("Child %d: reading file %s\n", getpid(), buffer);
                                Gserver(listenfd, datasocketfd, buffer);
                            }

//                          Put
                            else if(buffer[0] == 'P' ){
//                              get path from client 
                                numRead = read(listenfd,buffer, 128);
                                buffer[numRead-1] = '\0';
                                printf("Child %d: transmitting file %s\n", getpid(), buffer);
                                Pserver(listenfd, datasocketfd, buffer);
                            }

                        }// data socket worked if

                    }// data connection required
//                  ============================

//                  default
                    else{
                        printf("Wrong input\n");
                    }

                }//  END OF CLIENT LOOP
//              =======================

            }// child process
//          ================= 

//          clean up zombies whnohang makes to just go passed it 
            while(waitpid(0, &status, WNOHANHG) != 0){
                if(errno!=0){
                    printf("%s\n", strerror(errno)); 
                }
            } 

        }//  infinte daemon 
//      ===================

    }// int main
//  ============
