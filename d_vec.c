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
#include <arpa/inet.h>
#include <stdbool.h>

int self_port_number;               /* Port Number For Router Other routers will connect to this port number */
int self_socket_fd;                 /* Socket fd for other routers to connect to */
struct sockaddr_in server_addr;     /* Socket Address for port connections */
pthread_t accept_neighbours_thread; /* Thread that accepts incoming neigbours */ 
pthread_t bind_listen_thread;       /* Thread that binds and listens to incoming neigbours */ 
pthread_t connect_thread;           /* Thread that connect to incoming neigbours */ 
pthread_t print_thread;             /* Thread that print router table values */ 
pthread_t sender_thread;            /* Thread that send info to neigbours */ 
pthread_t receiver_thread;          /* Thread that receives info from neighbour */ 


/*
    Each process will have a routing table of all nodes,
    in the network this table will include
*/
typedef struct {
    int port_number;    /* Port Number of Router */ 
    int	distance;           /* distance to reach neighbour said router */ 
    int sock_fd;        /* Socket FIle descriptor of router */ 
    bool is_neighbour;   /* Is This router a direct link */ 
    int next_hop;       /* neighbouring router of self to which packets 
                        with destination port_number will be forwarded  */
} router_info;

/* Maximum Number of routers in the network */
router_info router_table[ 20 ];

pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct MSG {
    int sender_id;        /* Router sending this message */
    int destination_id;   /* Destination router this entry refers to */
    int distance;         /* Sender's distance to that destination */
} MSG;

#define INF 9999

/*
 * Looks up a router in the routing table by port number
 *
 * Returns:
 *   index (0–19) if found
 *   -1 if not found
 */
int find_router_index(int port_number)
{
    int i;
    for (i = 0; i < 20; i++) {
        /* Skip empty entries */
        if (router_table[i].port_number == 0)
            continue;

        if (router_table[i].port_number == port_number) {
            return i;
        }
    }
    /* Router not found */
    return -1;
}

/* Function for Initializing the Router Table */
void init_table() {
    int i;
    for (i = 0; i < 20; i++) {
        router_table[i].port_number = 0;
        router_table[i].distance = INF;
        router_table[i].sock_fd = -1;
        router_table[i].is_neighbour = false;
        router_table[i].next_hop = -1;
    }
}

/*
 * Prints the current router table without SocketFD
 */
void print_router_table()
{
    int i;

    printf("\n================ ROUTER TABLE ================\n");
    printf("%-8s %-10s %-14s %-8s\n",
           "Port", "Distance", "IsNeighbour", "NextHop");
    printf("------------------------------------------------\n");

    for (i = 0; i < 20; i++) {

        /* Skip empty entries */
        if (router_table[i].port_number == 0)
            continue;

        /* Skip Your entry */
        if (router_table[i].port_number == self_port_number)
            continue;

        printf("%-8d %-10d %-14s ",
               router_table[i].port_number,
               router_table[i].distance,
               router_table[i].is_neighbour ? "true" : "false");

        /* Print next_hop or "-" if invalid */
        if (router_table[i].next_hop == -1)
            printf("%-8s\n", "-");
        else
            printf("%-8d\n", router_table[i].next_hop);
    }

    printf("================================================\n\n");
}
/*  
    Function that Continually Prints the details, 
    of the router Table every two seconds
*/
void *print_router_table_thread()
{
    while (1)
    {
        pthread_mutex_lock(&table_lock);
        print_router_table();
        pthread_mutex_unlock(&table_lock);
        sleep(5);
    }
    pthread_exit(NULL);
}

void add_to_router_table(int port_number, int distance, int next_hop) {
    int i, added;
    pthread_mutex_lock(&table_lock);
    added = 0;
    for (i = 0; i < 20; i++) {
        if (router_table[i].port_number == 0) { /* empty slot */
            router_table[i].port_number = port_number;
            router_table[i].distance = distance;
            router_table[i].next_hop = next_hop;
            added = 1;
            break;
        }
    }
    pthread_mutex_unlock(&table_lock);

    if (!added) {
        printf("Router table full! Cannot add new router.\n");
    }
}


