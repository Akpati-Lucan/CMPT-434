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

#define TCP_PORT 443

/* Create a struct for key-value pairs */
typedef struct k_val{
    int key;
    char value[100];
}k_val;


/* Create an array of Key-value structs called Dict */
struct k_val dict[100];


int main(){

    int socket_fd, connect_fd, len;
    struct sockaddr_in servaddr, cli;

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
    
    return 0;
}