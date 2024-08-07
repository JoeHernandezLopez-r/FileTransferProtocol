//  ===============
//    description:    
//      has client code and serveral commands
//      ls just show client side directory
//      rls get servers directory and prints to client 
//      rcd changes directory of server
//      cd changes clients directory
//      get gets from directory files
//      put puts file to server
//      show contents of something from file in server 
// ===============================================

//  =======
//  headers
//  =================
   #include "myftp.h"

/*
    Notes: inetconnect
    connects inet takes in a hostname and 
    returns a int value for the socket discriptor
    if anything goes wrong it simply returns a value of -1
*/

//  ===============
//  int inetconnect
//  ===========================================
    int inetconnect(char* name, char* portNum){

//      vars
        struct sockaddr_in address; 
        struct addrinfo addrH, *data;// data  
        int socketfd;
        errno=0;

//      set up addressinfo 
        memset(&addrH, 0 , sizeof(addrH));
        addrH.ai_family = AF_INET;
        addrH.ai_socktype = SOCK_STREAM;

//      get the info 
        int check = getaddrinfo(name ,portNum,&addrH, &data); 
        if(check != 0){
            fprintf(stdout, "Error: %s\n", gai_strerror(check));
            return -1;
        } 

//      socket
        socketfd = socket(data->ai_family, data->ai_socktype, 0); 
        if(errno!=0){
            fprintf(stdout, "Error: %s\n", strerror(errno));
            return -1;
        }

//      connect
        connect(socketfd, data->ai_addr, data->ai_addrlen);; 
        if(errno!=0){
            fprintf(stdout, "Error: %s\n", strerror(errno));
            return -1;
        }

        return socketfd; 

    }// inetconnect
//  ===============

/*
    Notes: ls
    printing out ls dies with nothing else
    so need to fork the process to something else before it dies 
    so make parent birth 2procs and it returns to parent when done 
    or just make a child process do the code simlar to previous assignemnet
    return -1 if failure
*/

//  ======
//  int ls
//  =========
    int ls(){

//      reset errno
        errno = 0; 

//      first fork
        int check = fork();
        if(errno != 0){
            fprintf(stderr, "%s\n", strerror(errno));
            return -1;  
        }

//      originol prnt
        if(check != 0){
            wait(NULL);
        }

//      firstborn
        else{

//          vars 
            int fd[2];
            int pid;// process id 

//          piping
            int errnoNum = pipe(fd);

            if(errnoNum == -1){
                errnoNum = errno; 
                fprintf(stderr, "%s\n", strerror(errno));
                return -1;  
            }

//          forking
            pid = fork();
            
//          too many processes can make it fail
            if(pid == -1){
                errnoNum = errno; 
                fprintf(stderr, "%s\n", strerror(errno));
                return -1;
            }   

//          parent reads
            if(pid!=0){

                wait(NULL); 

//              close write
                close(fd[1]);
                dup2(fd[0], 0); 
                close(fd[0]);
                execlp("more", "more", "-20", (char*)NULL); 
            }

//          child  writes
            else{ 
                
//              close read
                close(fd[0]); 
                dup2(fd[1], 1);
                close(fd[1]);  
                execlp("ls", "ls", "-l", (char*)NULL); 
            }

//          if it makes this far something went wrong above cause of exec
            fprintf(stderr, "%s\n", strerror(errno));
            return -1;

        }// firstborn
//      =============

    }// ls
//  ======

/*
    Notes: controlDataConnection
    all this does is handle the acknowledgements and potential errors
    as well as try to connect with another temporary socket for data
    return -1 if anything goes wrong 
*/

