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

    int socket_fd, status, server_port;
    struct sockaddr_in servaddr, *ipv4;
    struct addrinfo hints, *servinfo;
    char server_port_str[10], *machine;

    if (argc != 3) {
        printf("Usage: ./client <server port> <machine> \n");
        exit(-1);
    }

    /* Collect command line arguments */
    if (atoi(arg[1]) < 30001 || atoi(arg[1]) > 40000) {
        printf("Invalid Port Number\n");
        exit(-1);
    }


    server_port = atoi(arg[1]);
    machine = arg[2];

    sprintf(server_port_str, "%d", server_port);

   

    
    /* socket create and verification */
    socket_fd = socket(servinfo->ai_family,
                    servinfo->ai_socktype,
                    servinfo->ai_protocol);
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
    servaddr.sin_port = htons(server_port); 
    servaddr.sin_addr = ipv4->sin_addr; 


    /* connect the client socket to server socket */
    if (connect(socket_fd,
            servinfo->ai_addr,
            servinfo->ai_addrlen) != 0)  {
        printf("connection with the server failed...\n");
        exit(0);
    } else {
        printf("connected to the server..\n");
    }

    send_data_to_server(socket_fd);

    close(socket_fd);
    return 0;
}