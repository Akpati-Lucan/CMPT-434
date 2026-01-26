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


int udp_socket_fd, status, server_port, proxy_port;
struct addrinfo hints_udp, *servinfo_udp;
struct sockaddr_in *ipv4, proxy_info, server_info;
char server_port_str[16];
char proxy_port_str[16];
char *hostname;
pthread_t thread;

/* Create a struct for key-value pairs */
typedef struct k_val{
    int key;
    char value[100];
}k_val;

/* Create an array of Key-value structs called Dict */
struct k_val dict[100];

void print_dict() {
    int i;
    for (i = 0; i < 100; i++) {
         /* Only print entries that are in use */
        if (dict[i].value[0] != '\0') {
            printf("Key: %d, Value: %s\n", dict[i].key, dict[i].value);
        }
    }
}

void add_key(int key, char val[]){

    /* Validate key */
    if (key < 0 || key >= 100) {
        printf("Key must be between 0 and 99\n");
        return;
    }
    /* If the value of a certain key is empty you can add it
        else throw a error */
    if (dict[key].value[0] == '\0') {
        strncpy(dict[key].value, val, sizeof(dict[key].value) - 1);
        dict[key].value[sizeof(dict[key].value) - 1] = '\0';
        dict[key].key = key;
    } else {
        printf("Key already exists\n");
    }
}

char* get_value(int key){
    /* If the value of a certain key is empty return;
        else return the correct value */
    /* Validate key */
    if (key < 0 || key >= 100) {
        printf("Key must be between 0 and 99\n");
        return NULL;
    }

    /* Check if key exists */
    if (dict[key].value[0] == '\0') {
        return NULL;
    }

    return dict[key].value;
}


void send_data_to_client(int udp_socket_fd, char *msg) {

    int status;
    socklen_t to_len;

    to_len = sizeof(proxy_info);

    status = sendto(udp_socket_fd, msg, sizeof(*msg), \
		0, (struct sockaddr *)&proxy_info, to_len);
        if (status == -1)
        {
            perror("Sender Failed to Send Message");
        }

}
void get_all() {
    /* Allocate a large enough buffer */
    char* response = malloc(4096);
    int i;
    if (!response) return;  /* handle malloc failure */
    response[0] = '\0';          /* start with empty string */

    for (i = 0; i < 100; i++) {
        char line[128];
        if (strcmp(dict[i].value, "") == 0) {
            continue;
        }
        /* append new entry to the response string */
        snprintf(line, sizeof(line), "Key: %d, Value: %s\n", dict[i].key, dict[i].value);
        send_data_to_client(udp_socket_fd, line);
    }
}


void remove_key(int key){
    /* Validate key */
    if (key < 0 || key >= 100) {
        printf("Key must be between 0 and 99\n");
        return;
    }
    /* If the value of a certain key is empty you can
        else throw a error else remove it */
    /* Check if key exists */
    if (strcmp(dict[key].value, "") == 0) {
        printf("Key-value pair does not exist\n");
    } else {
        /* Remove value */
        strcpy(dict[key].value, "");
    }
}

/* An array of strings that will hold the users input 
    A double pointer */
char input_str[8][100];


int execute_command(int connect_fd, char str[][100]){
    int key;
    char *test_val;
    char response[4096];
    /*  check if str[0] is in [add, getvalue, getall, remove] */
    if (strcmp(str[0], "add") == 0) {
        key = atoi(str[1]);
        add_key(key, str[2]);
    } else if (strcmp(str[0], "getvalue") == 0) {
        key = atoi(str[1]);
        test_val = get_value(key);
        if (test_val == NULL || test_val[0] == '\0') {
           send_data_to_client(connect_fd, "No values found.\n");
        } else {
        snprintf(response, sizeof(response),
            "get_val for Key: %d, Value: %s\n", key, test_val);
        send_data_to_client(connect_fd, response);
        }
    } else if (strcmp(str[0], "getall") == 0) {
        get_all();
    } else if (strcmp(str[0], "remove") == 0) {
        int key = atoi(str[1]);
        remove_key(key);
    } else {
        printf("Unknown command\n");
    }
    return 0;
}