//  =========================
//  int controlDataConnection
//  ====================================================
    int controlDataConnection(int socketfd, char* host){

//      vars 
        char ackbuffer[128];
        int numRead =0;

//      send D
        write(socketfd, "D\n", 2);

//      ============================
//      read ACKNOWLEDGE from server
        numRead = read(socketfd, ackbuffer, 1); 

        if(ackbuffer[0] == 'A'){
        }
        else if(ackbuffer[0] == 'E'){
            numRead = read(socketfd, ackbuffer, 128); 
            write(1, ackbuffer, numRead); 
            return -1;
        }

//      =====================
//      read PORT from server
        numRead = read(socketfd, ackbuffer, 10); 
        ackbuffer[numRead-1] = '\0';  

        if(ackbuffer == NULL){
            fprintf(stderr, "missing port number cannot complete action\n");
            return -1;
        }

//      data socket 
        int tempsocket = inetconnect(host, ackbuffer);
        
        if(tempsocket == -1){
            fprintf(stderr,"error data socket not connected \n");
            return -1;
        }

        return tempsocket;

    }// int controlDataConnection
//  =============================

//  ========
//  void rls
//  ===================================
    void rls(int socketfd, char* host){

//      vars
        char buffer[128];
        int numRead=0;
        errno=0;

//      data conection
        int dataSocketfd = controlDataConnection(socketfd, host);
        write(socketfd, "L\n", 2); 

        numRead = read(socketfd, buffer, 1); 

//      accept
        if(buffer[0] == 'A'){
            read(socketfd, buffer, 128);

//          fork
            int check = fork();
            if(errno != 0){
                fprintf(stderr, "%s\n", strerror(errno));
            }

//          parent
            if(check != 0){
                close(dataSocketfd);
                wait(NULL); 
            }

//          child
            else{
//          close write
                dup2(dataSocketfd, 0); 
                close(dataSocketfd);
                execlp("more", "more", "-20", (char*)NULL); 
            }

//          if it makes this far something went wrong above cause of exec
            if(errno!=0){
                fprintf(stderr, "%s\n", strerror(errno));
            }
            return;
        }

//      error
        else if(buffer[0] == 'E'){
            numRead = read(socketfd, buffer, 128); 
            write(1, buffer, numRead); 
            return;
        }

    }//  void rls
//  =============

//  =======
//  int rcd
//  ======================================
    int rcd(int socketfd, char* pathname){

//      vars
        int lengthOfString =0;
        char buffer[128];
        int numRead =0;
        errno=0;

//      combine C and path
        buffer[0] = 'C';
        buffer[1] = '\0';

        strcat(buffer, pathname);
        lengthOfString = strlen(buffer);

//      swap out null for newline
        buffer[lengthOfString] = '\n'; 

//      send to server
        write(socketfd, buffer, lengthOfString+1);
        read(socketfd, buffer, 1);

//      accept or error
        if(buffer[0] == 'A'){
            read(socketfd, buffer, 128);
        }
        else if(buffer[0] == 'E'){
            numRead=read(socketfd, buffer, 128);
            write(1, buffer, numRead); 
            return -1; 
        }

    }// int rcd
//  ============

/*
    Notes: show
    all we do is send G and concateate path
    then follow up by redir3ecting output to 
    more with exec some issues with profserv 
    if it doesnt find the file it seems to break and i dont 
    know why 

*/

//  ========
//  int show
//  ===================================================
    int show(int socketfd, char* pathname, char* host){

//      vars
        int lengthOfString =0;
        char buffer[128];
        int numRead =0;
        errno=0;

//      data connection
        int dataSocketfd = controlDataConnection(socketfd, host);

//      path making and sending control G<path>
        buffer[0] = 'G';
        buffer[1] = '\0';

        strcat(buffer, pathname);
        lengthOfString = strlen(buffer);
        buffer[lengthOfString] = '\n'; 

        write(socketfd, buffer, lengthOfString+1);
        read(socketfd, buffer, 1);

//      accept
        if(buffer[0] == 'A'){
            read(socketfd, buffer, 128); 
        }
//      error
        else if(buffer[0] == 'E'){
            numRead = read(socketfd, buffer, 128); 
            write(1, buffer, numRead); 
            return -1;
        }

//      exec and redirect
        int check = fork();

//      parent
        if(check != 0){
            close(dataSocketfd);
            wait(NULL); 
        }

//      child
        else{
//          close 
            dup2(dataSocketfd, 0); 
            close(dataSocketfd);
            execlp("more", "more", "-20", (char*)NULL); 
        }

        if(errno != 0){
            fprintf(stderr, "%s\n", strerror(errno));
            return -1;
        }
        return 0;

    }// int show
