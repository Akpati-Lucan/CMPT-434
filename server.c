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


int socket_fd, status, server_port;
struct addrinfo hints, *servinfo;
char server_port_str[16];
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
char* get_all() {
    /* Allocate a large enough buffer */
    char* response = malloc(4096);
    int i;
    if (!response) return NULL;  /* handle malloc failure */
    response[0] = '\0';          /* start with empty string */

    for (i = 0; i < 100; i++) {
        char line[128];
        if (strcmp(dict[i].value, "") == 0) {
            continue;
        }
        /* append new entry to the response string */
        snprintf(line, sizeof(line), "Key: %d, Value: %s\n", dict[i].key, dict[i].value);
        strncat(response, line, 4096 - strlen(response) - 1);
    }

    return response;  /* caller must free() this */
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

void send_data_to_client(int socket_fd, char *msg) {
    ssize_t total = 0;
    ssize_t len = strlen(msg);

    while (total < len) {
        ssize_t n = write(socket_fd, msg + total, len - total);
        if (n < 0) {
            perror("write");
            return;
        }
        total += n;
    }
}



int validate_str(int connect_fd, char str[][100]){
    int key;
    char *test_val;
    char* all_values;
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
        all_values = get_all();
        if (all_values == NULL || all_values[0] == '\0') {
            send_data_to_client(connect_fd, "No values found.\n");
        } else { 
            send_data_to_client(connect_fd, all_values); 
        }   
    } else if (strcmp(str[0], "remove") == 0) {
        int key = atoi(str[1]);
        remove_key(key);
    } else {
        printf("Unknown command\n");
    }
    return 0;
}


void get_data_from_client(int connect_fd, char str[][100]){

    char buff[1024];
    ssize_t nbytes;
    char *token;
    int n;
    int status;

    while (1) { 
        
        memset(buff, 0, sizeof(buff));
        n = 0;
  
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

        /* Tokenize and add tokens to input str */ 
        token = strtok(buff, " \n");
        while (token != NULL) {
            if (n >= 5) {
                printf("Too many tokens\n");
                break;
            }
            strcpy(input_str[n], token);
            n += 1;
            token = strtok(NULL, " \n");
        }

        status = validate_str(connect_fd, str);

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

void *handle_client(void *arg)
{
    int connect_fd = *(int *)arg;
    free(arg);
        
    get_data_from_client(connect_fd, input_str);
    close(connect_fd);
    
    
    close(connect_fd);

    pthread_exit(NULL);
}

/* Accept incoming connections */
void *accept_client(void *arg)
{
    int *connect_fd;
    pthread_t client_thread;
    socket_fd = *(int *)arg;

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

void init_dict() {
    int i;
    for (i = 0; i < 100; i++) {
        dict[i].value[0] = '\0';
    }
}

int main(int argc, char *arg[]){

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

    /* Initialize the dictionary */
    init_dict();


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