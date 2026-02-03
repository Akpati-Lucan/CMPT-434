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

/* Create an array of Messages that incoming messages will be buffered into 
    Max 8 */
MSG MSG_BUFFER[8];

struct addrinfo hints, *sendinfo;
struct sockaddr_in *ipv4, receiver_info, sender_info;
int sender_port, receiver_port, udp_socket;
char *hostname, sender_port_str[16];
socklen_t sender_len = sizeof(sender_info);
pthread_t sender_thread, receiver_thread;

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

/* When an Incoming message is received it buffers it in the 
    index of the msg_buffer array */
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
    strcpy(MSG_BUFFER[msg->seq_num].msg, msg->msg);
    return nbytes;
}

void *send_thread_func() {
    char buff[1024];
    struct MSG msg;
    while (1)
    {   
        printf("Enter the string: ");
        if (fgets(buff, sizeof(buff), stdin) == NULL) break;

        msg.seq_num = 5;
        strcpy(msg.msg, buff);
        send_msg(&msg, &sender_info, sender_len);
    }
    pthread_exit(NULL);
}


void *receive_thread_func() {
    struct MSG msg;
    while (1) {
        memset(msg.msg, 0, sizeof(msg.msg));
        if (receive(&msg, &receiver_info, &sender_len) > 0) {
            printf("Sequence Number: %d\nMessage: %s\n", msg.seq_num, msg.msg);
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *arg[])
{
    int status;
    /* Check Command line arguments validity */
    if (argc != 5) {
        printf("Usage: ./receiver  <receiver port> <hostname> <sender port> \
                <receiveing window size> \n");
        exit(-1);
    }

    receiver_port = atoi(arg[1]);
    hostname = arg[2];
    sender_port = atoi(arg[3]);
    sprintf(sender_port_str, "%d", sender_port);

    /* Collect command line arguments */
    if (receiver_port < 30001 || receiver_port > 40000 ||
        sender_port < 30001 || sender_port > 40000) {
        fprintf(stderr, "Port must be between 30000 and 40000\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;

    if (( status = getaddrinfo (hostname, 
			                    sender_port_str, 
			                    &hints, 
                                &sendinfo)) != 0) {
	    fprintf(stderr, "gai error: %s\n", gai_strerror(status));
	    exit(1);
    } 

    ipv4 = (struct sockaddr_in *) sendinfo->ai_addr;

    /* Create Socket*/
    udp_socket = socket(sendinfo->ai_family,
                        sendinfo->ai_socktype,
                        sendinfo->ai_protocol);
    if (udp_socket == -1) {
        perror("Socket creation failed!");
        exit(1);
    }

    /* Making Sender address structure */
    memset(&receiver_info, 0, sizeof(receiver_info));
    receiver_info.sin_family = AF_INET;
    receiver_info.sin_port = htons(receiver_port);
    receiver_info.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Making Receiver address Structure */
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

    freeaddrinfo(sendinfo);
    close(udp_socket);
    return 0;
}
