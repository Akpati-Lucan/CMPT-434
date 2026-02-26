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

int self_port_number;       /* Port Number For Router 
                                Other routers will connect to this port number */
int router_table_index = 0; /* Variable specifically for appending routers to the router table*/
pthread_t thread;

/**/
/*
    Each process will have a routing table of all nodes,
    in the network this table will include
*/
typedef struct router_info {
    int router_id;	    /* Identificator in the Router Table {Array} */ 
    int port_number;    /* Port Number of Router */ 
    int	cost;           /* cost to reach said router */ 
    int sock_fd;        /* Socket FIle descriptor of router */ 
} router_info;

/* Maximum Number of routers in the network */
struct routing_info router_table[ 20 ];

/*  
    Function that Continually Prints the details, 
    of the router Table every two seconds
*/
void *print_router_table()
{
    while (1)
    {
        /* Print the rows in the router table that have valid values */
        printf("Print Router Table Working\n");
        sleep(2);
    }
    pthread_exit(NULL);
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

/**/
int main(int argc, char *arg[]){

    int i;
    /* Check Command line arguments validity */
    if ((argc % 2) == 0 ) {
        printf("Usage: ./d_vec <self_port_number> [ <port number> <cost> ] \n");
        exit(-1);
    }

    self_port_number = atoi(arg[1]);
    if (self_port_number < 30000 || self_port_number > 40000) {
        fprintf(stderr, "Port must be between 30000 and 40000\n");
        exit(1);
    }

    /*
        Every other CLI argument should be put in the router table in Pairs
        First argument for id and port number and second argument for weight
    */  

    i = 2;
    while (i < argc) {
        /* Use atoi() to turn CLI argument to integers */
        router_table[router_table_index].router_id = router_table_index;
        router_table[router_table_index].port_number = atoi(arg[i]);
        router_table[router_table_index].weight = atoi([i++]);
        i += 2;

        /* Set Up a TCP Connection between Self and router */

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

        router_table[router_table_index].sock_fd = socket_fd;
        /* Bind socket */ 
        if (bind(socket_fd,
                servinfo->ai_addr,
                servinfo->ai_addrlen) != 0) { 
            printf("socket bind failed...\n"); 
            exit(0); 
        } else {
            printf("Socket successfully binded..\n"); 
        }

        /* Now server is ready to listen and verification */
        if ((listen(socket_fd, 20)) != 0) { 
            printf("Listen failed...\n"); 
            exit(0); 
        } else {
            printf("Server listening..\n"); 
        }
    
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