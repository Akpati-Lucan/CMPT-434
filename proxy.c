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
#include <pthread.h>


int tcp_socket_fd, status, server_port;
struct addrinfo hints, *servinfo;
char server_port_str[16];
pthread_t thread;

void send_data_to_server(){

}

void get_data_from_server(){

}


void send_data_to_client(int tcp_socket_fd, char *msg) {
    ssize_t total = 0;
    ssize_t len = strlen(msg);

    while (total < len) {
        ssize_t n = write(tcp_socket_fd, msg + total, len - total);
        if (n < 0) {
            perror("write");
            return;
        }
        total += n;
    }
}

void get_data_from_client(int client_socket_fd){

    char buff[1024];
    ssize_t nbytes;

    while (1) { 
        
        memset(buff, 0, sizeof(buff));
  
        /* read the message from client and copy it in buffer */ 
        nbytes = read(client_socket_fd, buff, sizeof(buff) - 1);

        if (nbytes < 0) {
            perror("read");
            break;
        } else if (nbytes == 0) {
            printf("Client disconnected\n");
            break;
        }
        
        /* Stub send data back to client */
        send_data_to_client(client_socket_fd, "Proxy server recieved \n");
        /* Send to real server */
        send_data_to_server();


        /* if msg contains "server exit" then server exit and chat ended. */
        if (strncmp(buff, "shutdown", 8) == 0) {
            printf("Server Exit...\n");
            break;
        }
    } 
}

void *handle_client(void *arg)
{
    int client_socket_fd = *(int *)arg;
    free(arg);
    get_data_from_client(client_socket_fd);
    close(client_socket_fd);
    pthread_exit(NULL);
}

/* Accept incoming connections */
void *accept_client(void *arg)
{
    int *client_socket_fd;
    pthread_t client_thread;
    tcp_socket_fd = *(int *)arg;

    while (1) {

        client_socket_fd = malloc(sizeof(int));

        if (!client_socket_fd) {
            perror("malloc failed");
            pthread_exit(NULL);
        }

        /* Accept the data packet from client and verification */
        *client_socket_fd = accept(tcp_socket_fd, NULL, NULL); 
        if (*client_socket_fd < 0) { 
            printf("server accept failed...\n"); 
            free(client_socket_fd);
            continue;
        } else {
            printf("server accept the client...\n"); 
        }

        pthread_create(&client_thread, NULL, handle_client, client_socket_fd);
        pthread_detach(client_thread);
    }

    pthread_exit(NULL);
}

int main(int argc, char *arg[]){

    /* Check Command line arguments validity */
    if (argc != 3) {
        printf("Usage: ./proxy <server port> hostname \n");
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
    tcp_socket_fd = socket(servinfo->ai_family,
                    servinfo->ai_socktype,
                    servinfo->ai_protocol); 
    if (tcp_socket_fd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } else {
        printf("Socket successfully created..\n"); 
    }

    /* Bind socket */ 
    if (bind(tcp_socket_fd,
             servinfo->ai_addr,
             servinfo->ai_addrlen) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } else {
        printf("Socket successfully binded..\n"); 
    }
    
    freeaddrinfo(servinfo);

    /* Now server is ready to listen and verification */
    if ((listen(tcp_socket_fd, 10)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } else {
        printf("Proxy listening..\n"); 
    }

    /* Create a thread that just accepts new connections */
    /* Have all the accept logic be in the function of this thread */
     /* Create a new thread */
    if (pthread_create(&thread, NULL, accept_client, &tcp_socket_fd) != 0) {
        perror("pthread_create failed");
        return 1;
    }

   /* Do NOT join – accept thread runs forever */
    pthread_detach(thread);

    /* main can now do other work or sleep */
    pause();
  
    close(tcp_socket_fd);
    return 0;
}