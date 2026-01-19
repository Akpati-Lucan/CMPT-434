/*
# Name: Lucan Akpati
# NSID: jbs671
# Student Number: 11331253
*/

#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 

#define TCP_PORT 8080 

int server_port;
char *machine;


void send_data_to_server(int socket_fd) {

    char buff[256];
    int n;
    for (;;) {
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n');
        write(socket_fd, buff, sizeof(buff));


        if ((strncmp(buff, "exit", 4)) == 0) {
            printf("Client Exit...\n");
            break;
        }
    }
}

int main(int argc, char *arg[]){

    int socket_fd, status;
    struct sockaddr_in servaddr;
    struct addrinfo hints, *servinfo;
    char server_port_str[10];

    if (argc != 3) {
        printf("Usage: ./client <server port> <machine> \n");
        exit(-1);
    }

    /* Collect command line arguments */
    if (atoi(arg[1]) < 30001 || atoi(arg[1]) > 40000 || atoi(arg[3]) < 30001 || atoi(arg[3]) > 40000) {
        printf("Invalid Port NUmbers\n");
        exit(-1);
    }

    server_port = atoi(arg[1]);
    machine = arg[2];

    sprintf(server_port_str, "%d", server_port);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(machine, 
			      remote_port_str, 
			      &hints, &servinfo)) != 0) {
	fprintf(stderr, "gai error: %s\n", gai_strerror(status));
	exit(1);
    } 
    
    ipv4 = (struct sockaddr_in *) servinfo->ai_addr;


    /* socket create and verification */
    socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (socket_fd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } else {
        printf("Socket successfully created..\n"); 
    }

    /* Set all garbage data in servaddr struct to 0 */
    bzero(&servaddr, sizeof(servaddr)); 

     /* Socket address structure initialization */
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(TCP_PORT); 


    /* connect the client socket to server socket */
    if (connect(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    } else {
        printf("connected to the server..\n");
    }

    send_data_to_server(socket_fd);

    close(socket_fd);
    return 0;
}