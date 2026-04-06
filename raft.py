# Name: Lucan Akpati
# NSID: jbs671
# Student Number: 11331253

# Name: Samuel Olukuewu
# NSID: nds091
# Student Number: 11323380

from queue import Queue, Empty
import pickle
import socket
import sys
import threading
import time
import random
from threading import Thread
from time import sleep

from raft_header import Message, Server, Label

####################################################################################
# Global Variables

keepRunning = True # Global Variable to stop threads
electing = False # Global Variable to indicate if an election is in progress

hostname = 'localhost'
node_name = None
node_port = None
proxy_name = None
proxy_port = None
leader_name = None
leader_port = None
node_socket = None
outgoing_messages = Queue()
incoming_messages = Queue()

number_of_votes = 0
number_of_commits = 0
term = 0
voted = False

heartbeat_timeout = random.randint(0, 5)
election_timeout = 1
append_timeout = random.uniform(0.005, 0.02)

heartbeat_ack = True
is_leader = False
seq_number_of_log = 0

table_of_nodes = []
Total_nodes = 0
MAX_SERVERS = 20
log = [""] * 1024
log_append_tracker = [0] * 1024

election = Thread # A thread
heartbeat = Thread
append_handler = Thread
has_joined = False

####################################################################################
def print_server_table():
    global table_of_nodes
    global node_name, node_port
    global proxy_name, proxy_port
    global leader_name, leader_port

    # === Node Info ===
    print("\n=== Current Node Info ===")
    print(f"{'Proxy:':<15} {proxy_name}-{proxy_port}")
    print(f"{'Current Node:':<15} {node_name}-{node_port}")
    print(f"{'Leader:':<15} {leader_name}-{leader_port}")

    # === Server Table ===
    print("\n=== Server Table ===")
    print(f"{'Name':<20} {'Port':<10} {'Leader':<10}")
    print("-" * 45)

    for node in table_of_nodes:
        leader_status = "Yes" if node.is_leader else "No"
        print(f"{node.name:<20} {node.port:<10} {leader_status:<10}")

    print("-" * 45)

def print_log():
    """
    Prints the current state of the log in order,
    only showing entries that have been appended.

    Uses log_append_tracker to determine valid log positions.
    """

    print("\n========= LOG STATE =========")

    if is_leader:
        for i in range(len(log)):
            if log[i] != "":
                print(f"Index {i}: message:{log[i]}   appends:{log_append_tracker[i]}")
        print("============================\n")

    else:
        for i in range(len(log)):
            if log[i] != "":
                print(f"Index {i}: message:{log[i]} ")
        print("============================\n")

####################################################################################

def heartbeat_thread():
    global heartbeat_ack
    global election
    global electing
    global leader_name, leader_port

    while keepRunning:
        if not is_leader:
            leader_name, leader_port = get_leader()
            
            if leader_name is not None:
                msg = Message(Label.HEARTBEAT, 0, node_name, node_port, leader_name, leader_port, "", 0, term)
                outgoing_messages.put(msg)
                heartbeat_ack = False
                time.sleep(heartbeat_timeout)
                
                if not heartbeat_ack:
                    if not electing:
                        electing = True
                        election = Thread(target=election_thread)
                        election.start()
            else:
                # No leader found, start election if not already electing
                if not electing:
                    electing = True
                    election = Thread(target=election_thread)
                    election.start()
        else:
            time.sleep(heartbeat_timeout)  # Leaders can sleep or handle other tasks

def election_thread():
    global number_of_votes
    global is_leader
    global term
    global heartbeat
    global electing

    term += 1
    number_of_votes += 1
    majority = len(table_of_nodes) // 2 + 1

    if not electing:
        return

    for node in table_of_nodes:
        if node.name == node_name and node.port == node_port:
            continue
        msg = Message(Label.VOTE, seq_number_of_log, node_name, node_port, node.name, node.port, "", 0, term)
        outgoing_messages.put(msg)

    time.sleep(election_timeout)

    if number_of_votes >= majority:
        is_leader = True
        leader_name = node_name
        leader_port = node_port
        print("Election won, becoming leader!")
        for node in table_of_nodes:
            msg = Message(Label.NEW_LEADER, 0, node_name, node_port, node.name, node.port, "", 0, term)
            outgoing_messages.put(msg)

    number_of_votes = 0
    electing = False

def append_handler_thread(seq_number, message):

    global log_append_tracker
    global seq_number_of_log
    log_append_tracker[seq_number] += 1

    time.sleep(append_timeout)

    if log_append_tracker[seq_number] >= len(table_of_nodes) / 2:

        for node in table_of_nodes:
            msg = Message(Label.COMMIT, seq_number, node_name, node_port, node.name, node.port, "", 0, term)
            outgoing_messages.put(msg)

        # Add msg to the log
        log[seq_number] = message
        seq_number_of_log += 1 # Increment seq number of log when message has been commited
        print_log()
    else:

        for node in table_of_nodes:
            msg = Message(Label.REJECT, seq_number, node_name, node_port, node.name, node.port, "", 0, term)
            outgoing_messages.put(msg)

        # Set log_append_tracker for that seq number to zero
        log_append_tracker[seq_number] = 0
        print_log()

