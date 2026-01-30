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


struct addrinfo hints, *recvinfo;
struct sockaddr_in sender_info;
int sender_port, receiver_port, udp_socket;
char *hostname, receiver_port_str[16];

void send(char *buff)
{
    int status;
    status = sendto(udp_socket, 
                    buff, 
                    sizeof(*buff),
                    0, 
                    recvinfo->ai_addr,
                    recvinfo->ai_addrlen);
    if (status == -1) {
        perror("Sender Failed to Send Message");
    }
}

void receive(char *buff, int buff_size) 
{
    int status;
    status = recvfrom(  udp_socket, 
                        &buff, 
                        buff_size,
		                0, 
                        recvinfo->ai_addr,
                        recvinfo->ai_addrlen);
    if (status == -1){
        perror("Sender Failed to Send Message");
        return;
    }
}


int main(int argc, char *arg[])
{
    
    char buff[1024];
    int status;
    /* Check Command line arguments validity */
    if (argc != 4) {
        printf("Usage: ./s-chat <sender port> <hostname> <receiver port>\n");
        exit(-1);
    }

    sender_port = atoi(arg[1]);
    hostname = arg[2];
    receiver_port = atoi(arg[3]);
    sprintf(receiver_port_str, "%d", receiver_port);

    /* Collect command line arguments */
    if (sender_port < 30001 || sender_port > 40000 || 
        receiver_port < 30001 || receiver_port > 40000) {
        fprintf(stderr, "Port must be between 30000 and 40000\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;

    if (( status = getaddrinfo (hostname, 
			                    receiver_port_str, 
			                    &hints, 
                                &recvinfo)) != 0) {
	    fprintf(stderr, "gai error: %s\n", gai_strerror(status));
	    exit(1);
    }

    /* Create Socket*/
    udp_socket = socket(recvinfo->ai_family,
                        recvinfo->ai_socktype,
                        recvinfo->ai_protocol);
    if (udp_socket == -1) {
        perror("Socket creation failed!");
        exit(1);
    }

    /* Making local address structure */
    memset(&sender_info, 0, sizeof(sender_info));
    sender_info.sin_family = AF_INET;
    sender_info.sin_port = htons(sender_port);
    sender_info.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Bind socket to port*/
    if (bind(   udp_socket, 
                (struct sockaddr *)&sender_info, 
			    sizeof(sender_info)) == -1) {
        perror("Socket could not be bound");
        exit(1);
    }

    while (1)
    {
    printf("Enter the string: ");
    if (fgets(buff, sizeof(buff), stdin) == NULL) break;

    send(buff);

    printf("Sender received \n");
    memset(buff, 0, sizeof(buff));
    receive(buff, sizeof(buff));
    }

    freeaddrinfo(recvinfo);
    close(udp_socket);
    return 0;
}
