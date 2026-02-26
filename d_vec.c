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

int self_port_number;       /* Port Number For Router Other routers will connect to this port number */
int self_socket_fd;         /* Socket fd for other routers to connect to */
struct sockaddr_in server_addr;
int router_table_index = 0; /* Variable specifically for appending routers to the router table*/
pthread_t accept_neighbours_thread; /* Thread that accepts incoming neigbours */ 
pthread_t bind_listen_thread;       /* Thread that binds and listens to incoming neigbours */ 


/**/
/*
    Each process will have a routing table of all nodes,
    in the network this table will include
*/
struct router_info {
    int router_id;	    /* Identificator in the Router Table {Array} */ 
    int port_number;    /* Port Number of Router */ 
    int	cost;           /* cost to reach said router */ 
    int sock_fd;        /* Socket FIle descriptor of router */ 
} router_info;

/* Maximum Number of routers in the network */
struct router_info router_table[ 20 ];

pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;

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
void *accept_neighbours()
{
    int *connect_fd, i, neighbour_port;
    struct sockaddr_in *neighbour_addr;
    socklen_t neighbour_len = sizeof(neighbour_addr);
    while (1) {
        connect_fd = malloc(sizeof(int));
        neighbour_addr = malloc(sizeof(struct sockaddr_in));
        if (!connect_fd) {
            perror("malloc failed");
            pthread_exit(NULL);
        }
        /* Accept the data packet from neigbour and verification */
        *connect_fd = accept(self_socket_fd, 
                            (struct sockaddr *)&neighbour_addr,
                            &neighbour_len); 
        if (*connect_fd < 0) { 
            printf("server accept failed...\n"); 
            free(connect_fd);
            continue;
        } else {
            /* Extract client port */
            neighbour_port = ntohs(neighbour_addr.sin_port);
            printf("Connected with neighbour from port: %d\n", neighbour_port);
        }

        /*  Loop through router table and set the socket_fd of the 
            element with the same port number 
        */
        for( i = 0; i < 20; i++) {
            if (router_table[i].port_number == neighbour_port) {
                pthread_mutex_lock(&table_lock);
                router_table[i].sock_fd = connect_fd;
                pthread_mutex_unlock(&table_lock);
            }
        }
    }
    pthread_exit(NULL);
}



/* Bind and Listen to neighbour routers */
void *bind_listen_to_neighbours()
{
    /* socket create and verification */
    self_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (self_socket_fd < 0) { 
        printf("Socket creation failed...\n"); 
        pthread_exit(NULL);
    } else {
        printf("Socket successfully created..\n"); 
    }

    /* Bind socket */ 
    if (bind(self_socket_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)) != 0) { 
        printf("Socket bind failed...\n"); 
        pthread_exit(NULL);
    } else {
        printf("Socket successfully binded..\n"); 
    }

    /* Now server is ready to listen and verification */
    if ((listen(self_socket_fd, 20)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } else {
        printf("Server listening..\n"); 
    }

    /* Create a thread that just accepts new connections */
    /* Have all the accept logic be in the function of this thread */
    if (pthread_create(&accept_neighbours_thread, NULL, accept_neighbours NULL) != 0) {
        perror("pthread_create failed");
        return 1;
    }

    pthread_detach(accept_neighbours_thread);
    pthread_exit(NULL);
}


/* Connect to neighbour routers */
void *connect_to_neighbours(void *arg)
{
    int neighbour_socket_fd, neighbour_port_number, i;
    neighbour_port_number = *(int *)arg;

    /* client socket create and verification */
    neighbour_socket_fd = socket(server_addr->ai_family,
                                server_addr->ai_socktype,
                                server_addr->ai_protocol); 
    if (neighbour_socket_fd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } else {
        printf("Socket successfully created..\n"); 
    }

    /* connect the client socket to server socket */
    if (connect(neighbour_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))){
        printf("connection with the server failed...\n");
        exit(0);
    } else {
        printf("connected to the server..\n");
    }

    /*  Loop through router table and set the socket_fd of the 
        element with the same port number 
    */
    for( i = 0; i < 20; i++) {
        if (router_table[i].port_number == neighbour_port_number) {
            pthread_mutex_lock(&table_lock);
            router_table[i].sock_fd = neighbour_socket_fd;
            pthread_mutex_unlock(&table_lock);
        }
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

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    /*
        Every other CLI argument should be put in the router table in Pairs
        First argument for id and port number and second argument for weight
    */  

    i = 2;
    while (i < argc) {
        /* Use atoi() to turn CLI argument to integers */
        router_table[router_table_index].router_id = router_table_index;
        router_table[router_table_index].port_number = atoi(arg[i]);
        router_table[router_table_index].cost = atoi(arg[i++]);
        i += 2;

        /* Set Up a TCP socket and listen for connections
            for any router that has lower port number than sel_port_number */

        /* Set up a TCP socket and connect to any router that their
            port number is bigger than self port number */
        
    }

    /* Initialize the dictionary */
    init_dict();


    /* Create a thread that just accepts new connections */
    /* Have all the accept logic be in the function of this thread */
     /* Create a new thread */
    if (pthread_create(&bind_listen_thread, NULL, bind_listen_to_neighbours, NULL) != 0) {
        perror("pthread_create failed");
        return 1;
    }

   /* Do NOT join – accept thread runs forever */
    pthread_detach(bind_listen_thread);

    /* main can now do other work or sleep */
    pause();
  
    close(self_socket_fd);
    return 0;
}