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

#define TCP_PORT 8443

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

int main(){

    int socket_fd, connect_fd;
    socklen_t len;
    struct sockaddr_in servaddr, clientaddr;

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


    /* Bind socket */ 
    if ((bind(socket_fd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } else {
        printf("Socket successfully binded..\n"); 
    }

    /* Now server is ready to listen and verification */
    if ((listen(socket_fd, 10)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } else {
        printf("Server listening..\n"); 
    }

    len = sizeof(clientaddr); 
  
    /* Accept the data packet from client and verification */
    connect_fd = accept(socket_fd, (struct sockaddr*)&clientaddr, &len); 
    if (connect_fd < 0) { 
        printf("server accept failed...\n"); 
        exit(0); 
    } else {
        printf("server accept the client...\n"); 
    }
  
    get_data_from_client(connect_fd);
    close(socket_fd);
    return 0;
}