void get_data_from_client(int udp_socket_fd, char str[][100]){

    char buff[1024];
    char *token;
    int n;
    int status;
    socklen_t from_len;

    while (1) { 
        n = 0;
        from_len = sizeof(proxy_info);

        status = recvfrom(udp_socket_fd, &buff, sizeof(buff), \
		0, (struct sockaddr *)&proxy_info, &from_len);

        
        if (status > 0) {
            buff[status] = '\0';
        }

        if (status == -1) {
            continue;
        }



        /* Tokenize and add tokens to input str */ 
        token = strtok(buff, " \n");
        while (token != NULL) {
            if (n >= 5) {
                printf("Too many tokens\n");
                break;
            }
            strcpy(str[n], token);
            n += 1;
            token = strtok(NULL, " \n");
        }

        status = execute_command(udp_socket_fd, str);

        if (status != 0){
            printf("Wrong Input\n");
            break;
        }
        /* if msg contains "server exit" then server exit and chat ended. */
        if (strncmp(buff, "shutdown", 8) == 0) {
            printf("Server Exit...\n");
            break;
        }
    } 
}


/* Accept incoming connections */
void *accept_proxy()
{
    get_data_from_client(udp_socket_fd, input_str);

    pthread_exit(NULL);
}

void init_dict() {
    int i;
    for (i = 0; i < 100; i++) {
        dict[i].value[0] = '\0';
    }
}

int main(int argc, char *arg[]){

    /* Check Command line arguments validity */
    if (argc != 4) {
        printf("Usage: ./server_with_proxy <server port> hostname <proxy port> \n");
        exit(-1);
    }

    server_port = atoi(arg[1]);
    hostname = arg[2];
    proxy_port = atoi(arg[3]);

    if (server_port < 30000 || server_port > 40000) {
        fprintf(stderr, "Port must be between 30000 and 40000\n");
        exit(1);
    }

    if (proxy_port < 30000 || proxy_port > 40000) {
        fprintf(stderr, "Port must be between 30000 and 40000\n");
        exit(1);
    }

    
    sprintf(server_port_str, "%d", server_port);
    sprintf(proxy_port_str, "%d", proxy_port);

    /* Get IP address of server */
    memset(&hints_udp, 0, sizeof hints_udp);
    hints_udp.ai_family = AF_UNSPEC;
    hints_udp.ai_socktype = SOCK_DGRAM;
    hints_udp.ai_flags = AI_PASSIVE;



    if ((status = getaddrinfo(hostname, 
			                    server_port_str, 
			                    &hints_udp, 
                                &servinfo_udp)) != 0) {
	    fprintf(stderr, "gai error: %s\n", gai_strerror(status));
	    exit(1);
    } 

    ipv4 = (struct sockaddr_in *) servinfo_udp->ai_addr;

   
    /* Create Socket - UDP - Mainserver*/
    udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket_fd == -1) {
        perror("Socket creation failed!");
        exit(1);
    }

    /* Making Remote address Structure */
    memset(&proxy_info, 0, sizeof(proxy_info));
    proxy_info.sin_family = AF_INET;
    proxy_info.sin_port = htons(server_port);
    proxy_info.sin_addr = ipv4->sin_addr;

    /* Change file descriptor to NON-BLOCKING*/
    if (fcntl(udp_socket_fd, F_SETFL, O_NONBLOCK) == -1) {
        perror("Could not make socket non-blocking!\n");
        exit(1);
    }
    if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK) == -1)
    {
        perror("Could not make stdin non-blocking!");
        exit(1);
    }

    /* Bind socket to port*/
    if (bind(udp_socket_fd, (struct sockaddr *)&server_info, 
			sizeof(server_info)) == -1){
        perror("Socket could not be bound");
        exit(1);
    }

    /* Initialize the dictionary */
    init_dict();

    /* Create a new thread */
    if (pthread_create(&thread, NULL, accept_proxy, NULL) != 0) {
        perror("pthread_create failed");
        return 1;
    }

   /* Do NOT join – accept thread runs forever */
    pthread_detach(thread);

    /* main can now do other work or sleep */
    pause();
  
    close(udp_socket_fd);
    return 0;
}