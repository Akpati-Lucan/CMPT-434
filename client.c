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


void get_data_from_client(int connect_fd){

    char buff[1024];
    ssize_t nbytes;
    char *token;
    int n;
    int status;

    while (1) { 
        
        memset(buff, 0, sizeof(buff));
        n = 0;
  
        /* read the message from client and copy it in buffer */ 
        nbytes = read(connect_fd, buff, sizeof(buff) - 1);

        if (nbytes < 0) {
            perror("read");
            break;
        }

        if (nbytes == 0) {
            printf("Client disconnected\n");
            break;
        }

        /* Tokenize and add tokens to input str */ 
        token = strtok(buff, " \n");
        while (token != NULL) {
            if (n >= 5) {
                printf("Too many tokens\n");
                break;
            }
            strcpy(input_str[n], token);
            n += 1;
            token = strtok(NULL, " \n");
        }

        status = validate_str(input_str);
        if (status != 0){
            printf("Wrong Input\n");
            break;
        }

        /* if msg contains "server exit" then server exit and chat ended. */
        if (strncmp(buff, "shutdown", 8) == 0) {
            printf("Server Exit...\n");
            break;
        }
    } 
}

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
    char server_port_str[10], *host_name;

    /* Check Command line arguments validity */
    if (argc != 3) {
        printf("Usage: ./client <server port> <hostname> \n");
        exit(-1);
    }

    server_port = atoi(arg[1]);
    host_name = arg[2];
    
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
    if ((status = getaddrinfo(host_name,
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