void update_router_table(int port_number, int distance, int next_hop) {
    int i = find_router_index(port_number);
    pthread_mutex_lock(&table_lock);
    router_table[i].port_number = port_number;
    router_table[i].distance = distance;
    router_table[i].next_hop = next_hop;
    pthread_mutex_unlock(&table_lock);
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
            continue;
        }

        /* Extract client port */
        recv(connect_fd, &neighbour_port, sizeof(int), 0);
        neighbour_port = ntohl(neighbour_port);
        printf("Accepted neighbour connection listening on port: %d\n", neighbour_port);
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
    }

    /* Bind socket */ 
    if (bind(self_socket_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)) != 0) { 
        printf("Socket bind failed...\n"); 
        pthread_exit(NULL);
    }

    /* Now server is ready to listen and verification */
    if ((listen(self_socket_fd, 20)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    }

    /* Create a thread that just accepts new connections */
    /* Have all the accept logic be in the function of this thread */
    if (pthread_create(&accept_neighbours_thread, NULL, accept_neighbours, NULL) != 0) {
        perror("pthread_create failed");
        exit(1); 
    }

    pthread_detach(accept_neighbours_thread);
    pthread_exit(NULL);
}

/* Connect to neighbour routers */
void *connect_to_neighbours(void *arg)
{
    int neighbour_socket_fd, neighbour_port_number, i, my_port;
    struct sockaddr_in neighbour_addr;
    neighbour_port_number = *(int *)arg;

    memset(&neighbour_addr, 0, sizeof(neighbour_addr));
    neighbour_addr.sin_family = AF_INET;
    neighbour_addr.sin_port = htons(neighbour_port_number);
    inet_pton(AF_INET, "127.0.0.1", &neighbour_addr.sin_addr);

    while (1)
    {
        /* client socket create and verification */
        neighbour_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (neighbour_socket_fd < 0) { 
            printf("Socket creation failed...\n"); 
            sleep(1);
            continue;
        }
        
        /* connect the client socket to server socket */
        if (connect(neighbour_socket_fd, (struct sockaddr *)&neighbour_addr, sizeof(neighbour_addr)) == 0){
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
            free(arg);

            /* Send port Number */
            my_port = htonl(self_port_number);
            send(neighbour_socket_fd, &my_port, sizeof(int), 0);
            break;
                    
        }
                
        close(neighbour_socket_fd);
        printf("Retrying connection to port %d...\n", neighbour_port_number);
        sleep(1); 
    }
    pthread_exit(NULL);
}


/* Send Info to neighbours */
void *send_to_neighbours()
{
    struct MSG msg;
    int i, j;
    int bytes_sent;

    while (1)
    {   
        /* send msg to only neighbours */
        for (i = 0; i < 20; i++) {
            /* Skip Non neighbour entries */
            if (router_table[i].is_neighbour == 0)
            continue;
            
            
            for (j = 0; j < 20; j++) {
                /* Skip empty entries */
                if (router_table[j].port_number == 0)
                continue;
                
                /* Construct msg */
                msg.sender_id = self_port_number;
                msg.destination_id = router_table[j].port_number;
                msg.distance = router_table[j].distance;   


                /* Implement Poisoned Reverse */
                if (router_table[j].next_hop == router_table[i].port_number) {
                    msg.distance = INF;
                }
                
                /* Send message to router */
                bytes_sent = send(router_table[i].sock_fd,
                                  &msg,
                                  sizeof(msg),
                                  0);

                if (bytes_sent < 0) {
                    continue;
                }
            }
            
        }
        sleep(5);
    }
        
    pthread_exit(NULL);
}

