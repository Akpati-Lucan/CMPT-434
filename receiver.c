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


struct addrinfo hints, *sendinfo;
struct sockaddr_in *ipv4, sender_info, receiver_info;
int sender_port, receiver_port, udp_socket;
char *hostname, sender_port_str[16];

void send(char *buff)
{
    int status;
    socklen_t to_len;
        
    to_len = sizeof(receiver_info);
    status = sendto(udp_socket, 
                    buff, 
                    sizeof(*buff),
                    0, 
                    (struct sockaddr *)&sender_info, 
                    to_len);
    if (status == -1) {
        perror("Sender Failed to Send Message");
    }
}


void receive(char *buff) 
{
    int status;
    socklen_t from_len;

    from_len = sizeof(receiver_info);
    status = recvfrom(  udp_socket, 
                        &buff, 
                        sizeof(*buff),
		                0, 
                        (struct sockaddr *)&sender_info, 
                        &from_len);
    if (status == -1){
        perror("Sender Failed to Send Message");
        return;
    }
}


int main(int argc, char *arg[])
{
    int status;
    /* Check Command line arguments validity */
    if (argc != 4) {
        printf("Usage: ./s-chat  <receiver port> <hostname> <sender port>\n");
        exit(-1);
    }

    receiver_port = atoi(arg[3]);
    hostname = arg[2];
    sender_port = atoi(arg[1]);
    sprintf(sender_port_str, "%d", sender_port);

    /* Collect command line arguments */
    if (receiver_port < 30001 || receiver_port > 40000 ||
        sender_port < 30001 || sender_port > 40000) {
        fprintf(stderr, "Port must be between 30000 and 40000\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (( status = getaddrinfo (hostname, 
			                    sender_port_str, 
			                    &hints, 
                                &sendinfo)) != 0) {
	    fprintf(stderr, "gai error: %s\n", gai_strerror(status));
	    exit(1);
    } 

    ipv4 = (struct sockaddr_in *) sendinfo->ai_addr;

    /* Create Socket*/
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket == -1) {
        perror("Socket creation failed!");
        exit(1);
    }

    /* Making local address structure */
    memset(&receiver_info, 0, sizeof(receiver_info));
    receiver_info.sin_family = AF_INET;
    receiver_info.sin_port = htons(receiver_port);
    receiver_info.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Making Remote address Structure */
    memset(&sender_info, 0, sizeof(sender_info));
    sender_info.sin_family = AF_INET;
    sender_info.sin_port = htons(sender_port);
    sender_info.sin_addr = ipv4->sin_addr;

    /* Bind socket to port*/
    if (bind(   udp_socket, 
                (struct sockaddr *)&receiver_info, 
			    sizeof(receiver_info)) == -1) {
        perror("Socket could not be bound");
        exit(1);
    }

    return 0;
}
