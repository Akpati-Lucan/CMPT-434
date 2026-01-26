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
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>



int tcp_socket_fd, udp_socket_fd, status, proxy_port, server_port;
struct addrinfo hints_tcp, hints_udp, *servinfo_tcp, *servinfo_udp;
struct sockaddr_in *ipv4, proxy_info, server_info;
char proxy_port_str[16];
char server_port_str[16];
char *hostname;
pthread_t thread;


/* An array of strings that will hold the users input 
    A double pointer */
char input_str[8][100];

void send_data_to_server(int socket_fd, char *msg){
    int status;
    socklen_t to_len;

    to_len = sizeof(proxy_info);

    status = sendto(socket_fd, msg, sizeof(*msg), \
		0, (struct sockaddr *)&server_info, to_len);
        if (status == -1)
        {
            perror("Sender Failed to Send Message");
        }

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

void get_data_from_server(int udp_socket_fd, char str[][100]){

    char buff[1024];
    char *token;
    int n;
    int status;
    socklen_t from_len;
    char* response = malloc(4096);

    if (!response) return;  /* handle malloc failure */
    response[0] = '\0';

    while (1) { 
        n = 0;
        from_len = sizeof(server_info);

        status = recvfrom(udp_socket_fd, &buff, sizeof(buff), \
		0, (struct sockaddr *)&server_info, &from_len);

        if (status == -1) {
                perror("Receiver Failed to Receive Message");
        }


        /* Tokenize and add tokens to input str */ 
        token = strtok(buff, " \n");
        while (token != NULL) {
            if (n >= 5) {
                printf("Too many tokens\n");
                break;
            }
            if (strcmp(token, "CS") == 0){
                strcpy(token, "CMPT");
            }
            strcpy(str[n], token);
            n += 1;
            token = strtok(NULL, " \n");
            strncat(response, token, 4096 - strlen(response) - 1);
        }

        send_data_to_client(tcp_socket_fd, response);

        /* if msg contains "server exit" then server exit and chat ended. */
        if (strncmp(buff, "shutdown", 8) == 0) {
            printf("Server Exit...\n");
            break;
        }
    } 
}


void get_data_from_client(int client_socket_fd){

    char buff[1024];
    ssize_t nbytes;
    char str[8][100];

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
        
        /* Send to real server */
        send_data_to_server(udp_socket_fd, buff);

         if (strcmp(input_str[0], "getvalue") == 0 ||
            strcmp(input_str[0], "getall") == 0) {
            get_data_from_server(udp_socket_fd, str);
        }


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
    if (argc != 4) {
        printf("Usage: ./proxy <proxy port> hostname <server port> \n");
        exit(-1);
    }

    proxy_port = atoi(arg[1]);
    hostname = arg[2];
    server_port = atoi(arg[3]);

    if (proxy_port < 30000 || proxy_port > 40000) {
        fprintf(stderr, "Port must be between 30000 and 40000\n");
        exit(1);
    }

    if (server_port < 30000 || server_port > 40000) {
        fprintf(stderr, "Port must be between 30000 and 40000\n");
        exit(1);
    }

    
    sprintf(proxy_port_str, "%d", proxy_port);
    sprintf(server_port_str, "%d", server_port);

    /* Get IP address of proxy */
    memset(&hints_tcp, 0, sizeof hints_tcp);
    hints_tcp.ai_family = AF_UNSPEC;
    hints_tcp.ai_socktype = SOCK_STREAM;
    hints_tcp.ai_flags = AI_PASSIVE;


    /* Get IP address of server */
    memset(&hints_udp, 0, sizeof hints_udp);
    hints_udp.ai_family = AF_UNSPEC;
    hints_udp.ai_socktype = SOCK_DGRAM;
    hints_udp.ai_flags = AI_PASSIVE;

    
    if ((status = getaddrinfo(NULL, 
                                proxy_port_str, 
                                &hints_tcp, 
                                &servinfo_tcp)) != 0) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(status));
        exit(1);
    }

    if ((status = getaddrinfo(hostname, 
			                    server_port_str, 
			                    &hints_udp, 
                                &servinfo_udp)) != 0) {
	    fprintf(stderr, "gai error: %s\n", gai_strerror(status));
	    exit(1);
    } 

    ipv4 = (struct sockaddr_in *) servinfo_udp->ai_addr;


    /* TCP socket - client create and verification */
    tcp_socket_fd = socket(servinfo_tcp->ai_family,
                    servinfo_tcp->ai_socktype,
                    servinfo_tcp->ai_protocol); 
    if (tcp_socket_fd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } else {
        printf("Socket successfully created..\n"); 
    }

    /* Create Socket - UDP - Mainserver*/
    udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket_fd == -1) {
        perror("Socket creation failed!");
        exit(1);
    }

    /* Making Remote address Structure */
    memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(server_port);
    server_info.sin_addr = ipv4->sin_addr;

    /* Change file descriptor to NON-BLOCKING*/
    if (fcntl(udp_socket_fd, F_SETFL, SOCK_NONBLOCK) == -1) {
        perror("Could not make socket non-blocking!\n");
        exit(1);
    }
    if (fcntl(STDIN_FILENO, F_SETFL, SOCK_NONBLOCK) == -1)
    {
        perror("Could not make stdin non-blocking!");
        exit(1);
    }

    /* Bind socket to port*/
    if (bind(udp_socket_fd, (struct sockaddr *)&proxy_info, 
			sizeof(proxy_info)) == -1){
        perror("Socket could not be bound");
        exit(1);
    }

    /* Bind socket */ 
    if (bind(tcp_socket_fd,
             servinfo_tcp->ai_addr,
             servinfo_tcp->ai_addrlen) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } else {
        printf("Socket successfully binded..\n"); 
    }
    
    freeaddrinfo(servinfo_tcp);

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