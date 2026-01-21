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

    char buff[1024];
    ssize_t len;

    for (;;) {

        printf("Enter the string: ");

        if (fgets(buff, sizeof(buff), stdin) == NULL)
            break;

        len = strlen(buff);

        if (write(socket_fd, buff, len) != (ssize_t)len) {
            perror("write");
            break;
        }

        if (strncmp(buff, "quit", 4) == 0) {
            printf("Client Exit...\n");
            break;
        }
    }
}

int main(int argc, char *arg[]){

    int client_socket, status, server_port;
    struct addrinfo hints, *servinfo;
    char server_port_str[10];

    /* Check Command line arguments validity */
    if (argc != 2) {
        printf("Usage: ./client <server port> \n");
        exit(-1);
    }

    server_port = atoi(arg[1]);
    
    if (server_port < 30000 || server_port > 40000) {
        fprintf(stderr, "Port must be between 30000 and 40000\n");
        exit(1);
    }

    sprintf(server_port_str, "%d", server_port);

    /* Get IP address of server */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;

    /* Get server IP address and Port */
    if ((status = getaddrinfo("127.0.0.1",
                                server_port_str, 
                                &hints, 
                                &servinfo)) != 0) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(status));
        exit(1);
    }
    
    /* client socket create and verification */
    client_socket = socket(servinfo->ai_family,
                    servinfo->ai_socktype,
                    servinfo->ai_protocol); 
    if (client_socket == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } else {
        printf("Socket successfully created..\n"); 
    }

    /* connect the client socket to server socket */
    if (connect(client_socket, servinfo->ai_addr, servinfo->ai_addrlen)){
        printf("connection with the server failed...\n");
        exit(0);
    } else {
        printf("connected to the server..\n");
    }

    send_data_to_server(client_socket);

    close(client_socket);
    return 0;
}