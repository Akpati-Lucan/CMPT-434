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

#define SEQ_SPACE 8
typedef struct MSG{
	int seq_num;
    char msg[128];
}MSG;

/* Create an array of Messages that incoming messages will be buffered into 
    Max SEQ_SPACE */
MSG MSG_BUFFER[SEQ_SPACE];

/*  Create a bitmap of ACK's that will be referenced to 
    Know when a msg has been acknowledged */
int MSG_ACK[SEQ_SPACE] = {0};

struct addrinfo hints, *recvinfo;
struct sockaddr_in *ipv4, sender_info, receiver_info;
int sender_port, receiver_port, udp_socket, send_win_size, timeout;
char *hostname, receiver_port_str[16];
socklen_t recv_len = sizeof(receiver_info);
pthread_t sender_thread, receiver_thread;
int base_seq = 0, next_seq = 0;
pthread_mutex_t ack_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER;


void print_buffer(){
    int i;
    printf("Window Size - %d\n", send_win_size);
    printf("Base seq number - %d\n", base_seq);
    for (i = 0; i < 8; i++){
        printf("Sequence Number: %d Message: %s\n", MSG_BUFFER[i].seq_num, MSG_BUFFER[i].msg);
        printf("ACK BITMAP %d\n", MSG_ACK[i]);
    }
}

int in_send_window(int seq, int base, int win) {
    if (base + win < SEQ_SPACE)
        return (seq >= base && seq < base + win);
    else
        return (seq >= base || seq < (base + win) % SEQ_SPACE);
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

/*  Function that hadles retransmition 
    It creates a tread for each package 
    And thread takes in the sequense number as parameter
    Thread constantly checks if ACK has been received and 
    retransmits if not */
void *retransmit_func(void *arg) {

    struct MSG msg;
    int seq_num = *(int *)arg;
    free(arg);

    /* While message is still not Acknowledged */
    while(1) {
        /* Sleep for the Time out period */
        sleep(timeout);
        
        pthread_mutex_lock(&ack_mutex);
        if (MSG_ACK[seq_num] == 1) {
            MSG_ACK[seq_num] = 0;
            pthread_mutex_unlock(&ack_mutex);
            break;
        }
        pthread_mutex_unlock(&ack_mutex);

        /* Load message to be retransmitted with correct data*/
        msg.seq_num = seq_num;
        strcpy(msg.msg, MSG_BUFFER[seq_num].msg);
        /* Retransmit */
        send_msg(&msg, &receiver_info, recv_len);
    }
    pthread_exit(NULL);
}

void *send_thread_func() {
    char buff[1024];
    struct MSG msg;
    pthread_t retransmit_thread;
    int *seq_arg;

    while (1)
    {
        pthread_mutex_lock(&send_mutex);

        if (!in_send_window(next_seq, base_seq, send_win_size)) {
            pthread_mutex_unlock(&send_mutex);
            usleep(10000);
            continue;
        }

        printf("Enter Message: ");
        if (fgets(buff, sizeof(buff), stdin) == NULL) {
            pthread_mutex_unlock(&send_mutex);
            break;
        }

        msg.seq_num = next_seq;
        strcpy(msg.msg, buff);
        /* Copy message into buffer in case of retransmission */
        strcpy(MSG_BUFFER[msg.seq_num].msg, buff);
        MSG_BUFFER[msg.seq_num].seq_num = msg.seq_num;

        send_msg(&msg, &receiver_info, recv_len);

        seq_arg = malloc(sizeof(int));
        *seq_arg = msg.seq_num;
        /* Create a thread that just retransmits messages */
        if (pthread_create(&retransmit_thread, NULL, retransmit_func, seq_arg) != 0) {
            perror("pthread_create failed");
            pthread_exit(NULL);
        }
        pthread_detach(retransmit_thread);

        memset(buff, 0, sizeof(buff));
        next_seq = (next_seq + 1) % SEQ_SPACE;
        
        pthread_mutex_unlock(&send_mutex);
    }
    
    pthread_exit(NULL);
}
    

void *receive_thread_func() {
    struct MSG msg;
    while (1) {
        memset(msg.msg, 0, sizeof(msg.msg));
        if (receive(&msg, &receiver_info, &recv_len) > 0) {
            /* printf("Sequence Number: %d\nMessage: %s\n", msg.seq_num, msg.msg); */
            if (strcmp(msg.msg, "ACK") == 0){
                print_buffer();
                /* Set the ACK bit of the seq_num just received */
                pthread_mutex_lock(&ack_mutex);
                
                if (in_send_window(msg.seq_num, base_seq, send_win_size)){
                    MSG_ACK[msg.seq_num] = 1;
                    while (MSG_ACK[base_seq] == 1) {
                        MSG_ACK[base_seq] = 0;
                        base_seq = (base_seq + 1) % SEQ_SPACE;
                    }
                }
                pthread_mutex_unlock(&ack_mutex);
            }   
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
    timeout = atoi(arg[5]);
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
