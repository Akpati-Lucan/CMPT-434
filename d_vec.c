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

int self_port_number;               /* Port Number For Router Other routers will connect to this port number */
int self_socket_fd;                 /* Socket fd for other routers to connect to */
struct sockaddr_in server_addr;     /* Socket Address for port connections */
int router_table_index = 0;         /* Variable specifically for appending routers to the router table*/
pthread_t accept_neighbours_thread; /* Thread that accepts incoming neigbours */ 
pthread_t bind_listen_thread;       /* Thread that binds and listens to incoming neigbours */ 
pthread_t connect_thread;           /* Thread that connect to incoming neigbours */ 

/*
    Each process will have a routing table of all nodes,
    in the network this table will include
*/
typedef struct {
    int port_number;    /* Port Number of Router */ 
    int	cost;           /* cost to reach said router */ 
    int sock_fd;        /* Socket FIle descriptor of router */ 
} router_info;

/* Maximum Number of routers in the network */
router_info router_table[ 20 ];

pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t table_index_lock = PTHREAD_MUTEX_INITIALIZER;

#define INF 9999

/* Function for Initializing the Router Table */
void init_table() {
    int i;
    for (i = 0; i < 20; i++) {
        router_table[i].port_number = 0;
        router_table[i].cost = INF;
        router_table[i].sock_fd = -1;
    }
}

void print_router_table()
{
    int i;
    printf("\n===== ROUTER TABLE =====\n");
    printf("ID\tPort\tCost\tSocketFD\n");

    for (i = 0; i < 20; i++) {

        /* Skip empty entries 
        if (router_table[i].port_number == 0)
        continue;
        */
        printf("%d\t%d\t%d\n",
               router_table[i].port_number,
               router_table[i].cost,
               router_table[i].sock_fd);
    }
    printf("========================\n\n");
}

/*  
    Function that Continually Prints the details, 
    of the router Table every two seconds
*/
void *print_router_table()
{
    while (1)
    {
        pthread_mutex_lock(&table_lock);
        print_router_table();
        pthread_mutex_unlock(&table_lock);
        sleep(2);
    }
    pthread_exit(NULL);
}

void add_to_router_table(int port_number, int cost) {
    int i;
    pthread_mutex_lock(&table_lock);
    int added = 0;
    for (i = 0; i < 20; i++) {
        if (router_table[i].sock_fd == -1) { /* empty slot */
            router_table[i].port_number = port_number;
            router_table[i].cost = cost;
            added = 1;
            break;
        }
    }
    pthread_mutex_unlock(&table_lock);

    if (!added) {
        printf("Router table full! Cannot add new router.\n");
    }
}

/* Accept incoming connections */
void *accept_neighbours()
{
    int connect_fd, i, neighbour_port;
    struct sockaddr_in neighbour_addr;
    socklen_t neighbour_len;
    while (1) {
        neighbour_len = sizeof(neighbour_addr);
        
        /* Accept the data packet from neigbour and verification */
        connect_fd = accept(self_socket_fd, 
                            (struct sockaddr *)&neighbour_addr,
                            &neighbour_len);
        if (connect_fd < 0) { 
            printf("Neighbour accept failed...\n"); 
            free(connect_fd);
            continue;
        }

        /* Extract client port */
        neighbour_port = ntohs(neighbour_addr.sin_port);
        printf("Connected with neighbour from port: %d\n", neighbour_port);

        /*  
            Loop through router table and set the socket_fd,
            of the element with the same port number 
        */
        pthread_mutex_lock(&table_lock);
        for( i = 0; i < 20; i++) {
            if (router_table[i].port_number == neighbour_port) {
                router_table[i].sock_fd = connect_fd;
            }
        }
        pthread_mutex_unlock(&table_lock);
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
    if (pthread_create(&accept_neighbours_thread, NULL, accept_neighbours, NULL) != 0) {
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

    struct sockaddr_in neighbour_addr;
    memset(&neighbour_addr, 0, sizeof(neighbour_addr));

    neighbour_addr.sin_family = AF_INET;
    neighbour_addr.sin_port = htons(neighbour_port_number);
    inet_pton(AF_INET, "127.0.0.1", &neighbour_addr.sin_addr);

    /* client socket create and verification */
     neighbour_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (neighbour_socket_fd < 0) { 
        printf("Socket creation failed...\n"); 
        pthread_exit(NULL);
    } else {
        printf("Socket successfully created..\n"); 
    }

    /* connect the client socket to server socket */
    if (connect(neighbour_socket_fd, (struct sockaddr *)&neighbour_addr, sizeof(neighbour_addr)) < 0){
        printf("Connected to neighbour on port %d Failed \n", neighbour_port_number);
        close(neighbour_socket_fd);
        pthread_exit(NULL);
    }

    printf("Connected to neighbour on port %d\n", neighbour_port_number);

    /*  
        Loop through router table and set the socket_fd of the 
        element with the same port number 
    */
    pthread_mutex_lock(&table_lock);
    for( i = 0; i < 20; i++) {
        if (router_table[i].port_number == neighbour_port_number) {
            router_table[i].sock_fd = neighbour_socket_fd;
            break;
        }
    }
    pthread_mutex_unlock(&table_lock);
    pthread_exit(NULL);
}

/**/
int main(int argc, char *arg[]){

    int i, router_port, router_cost;
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


    
    /* Initialize the Router Table */
    pthread_mutex_lock(&table_lock);
    init_table();
    pthread_mutex_unlock(&table_lock);


    /*
        Every other CLI argument should be put in the router table in Pairs
        First argument for id and port number and second argument for weight
    */  
    i = 2;
    while (i < argc) {
        /* Use atoi() to turn CLI argument to integers */
        router_port = atoi(arg[i]);
        router_cost = atoi(arg[i++]);
        
        add_to_router_table(router_port, router_cost);
        i += 2;

        /*  
            Set up a TCP socket and connect to any router that their
            port number is bigger than self port number 
        */
        if ( router_port > self_port_number ) {
            if (pthread_create(&connect_thread, NULL, connect_to_neighbours, router_port) != 0) {
                perror("pthread_create failed");
                return 1;
            }
        }
        pthread_mutex_lock(&table_index_lock);
        router_table_index += 1;
        pthread_mutex_unlock(&table_index_lock);
    }

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