def keyboard_thread():
    global seq_number_of_log
    global keepRunning

    while keepRunning:
        user_input = input("Enter message to send: ")

        if user_input == "exit":
            keepRunning = False
            break

        if user_input == "log":
            print_log()
            continue

        if user_input == "table":
            print_server_table()
            continue

        if is_leader:
            for node in table_of_nodes:
                msg = Message(Label.APPEND, seq_number_of_log, node_name, node_port, node.name, node.port, user_input, 0)
                outgoing_messages.put(msg)

            t = Thread(target=append_handler_thread, args=(seq_number_of_log, user_input))
            t.start()
            t.join() # BLOCK here until append_handler_thread finishes

        else:
            msg = Message(Label.NEW_LOG_VALUE, 0, node_name, node_port, leader_name, leader_port, user_input, 0)
            outgoing_messages.put(msg)

def sender_thread():
    while keepRunning:
        try:
            msg = outgoing_messages.get(timeout=0.5)
            serialized = pickle.dumps(msg)
            node_socket.sendto(serialized, (hostname, proxy_port))

        except Empty:
            continue

        except Exception as e:
            print("Send error:", e)

def receiver_thread():
    while keepRunning:
        try:
            data, addr = node_socket.recvfrom(2048)
            msg = pickle.loads(data)
            incoming_messages.put(msg)
        except socket.timeout:
            continue  # lets loop re-check keepRunning

        except Exception as e:
            print("Receive error:", e)

def main_server():
    global seq_number_of_log
    global heartbeat_ack
    global is_leader
    global number_of_votes
    global electing
    global term
    global leader_name
    global leader_port
    
    while keepRunning:

        try:
            msg = incoming_messages.get(timeout=0.5)

        except Empty:
            continue  # lets loop re-check keepRunning

        if msg.label == Label.NEW_LOG_VALUE:
            if is_leader:
                for node in table_of_nodes:
                    msg = Message(Label.APPEND, seq_number_of_log, node_name, node_port, node.name, node.port, msg.msg, 0)
                    outgoing_messages.put(msg)

                Thread(target=append_handler_thread, args=(seq_number_of_log, msg.msg)).start()

        elif msg.label == Label.APPEND:
            log[msg.seq_number] = msg.msg
            log_append_tracker[msg.seq_number] += 1

            msg = Message(Label.APPEND_ACK, msg.seq_number, node_name, node_port, leader_name, leader_port, msg.msg, 0, term)
            outgoing_messages.put(msg)

        elif msg.label == Label.APPEND_ACK:
            log_append_tracker[msg.seq_number] += 1

        elif msg.label == Label.REJECT:
            log[msg.seq_number] = ""
            print_log()

        elif msg.label == Label.COMMIT:
            seq_number_of_log = msg.seq_number
            print_log()

        elif msg.label == Label.HEARTBEAT:
            new_msg = Message(Label.HEARTBEAT_ACK, 0, node_name, node_port, msg.source_name, msg.source_port, "", 0, term)
            outgoing_messages.put(new_msg)

        elif msg.label == Label.HEARTBEAT_ACK:
            heartbeat_ack = True

        elif msg.label == Label.VOTE:
            electing = True
            if msg.term >= term:
                msg = Message(Label.VOTE_FOR, msg.seq_number, node_name, node_port, msg.source_name, msg.source_port, vote_for=1)
            else:
                msg = Message(Label.VOTE_FOR, msg.seq_number, node_name, node_port, msg.source_name, msg.source_port, vote_for=0)

            outgoing_messages.put(msg)

        elif msg.label == Label.VOTE_FOR:
            if msg.vote_for == 1:
                number_of_votes += 1

        elif msg.label == Label.NEW_LEADER:
            electing = False
            # set the flag of the old leader to zero
            for node in table_of_nodes:
                if node.is_leader:
                    node.is_leader = False
            # Set the flag in the node table of the new leader to 1
            for node in table_of_nodes:
                if (node.name == msg.source_name) and (node.port == msg.source_port):
                    node.is_leader = True
                    leader_name = msg.source_name
                    leader_port = msg.source_port

        elif msg.label == Label.UPDATE_CHECK:
            # Leader receives request to confirm the log position
            # Leader sends back an update signal if the seq number of follower is less than the current log seq number
            if msg.seq_number <= seq_number_of_log and is_leader:
                msg = Message(Label.UPDATE_SIGNAL, seq_number_of_log, node_name, node_port,
                               msg.source_name, msg.source_port,"", 0)
                outgoing_messages.put(msg)

        elif msg.label == Label.UPDATE_SIGNAL:
            # Follower initiates update sequence
            leader_name = msg.source_name
            leader_port = msg.source_port
            msg = Message(Label.UPDATE, seq_number_of_log, node_name, node_port,
                                  leader_name, leader_port,"",0)
            outgoing_messages.put(msg)

        elif msg.label == Label.UPDATE:
            # Leader sends missing log entry
            missing_log_value = log[msg.seq_number]

            msg = Message(Label.UPDATE_ACK, msg.seq_number, node_name, node_port,msg.source_name, msg.source_port, missing_log_value, 0, term)

            outgoing_messages.put(msg)

        elif msg.label == Label.UPDATE_ACK:
            # Follower applies update
            log[msg.seq_number] =  msg.msg  # <-- store log entry
            for node in table_of_nodes:
                if node.name == leader_name and node.port == leader_port:
                    node.is_leader = True
                else:
                    node.is_leader = False
            is_leader = False
            seq_number_of_log += 1
            print("Update applied, now a follower")
            print_log()
            print_server_table()
            msg = Message(Label.UPDATE_CHECK, seq_number_of_log, node_name, node_port, leader_name, leader_port, "", 0, term)
            outgoing_messages.put(msg)

        elif msg.label == Label.ADD_SERVER:
            table_of_nodes.append(
                Server(msg.source_name, msg.source_port, msg.vote_for)
            )