//  ============


/*
    Desc: get
    so basically retrieve a file, use the name of file on server side as the
    name for the of the file created in client 
    server side errors
    pathname doesnt exist
    not regular
    not readable
    
    client side errors
    cant open
    cant create
*/

//  ========
//  void get
//  ===================================================
    void get(int socketfd, char* pathname, char* host){
        
//      data connection set up
        char buffer[128];
        char filebuffer[256];
        int numRead =0; 
        char pathnameEnd[256];
        errno=0;

//      check if write permissions
        struct stat statEnt;

//      ==============================================
//      convert or get end of pathname if not already
        int fullpathlen = strlen(pathname)+1;
        int Endofpathlen =0;
        int j =0;
        int i = fullpathlen;

//      find length of end of path
	    while(pathname[i] != '/' || i ==0){
            i--;
            Endofpathlen++;
        }

//      fill new pathbuffer
        while(Endofpathlen!=0){
            pathnameEnd[j] = pathname[fullpathlen - Endofpathlen];
            Endofpathlen--;

        }
//      =============================================
	
//      check 
        if (lstat(pathnameEnd, &statEnt) != -1) {
            fprintf(stderr, "Exists\n");
            return;
        }

        int dataSocketfd = controlDataConnection(socketfd, host);


//      path making and sending control G<path>
        buffer[0] = 'G';
        buffer[1] = '\0';

        strcat(buffer, pathname);

        int lengthOfString = strlen(buffer);

        buffer[lengthOfString] = '\n'; 

        write(socketfd, buffer, lengthOfString+1);
        read(socketfd, buffer, 1);

//      accept
        if(buffer[0] == 'A'){
            read(socketfd, buffer, 128); 
        }

//      error and close file disc
        else if(buffer[0] == 'E'){
            numRead = read(socketfd, buffer, 128); 
            write(1, buffer, numRead);
            return;
        }

//      open write to and close
        int fd = open(pathnameEnd, O_CREAT|O_WRONLY| O_EXCL, 0744);
     
        if(errno!=0){
            fprintf(stdout, "%s\n", strerror(errno));
            close(dataSocketfd);
            return;
        }
            
        printf("File being transmitted is %s\n", pathnameEnd); 

        /*  read from socket add \n
        then write to filediscriptor   */ 

        while( (numRead=read(dataSocketfd, filebuffer, 256)) != 0 ){
            write(fd, filebuffer, numRead);
        } 

        close(dataSocketfd);
        close(fd);

    }// void get
//  ============

//  ========
//  void put
//  ===================================================
    void put(int socketfd, char* pathname, char* host){

//      vars
        char filebuffer[256];
        int fd;
        errno =0;
        char buffer[128];
        int numRead =0; 
        struct stat statEnt;

        if (lstat(pathname, &statEnt) == -1) {
            fprintf(stderr, "%s\n", strerror(errno));
            return;
        }

        if( !(S_ISREG(statEnt.st_mode)) ){
            fprintf(stderr, "Not a regular file\n");
            return;
        }

//      open 
        fd = open(pathname, O_RDONLY);

        if(errno!=0){
            fprintf(stderr, "%s\n", strerror(errno));
            return;
        }
        
        printf("File being read is %s\n", pathname); 


//      set up data connect
//      data connection set up
        int dataSocketfd = controlDataConnection(socketfd, host);

//      path making and sending control P<path>
        buffer[0] = 'P';
        buffer[1] = '\0';

        strcat(buffer, pathname);
        int lengthOfString = strlen(buffer);
        buffer[lengthOfString] = '\n'; 

        write(socketfd, buffer, lengthOfString+1);
        read(socketfd, buffer, 1);

//      accept
        if(buffer[0] == 'A'){
            read(socketfd, buffer, 128); 
        }
//      error
        else if(buffer[0] == 'E'){
            numRead = read(socketfd, buffer, 128); 
            write(1, buffer, numRead); 
            close(dataSocketfd);
            return;
        }

        printf("reading file and sending to datasocket %s\n", pathname); 

        /*  read from fd 
        then write to datasocket  */ 
        while( (numRead=read(fd, filebuffer, 256)) != 0 ){
            write(dataSocketfd, filebuffer, numRead);
        } 

        close(dataSocketfd);
        close(fd);

    }// void put