/* Prints a MSG struct */
void print_msg(struct MSG *msg)
{
    if (msg == NULL) {
        printf("msg is NULL\n");
        return;
    }

    printf("----- Distance Vector Entry -----\n");
    printf("Sender ID      : %d\n", msg->sender_id);
    printf("Destination ID : %d\n", msg->destination_id);
    printf("Cost           : %d\n", msg->distance);
    printf("----------------------------------\n");
}

/* Receive Info from neighbours */
void *recieve_from_neighbours()
{
    struct MSG msg;
    int i, sender_index, dest_index, new_cost;
    int bytes_received;

    while (1)
    {
        for (i = 0; i < 20; i++) {

            /* Skip non-neighbours */
            if (router_table[i].is_neighbour == 0)
                continue;

            bytes_received = recv(router_table[i].sock_fd,
                                  &msg,
                                  sizeof(msg),
                                  MSG_DONTWAIT);

            /* No data available */
            if (bytes_received <= 0)
                continue;

            /* Lookup indices once */
            sender_index = find_router_index(msg.sender_id);
            dest_index   = find_router_index(msg.destination_id);

            if (sender_index == -1)
                continue;   /* Unknown sender */

            pthread_mutex_lock(&table_lock);
            new_cost = msg.distance + router_table[sender_index].distance;
            pthread_mutex_unlock(&table_lock);

            /* Case 1: Destination not in table → add it */
            if (dest_index == -1) {
                add_to_router_table(msg.destination_id, new_cost, msg.sender_id);
                continue;
            }

            /* Case 2: Found destination → check for better path */
            if (new_cost < router_table[dest_index].distance) {
                update_router_table(msg.destination_id, new_cost, msg.sender_id);
            }
        }
        sleep(2);
    }

    pthread_exit(NULL);
}
/**/
int main(int argc, char *arg[]){

    int i, router_port, router_distance, router_index, *arg_port;
    /* Check Command line arguments validity */
    if ((argc % 2) != 0 ) {
        printf("Usage: ./d_vec <self_port_number> [ <port number> <distance> ... ] \n");
        exit(-1);
    }

    self_port_number = atoi(arg[1]);
    if (self_port_number < 30000 || self_port_number > 40000) {
        fprintf(stderr, "Port must be between 30000 and 40000\n");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(self_port_number);
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
        router_distance = atoi(arg[i+1]);
        add_to_router_table(router_port, router_distance, -1);
        router_index = find_router_index(router_port);
        router_table[router_index].is_neighbour = true;
        
        /*  
        Set up a TCP socket and connect to any router that their
        port number is bigger than self port number 
        */
       arg_port = malloc(sizeof(int));
       *arg_port = router_port;
       if ( router_port > self_port_number ) {
           if (pthread_create(&connect_thread, NULL, connect_to_neighbours, arg_port) != 0) {
               perror("pthread_create failed");
               return 1;
            }
            pthread_detach(connect_thread);
        }
        i += 2;
    }

    /* Create a thread that just accepts new connections */
    /* Have all the accept logic be in the function of this thread */
    if (pthread_create(&bind_listen_thread, NULL, bind_listen_to_neighbours, NULL) != 0) {
        perror("pthread_create failed");
        return 1;
    }
    pthread_detach(bind_listen_thread);

    if (pthread_create(&print_thread, NULL, print_router_table_thread, NULL) != 0) {
        perror("pthread_create failed");
        return 1;
    }
    pthread_detach(print_thread);

    if (pthread_create(&sender_thread, NULL, send_to_neighbours, NULL) != 0) {
        perror("pthread_create failed");
        return 1;
    }
    pthread_detach(sender_thread);

    if (pthread_create(&receiver_thread, NULL, recieve_from_neighbours, NULL) != 0) {
        perror("pthread_create failed");
        return 1;
    }
    pthread_detach(receiver_thread);

    
    /* main can now do other work or sleep */
    pause();
  
    close(self_socket_fd);
    return 0;
}