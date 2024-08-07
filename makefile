all: myftpserv myftp
myftpserv: myftpserv.o 
	gcc myftpserv.o -o myftpserv
myftp: myftp.o 
	gcc myftp.o -o myftp
myftp.o: myftp.c
	gcc -c myftp.c

myftpserv.o: myftpserv.c
	gcc -c myftpserv.c
clean:
	rm *.o myftp myftpserv