//  ============

/*
    Notes: cd
    simply change directories to pathname
    and print error if there is a problem
    dont exit out because its not a fatal error
*/

//  ======
//  int cd 
//  =======================
    int cd(char* pathname){

        errno =0; 

//      change to path 
        chdir(pathname);

//      error        
        if(errno != 0){
            fprintf(stderr, "%s\n", strerror(errno));
            return -1;
        } 

//      else see cur path
        else{
            char getpath[100];
            getcwd(getpath, 100);
            printf("Working directory is %s\n", getpath);
            return 0;
        }

    }// int cd
//  ==========

//  =============
//  void exitInet
//  ============================
    void exitInet(int socketfd){
//      vars
        char buffer[128];
        int numRead =0;

//      write Q and deal with acknowldements
        write(socketfd, "Q\n", 2); 
        read(socketfd, buffer, 128);

//      accept
        if(buffer[0] == 'A'){
            printf("Exiting\n");
            close(socketfd);
            exit(0);
        }
//      error
        else if(buffer[0] == 'E'){
            numRead = read(socketfd, buffer, 128); 
            write(1, buffer, numRead); 
            printf("error acknowledged fatal error exit\n"); 
            exit(-1);
        }

    }// exitInet
//  ============

//  ========
//  int main
//  ================================= 
    int main(int argc, char* argv[]){

//      vars
        char command[128]; // command input buffer  
        char spaces[] =" \n\v\r\t\f";// isspace filter
        char* argv1;
        char* argv2; 
        int numRead =0; 

//      ==========================
//      CONNECT TO INET FIRST TIME
        if(argc != 2){
            fprintf(stderr, "MUST ONLY BE 2 ARGUEMENTS\n");
            exit(-1);
        }
        int socketfd = inetconnect(argv[1], "49999");

        if(socketfd == -1){
            fprintf(stderr,"FATAL ERROR EXITING\n");
            exit(-1);  
        }
//      =

//      ======================
//      start of do while menu
//      ===
        do{
            printf("MFTP > ");
            fflush(stdout);

//          getinput turn to string
            numRead = read(0, command, 128);  
            command[numRead-1] = '\0';

//          convert to 2 strings                                    
            argv1 = strtok(command, spaces);
            argv2 = strtok(NULL, spaces);

//          if argv1 not null
            if(argv1!=NULL){  
//              ls 
                if(strcmp(argv1, "ls") == 0){
                    ls(); 
                }
//              rls
                else if(strcmp(argv1, "rls") == 0 ){ 
                    rls(socketfd, argv[1]); 
                }
//              cd 
                else if(strcmp(argv1, "cd") == 0 && argv2 !=NULL){
                    cd(argv2); 
                }
//              rcd
                else if(strcmp(argv1, "rcd") == 0 && argv2 !=NULL){
                    rcd(socketfd,argv2); 

                }
//              show
                else if(strcmp(argv1, "show") == 0 && argv2 !=NULL){
                    show(socketfd, argv2, argv[1]); 
                }

//              put
                else if(strcmp(argv1, "put") == 0 && argv2 !=NULL){
                    put(socketfd, argv2, argv[1]); 
                }
//              get 
                else if(strcmp(argv1, "get") == 0 && argv2 !=NULL){
                    get(socketfd, argv2, argv[1]);   
                }
//              exit
                else if(strcmp(argv1, "exit") == 0){
                    exitInet(socketfd); 
                }
//              default
                else{
                    printf("WRONG INPUT\n");
                }

            }

//          not
            else{
                printf("WRONG INPUT\n");
            }

        }while(strcmp(command, "exit") != 0);
//      END OF while menu
//      =================

    }// int main
//  ============
