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


typedef struct MSG{
	int seq_num;
    char msg[1024];
}MSG;


/*  Create a bitmap of ACK's that will be referenced to 
    Know when a msg has been acknowledged */
int MSG_ACK[8];

struct addrinfo hints, *recvinfo;
struct sockaddr_in *ipv4, sender_info, receiver_info;
int sender_port, receiver_port, udp_socket, send_win_size;
char *hostname, receiver_port_str[16];
socklen_t recv_len = sizeof(receiver_info);
pthread_t sender_thread, receiver_thread;
int sequence_number = 0;

int seq_num_generator(){
    int temp_seq;
    temp_seq = sequence_number % 7;
    sequence_number += 1;
    return temp_seq;
}

void send_msg(MSG *msg, struct sockaddr_in *remote_addr, socklen_t addr_len) {

    int status = sendto(udp_socket,
                        msg,
                        sizeof(*msg),
                        0,
                        (struct sockaddr *)remote_addr,
                        addr_len);
    if (status == -1) {
        perror("Sender Failed to Send Message");
    }
}

int receive(MSG *msg, struct sockaddr_in *sender_addr, socklen_t *addr_len) {

    int nbytes = recvfrom(udp_socket,
                          msg,
                          sizeof(*msg),
                          0,
                          (struct sockaddr *)sender_addr,
                          addr_len);
    if (nbytes == -1) {
        perror("Receiver Failed to Receive Message");
        return -1;
    }
    /* Immediately an ack is received update the ack bit map*/
    return nbytes;
}

void *send_thread_func() {
    char buff[1024];
    struct MSG msg;
    int temp_seq;
    while (1)
    {   
        printf("Enter the string: ");
        if (fgets(buff, sizeof(buff), stdin) == NULL) break;
        
        temp_seq = seq_num_generator();
        msg.seq_num = temp_seq;
        strcpy(msg.msg, buff);
        send_msg(&msg, &receiver_info, recv_len);
    }
    pthread_exit(NULL);
}


void *receive_thread_func() {
    struct MSG msg;
    while (1) {
        memset(msg.msg, 0, sizeof(msg.msg));
        if (receive(&msg, &receiver_info, &recv_len) > 0) {
            printf("Sequence Number: %d\nMessage: %s\n", msg.seq_num, msg.msg);
        }
    }
    pthread_exit(NULL);
}
int main(int argc, char *arg[])
{
    int status;
    /* Check Command line arguments validity */
    if (argc != 6) {
        printf("Usage: ./sender <sender port> <hostname> <receiver port> \
            <sending window size> <timeout> \n");
        exit(-1);
    }

    sender_port = atoi(arg[1]);
    hostname = arg[2];
    receiver_port = atoi(arg[3]);
    send_win_size = atoi(arg[4]);
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

    ipv4 = (struct sockaddr_in *) recvinfo->ai_addr;

    /* Create Socket*/
    udp_socket = socket(recvinfo->ai_family,
                        recvinfo->ai_socktype,
                        recvinfo->ai_protocol);
    if (udp_socket == -1) {
        perror("Socket creation failed!");
        exit(1);
    }

    /* Making Sender address structure */
    memset(&sender_info, 0, sizeof(sender_info));
    sender_info.sin_family = AF_INET;
    sender_info.sin_port = htons(sender_port);
    sender_info.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Making Receiver address Structure */
    memset(&receiver_info, 0, sizeof(receiver_info));
    receiver_info.sin_family = AF_INET;
    receiver_info.sin_port = htons(receiver_port);
    receiver_info.sin_addr = ipv4->sin_addr;


    /* Bind socket to port*/
    if (bind(   udp_socket, 
                (struct sockaddr *)&sender_info, 
			    sizeof(sender_info)) == -1) {
        perror("Socket could not be bound");
        exit(1);
    }

    /* Create a thread that just sends messages */
    if (pthread_create(&sender_thread, NULL, send_thread_func, NULL) != 0) {
        perror("pthread_create failed");
        return 1;
    }

    /* Create a thread that just receives messages */
    if (pthread_create(&receiver_thread, NULL, receive_thread_func, NULL) != 0) {
        perror("pthread_create failed");
        return 1;
    }

    /* Do NOT join – accept thread runs forever */
    pthread_detach(sender_thread);

    /* main can now do other work or sleep */
    pause();

    freeaddrinfo(recvinfo);
    close(udp_socket);
    return 0;
}
