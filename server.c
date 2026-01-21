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

/* Create a struct for key-value pairs */
typedef struct k_val{
    int key;
    char value[100];
}k_val;


/* Create an array of Key-value structs called Dict */
struct k_val dict[100];



void get_data_from_client(int connect_fd){

    char buff[1024];
    ssize_t nbytes;

    for (;;) { 
        
        memset(buff, 0, sizeof(buff));
  
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

        printf("From client: %s\n", buff);

        /* if msg contains "server exit" then server exit and chat ended. */
        if (strncmp(buff, "server exit", 11) == 0) {
            printf("Server Exit...\n");
            break;
        }
    } 
}

void *handle_client(void *arg)
{
    int connect_fd = *(int *)arg;
    free(arg);

    get_data_from_client(connect_fd);
    close(connect_fd);

    pthread_exit(NULL);
}

/* Accept incoming connections */
void *accept_client(void *arg)
{
    int *connect_fd;
    int socket_fd = *(int *)arg;
    pthread_t client_thread;

    printf("Hello from thread %d\n", socket_fd);

    while (1) {

        connect_fd = malloc(sizeof(int));

        if (!connect_fd) {
            perror("malloc failed");
            pthread_exit(NULL);
        }

        /* Accept the data packet from client and verification */
        *connect_fd = accept(socket_fd, NULL, NULL); 
        if (*connect_fd < 0) { 
            printf("server accept failed...\n"); 
            free(connect_fd);
            continue;
        } else {
            printf("server accept the client...\n"); 
        }

        pthread_create(&client_thread, NULL, handle_client, connect_fd);
        pthread_detach(client_thread);
    }

    pthread_exit(NULL);
}



int main(int argc, char *arg[]){

    int socket_fd, status, server_port;
    struct addrinfo hints, *servinfo;
    char server_port_str[16];
    pthread_t thread;

    /* Check Command line arguments validity */
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


    /* Create a thread that just accepts new connections */
    /* Have all the accept logic be in the function of this thread */
     /* Create a new thread */
    if (pthread_create(&thread, NULL, accept_client, &socket_fd) != 0) {
        perror("pthread_create failed");
        return 1;
    }

   /* Do NOT join – accept thread runs forever */
    pthread_detach(thread);

    /* main can now do other work or sleep */
    pause();
  
    close(socket_fd);
    return 0;
}