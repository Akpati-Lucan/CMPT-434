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

/* Create a struct for key-value pairs */
typedef struct k_val{
    int key;
    char value[100];
}k_val;


/* Create an array of Key-value structs called Dict */
struct k_val dict[100];



void get_data_from_client(int connect_fd){

    char buff[256];
    
    for (;;) { 
        
        bzero(buff, 256); 
  
        /* read the message from client and copy it in buffer */ 
        read(connect_fd, buff, sizeof(buff)); 
        /* print buffer which contains the client contents */
        printf("From client: %s\t To client : ", buff); 
        bzero(buff, 256); 

        /* if msg contains "Exit" then server exit and chat ended. */
        if (strncmp("exit", buff, 4) == 0) { 
            printf("Server Exit...\n"); 
            break; 
        } 
    } 
}

int main(int argc, char *arg[]){

    int socket_fd, connect_fd, status, server_port;
    socklen_t len;
    struct sockaddr_in servaddr;
    struct addrinfo hints, *servinfo;
    char server_port_str[16];

    /* Choose port number for server */
    if (argc != 2) {
        printf("Usage: ./server <server port> \n");
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
    hints.ai_flags = AI_PASSIVE;

    
    if ((status = getaddrinfo(NULL, 
                                server_port_str, 
                                &hints, 
                                &servinfo)) != 0) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(status));
        exit(1);
    }

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

    /* Bind socket */ 
    if (bind(socket_fd,
             servinfo->ai_addr,
             servinfo->ai_addrlen) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } else {
        printf("Socket successfully binded..\n"); 
    }
    
    freeaddrinfo(servinfo);

    /* Now server is ready to listen and verification */
    if ((listen(socket_fd, 10)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } else {
        printf("Server listening..\n"); 
    }

    /* Accept the data packet from client and verification */
    connect_fd = accept(socket_fd, NULL, NULL); 
    if (connect_fd < 0) { 
        printf("server accept failed...\n"); 
        exit(0); 
    } else {
        printf("server accept the client...\n"); 
    }
  
    get_data_from_client(connect_fd);

    close(connect_fd);
    close(socket_fd);
    return 0;
}