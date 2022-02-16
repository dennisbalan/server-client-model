#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/types.h>
//main can take 2 or 3 command line arguments and is a server/client program. The server side takes 2 command line args, prints the number of times a host has connected, and writes the current date to the client. The client takes 3 inputs, the 3rd of which is the hostname. The client connects to the server using the hostname and prints the data it has recieved from the server, the current date
int main(int argc, char **argv){
	//client
	//if there are 3 arguments, the client is set up. The 3rd argument is the address of the address to connect to. The client connects to the server and prints a date that the server sends to it
	if(argc == 3){
		//socket_fd is the socket
		int socket_fd;
		struct addrinfo client, *data;
		//create a socket 
		socket_fd = socket(AF_INET, SOCK_STREAM, 0);
		if(socket_fd == -1){
			perror("Error");
			exit(1);
		}
		if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
			perror("Error");
			exit(1);
		}
		//initialize client
		if(memset(&client,0,sizeof(client)) < 0){
			perror("Error");
			exit(1);
		}
		client.ai_family = AF_INET;
		client.ai_socktype = SOCK_STREAM;
		//use argv[2] or the 3rd command line arguement to set up the text host name
		int error = getaddrinfo(argv[2],"49999", &client, &data);
		if(error != 0){
			fprintf(stderr,"Error: %s\n",gai_strerror(error));
			exit(1);
		}
		socket_fd = socket(data->ai_family,data->ai_socktype,0);
	       	if(socket_fd == -1){
			perror("Error");
			exit(1);
		}	
		//connect to the server
		if(connect(socket_fd,data->ai_addr,data->ai_addrlen) < 0){
			perror("Error");
			exit(1);
		}
		char buffer[18];
		int read_number;
		//read the date into the buffer
		read_number = read(socket_fd,buffer,18);
		if(read_number != 18){
			int error = errno;
			fprintf(stderr,"Error: %s\n",strerror(error));
		}
		//print the buffer and a newline
		printf("%s\n",buffer);
		fflush(stdout);

	}
	//server
	//server needs 2 command line args. It prints the amount of times a client connects to it and the client's name. It writes the current date to the client for it to read
	if(argc == 2){
		struct sockaddr_in serveraddr;
		int socketfd, listenfd;
		//create a socket
		socketfd = socket(AF_INET, SOCK_STREAM, 0);
		if(socketfd == -1){
			perror("Error");
			exit(1);
		}
		if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
			perror("Error");
			exit(1);
		}
		//initialize serveraddr
		if(memset(&serveraddr,0,sizeof(struct sockaddr_in)) < 0){
			perror("Error");
			exit(1);
		}
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(49999);
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
		//bind the address
		if(bind(socketfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0){
			perror("Error");
			exit(1);
		}
		//listen for any connections
		if(listen(socketfd,1) < 0){
			perror("Error");
			exit(1);
		}
		int count = 0;
		while(1){
				//fork when you get a connection
				if(fork()){
					//accept the text host name
					int length = sizeof(struct sockaddr_in);
                              		struct sockaddr_in client;
                               	 	if((listenfd = accept(socketfd,(struct sockaddr*) &client,&length)) < 0){
                                        	perror("Error");
                                        	exit(1);
                                	}
					//get the client address name and print it along with how many times it has connected
                                	char name[255];
                               		int entry = getnameinfo((struct sockaddr*)&client,sizeof(client),name,sizeof(name),NULL,0, NI_NUMERICSERV);
                                	if(entry != 0){
                                        	fprintf(stderr,"Error: %s\n",gai_strerror(entry));
						exit(1);
                                	}
					//increment the amount of times a client has connected
                               		count++;
                                	printf("%s %d\n",name,count);
                                	fflush(stdout);
				}
				else{
					//get the date on the Linux Machine and save it in a buffer size 18
					time_t clock;
					time(&clock);
					char buffer[18];
					sprintf(buffer,"%s\n",ctime(&clock));
					if(buffer == NULL){
						int error = errno;
						fprintf(stderr,"Error: %s\n",strerror(error));
						exit(1);
					}
					//buffer[18] = '\n';
					//write to the buffer
					int written = write(listenfd,buffer,18);
					fflush(stdin);
					//close the connection
					close(listenfd);
					//exit the child
					exit(0);
				}
		}
	}
}	