def setup_udp_socket():
    global node_socket
    node_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    node_socket.bind((hostname, node_port))
    node_socket.settimeout(0.5)

def get_leader():
    for server in table_of_nodes:
        if server.is_leader:
            return server.name, server.port
    return None, None

def join_server_thread():
    global heartbeat
    global leader_name, leader_port
    # Send Message to proxy to add you
    msg = Message(Label.NEW_SERVER, 0, node_name, node_port, proxy_name, proxy_port, "", 0, term)
    outgoing_messages.put(msg)
    while True:
        leader_name, leader_port = get_leader()
        if leader_name is not None:
            break
        sleep(0.5)
    msg = Message(Label.UPDATE_CHECK, 0, node_name, node_port, leader_name, leader_port, "", 0, term)
    outgoing_messages.put(msg)
    heartbeat = threading.Thread(target=heartbeat_thread, args=())
    heartbeat.start()


def parse_node_list(args, start_index):
    global Total_nodes
    remaining = args[start_index:]

    if len(remaining) % 2 != 0:
        raise ValueError("Server arguments must be in pairs (name port)")

    for i in range(0, len(remaining), 2):
        name = remaining[i]
        port = int(remaining[i + 1])

        if len(table_of_nodes) >= MAX_SERVERS:
            print("Error: Maximum number of servers reached")
            break

        table_of_nodes.append(Server(name, port))
        Total_nodes += 1

def parse_command_line_arguments():
    args = sys.argv

    global proxy_name
    global proxy_port
    global node_name
    global node_port
    global leader_name
    global leader_port
    global is_leader
    global Total_nodes

    # Minimum required args: script + proxy_name + proxy_port + 0 + leader_name + leader_port
    if len(args) < 5:
        print("Usage: python raft.py proxy_name proxy_port node_name node_port 1 [server_name server_port ...] \n OR \n")
        print("USage: python raft.py proxy_name proxy_port server_name server_port 0 leader_name leader_port [server_name server_port...]")
        print("USage: python raft.py proxy_name proxy_port server_name server_port 0 join ") # If leader is not known
        sys.exit(1)

    # The 0 after leader_port tell the program whether it's the leader or not
    # Extract proxy info
    proxy_name = args[1]
    proxy_port = int(args[2])

    # Extract node info
    node_name = args[3]
    node_port = int(args[4])
    Total_nodes += 1

    if int(args[5]) == 1:
        is_leader = True

        leader_name = args[3]
        leader_port = int(args[4])
        print("Leader Node is listening...")
        # Remaining args (servers start from index 5)
        remaining = args[6:]

        parse_node_list(remaining, 0)

    if int(args[5]) == 0:
        is_leader = False

        print("Follower Node is listening...")
        # Check if leader info was actually provided
        # The next hostname and port number is the leader
        if len(args) > 8:
            leader_name = args[6]
            leader_port = int(args[7])

            table_of_nodes.append(Server(leader_name, leader_port, True))
            Total_nodes += 1

            # Remaining args (servers start from index 8)
            remaining = args[8:]
            parse_node_list(remaining, 0)


def main():
    args = sys.argv
    parse_command_line_arguments()
    print_server_table()
    setup_udp_socket()

    global election
    global append_handler
    global heartbeat


    # Create threads
    keyboard = threading.Thread(target=keyboard_thread, args=())
    sender = threading.Thread(target=sender_thread, args=())
    receiver = threading.Thread(target=receiver_thread, args=())
    server = threading.Thread(target=main_server, args=())
    heartbeat = threading.Thread(target=heartbeat_thread, args=())
    election = threading.Thread(target=election_thread)

    # Start threads
    sender.start()
    receiver.start()
    keyboard.start()
    server.start()

    if args[6] == "join":
        t = Thread(target=join_server_thread, args=())
        t.start()

    if (leader_name is not None) or (leader_port is not None):
        heartbeat.start()

if __name__ == "__main__":
    main()

