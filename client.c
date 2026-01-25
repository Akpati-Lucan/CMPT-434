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


void get_data_from_server(int connect_fd){

    char buff[1024];
    ssize_t nbytes;

     while (1) {
        nbytes = read(connect_fd, buff, sizeof(buff) - 1);
        if (nbytes < 0) {
            perror("read");
            return;
        } else if (nbytes == 0) {
            printf("Server disconnected\n");
            return;
        }
        buff[nbytes] = '\0';
        printf("Client recieved -  \n");
        printf("%s", buff);
        /* stop after newline */
        if (strchr(buff, '\n')) {
            break;
        }
    }
}


/* An array of strings that will hold the users input 
    A double pointer */
char input_str[8][100];

void send_data_to_server(int socket_fd) {
    char buff[1024];
    char sendbuf[1024];
    ssize_t len;
    char *token;
    int n;


    for (;;) {
        n = 0;
        memset(input_str, 0, sizeof(input_str));

        printf("Enter the string: ");
        if (fgets(buff, sizeof(buff), stdin) == NULL)
            break;

        strncpy(sendbuf, buff, sizeof(sendbuf) - 1);
        sendbuf[sizeof(sendbuf) - 1] = '\0';


        
        /* Tokenize and add tokens to input str */ 
        token = strtok(buff, " \n");
        while (token != NULL) {
            if (n >= 5) {
                printf("Too many tokens\n");
                break;
            }
            strncpy(input_str[n], token, sizeof(input_str[n]) - 1);
            n += 1;
            token = strtok(NULL, " \n");
        }
        
        len = strlen(sendbuf);

        if (write(socket_fd, sendbuf, len) != (ssize_t)len) {
            perror("write");
            break;
        }

        if (strncmp(sendbuf, "quit", 4) == 0) {
            printf("Client Exit...\n");
            break;
        }

         if (strcmp(input_str[0], "getvalue") == 0 ||
            strcmp(input_str[0], "getall") == 0) {
            get_data_from_server(socket_fd);
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