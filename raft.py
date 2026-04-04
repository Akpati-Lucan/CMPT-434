# Name: Lucan Akpati
# NSID: jbs671
# Student Number: 11331253

# Name: Samuel Olukuewu
# NSID: nds091
# Student Number: 11323380

from queue import Queue
import pickle
import socket
import sys
import threading
import time
import random
from threading import Thread

from raft_header import Message, Server, Label

####################################################################################
# Global Variables

server_hostname = None
server_port = None
proxy_hostname = None
proxy_port = None
leader_hostname = None
leader_port = None
server_socket = None
outgoing_messages = Queue()
incoming_messages = Queue()

number_of_votes = 0
number_of_commits = 0

heartbeat_timeout = random.randint(0, 5)
election_timeout = random.randint(0, 5)
append_timeout = random.randint(0, 5)

heartbeat_ack = False
is_leader = False
seq_number_of_log = 0

table_of_servers = []
MAX_SERVERS = 20
log = [""] * 1024
log_append_tracker = [0] * 1024

election = Thread # A thread
heartbeat = Thread
append_handler = Thread

####################################################################################
def print_server_table():
    global table_of_servers
    global server_hostname, server_port
    global proxy_hostname, proxy_port
    global leader_hostname, leader_port

    # === Node Info ===
    print("\n=== Current Node Info ===")
    print(f"{'Proxy:':<15} {proxy_hostname}:{proxy_port}")
    print(f"{'Server:':<15} {server_hostname}:{server_port}")
    print(f"{'Leader:':<15} {leader_hostname}:{leader_port}")

    # === Server Table ===
    print("\n=== Server Table ===")
    print(f"{'Hostname':<20} {'Port':<10} {'Leader':<10}")
    print("-" * 45)

    for server in table_of_servers:
        leader_status = "Yes" if server.is_leader else "No"
        print(f"{server.hostname:<20} {server.port:<10} {leader_status:<10}")

    print("-" * 45)
####################################################################################


def heartbeat_thread():
    global heartbeat_ack
    global election
    while True:

        msg = Message(Label.HEARTBEAT, 0, server_hostname, server_port, leader_hostname, leader_port, "", 0)
        outgoing_messages.put(msg)

        heartbeat_ack = False

        time.sleep(heartbeat_timeout)

        if not heartbeat_ack:
            Thread(target=election_thread).start()
            break

def election_thread():
    global number_of_votes
    global is_leader
    global heartbeat

    for server in table_of_servers:
        msg = Message(Label.VOTE, seq_number_of_log, server_hostname, server_port, server.hostname, server.port, "", 0)
        outgoing_messages.put(msg)

    number_of_votes += 1

    time.sleep(election_timeout)

    if number_of_votes >= len(table_of_servers) / 2:

        is_leader = True

        for server in table_of_servers:
            msg = Message(Label.NEW_LEADER, 0, server_hostname, server_port, server.hostname, server.port, "", 0)
            outgoing_messages.put(msg)

    else:
        Thread(target=heartbeat_thread).start()

    number_of_votes = 0

def append_handler_thread(seq_number):

    global log_append_tracker
    log_append_tracker[seq_number] += 1

    time.sleep(append_timeout)

    if log_append_tracker[seq_number] >= len(table_of_servers) / 2:

        for server in table_of_servers:
            msg = Message(Label.COMMIT, seq_number, server_hostname, server_port, server.hostname, server.port, "", 0)
            outgoing_messages.put(msg)
    else:

        for server in table_of_servers:
            msg = Message(Label.REJECT, seq_number, server_hostname, server_port, server.hostname, server.port, "", 0)
            outgoing_messages.put(msg)

def keyboard_thread():

    while True:
        user_input = input("Enter message to send ")

        if is_leader:
            for server in table_of_servers:
                msg = Message(Label.APPEND, seq_number_of_log, server_hostname, server_port, server.hostname, server.port, user_input, 0)
                outgoing_messages.put(msg)

            #Thread(target=append_handler_thread, args=(seq_number_of_log,)).start()

        else:
            msg = Message(Label.NEW_LOG_VALUE, 0, server_hostname, server_port, leader_hostname, leader_port, user_input, 0)
            outgoing_messages.put(msg)

def sender_thread():
    while True:
        try:
            msg = outgoing_messages.get()
            serialized = pickle.dumps(msg)
            server_socket.sendto(serialized, (proxy_hostname, proxy_port))
        except Exception as e:
            print("Sender error:", e)

def receiver_thread():
    while True:
        try:
            data, addr = server_socket.recvfrom(2048)
            msg = pickle.loads(data)
            incoming_messages.put(msg)
            print(f"Server got {msg.msg} from {msg.source_name}:{msg.source_port}")
        except Exception as e:
            print("Sender error:", e)

def main_server():
    global seq_number_of_log
    global heartbeat_ack
    global number_of_votes
    while True:

        msg = incoming_messages.get()

        if msg.label == Label.NEW_LOG_VALUE:

            if is_leader:
                for server in table_of_servers:
                    msg = Message(Label.APPEND, seq_number_of_log, server_hostname, server_port, server.hostname,
                                  server.port, msg.msg, 0)
                    outgoing_messages.put(msg)

                append_handler.start(seq_number_of_log)

        elif msg.label == Label.APPEND:

            log[msg.seq_number] = msg.msg
            log_append_tracker[msg.seq_number] += 1

            msg = Message(Label.APPEND_ACK, msg.seq_number, server_hostname, server_port, leader_hostname,
                                  leader_port, msg.msg, 0)
            outgoing_messages.put(msg)

        elif msg.label == Label.APPEND_ACK:

            log_append_tracker[msg.seq_number] += 1

        elif msg.label == Label.REJECT:

            log[msg.seq_number] = ""

        elif msg.label == Label.COMMIT:

            seq_number_of_log += 1

        elif msg.label == Label.HEARTBEAT:
            msg = Message(Label.HEARTBEAT_ACK, 0, server_hostname, server_port, leader_hostname,
                                  leader_port, "", 0)
            outgoing_messages.put(msg)

        elif msg.label == Label.HEARTBEAT_ACK:

            heartbeat_ack = True

        elif msg.label == Label.VOTE:

            if msg.seq_number >= seq_number_of_log:
                msg = Message(Label.VOTE_FOR, msg.seq_number, server_hostname, server_port, msg.source_name, msg.source_port, vote_for=1)

            else:
                msg = Message(Label.VOTE_FOR, msg.seq_number, server_hostname, server_port, msg.source_name, msg.source_port, vote_for=0)

            outgoing_messages.put(msg)

        elif msg.label == Label.VOTE_FOR:

            if msg.vote_for == 1:
                number_of_votes += 1

        elif msg.label == Label.NEW_LEADER:
            # set the flag of the old leader to zero
            for server in table_of_servers:
                if server.is_leader:
                    server.is_leader = False
            # Set the flag in the sever table of the new leader to 1
            for server in table_of_servers:
                if (server.hostname == msg.source_name) and (server.port == msg.source_port):
                    server.is_leader = True


        elif msg.label == Label.UPDATE_CHECK:
            # Leader receives request to confirm the log position
            # Leader sends back an update signal if the seq number of follower is less than the current log seq number
            if msg.seq_number < seq_number_of_log:
                msg = Message(Label.UPDATE_SIGNAL, seq_number_of_log, server_hostname, server_port,
                               msg.source_name, msg.source_port,"", 0)
                outgoing_messages.put(msg)

        elif msg.label == Label.UPDATE_SIGNAL:
            # Follower initiates update sequence
            msg = Message(Label.UPDATE, seq_number_of_log, server_hostname, server_port,
                                  leader_hostname, leader_port,"",0)
            outgoing_messages.put(msg)

        elif msg.label == Label.UPDATE:
            # Leader sends missing log entry
            missing_log_value = log[msg.seq_number]

            msg = Message(Label.UPDATE_ACK, msg.seq_number, server_hostname, server_port,
                               msg.source_name, msg.source_port, missing_log_value, 0)

            outgoing_messages.put(msg)

        elif msg.label == Label.UPDATE_ACK:
            # Follower applies update
            log[msg.seq_number] =  msg.msg  # <-- store log entry

            seq_number_of_log += 1

            # confirm again
            msg = Message(Label.UPDATE_CHECK, seq_number_of_log, server_hostname, server_port,
                              leader_hostname,leader_port,"",0)
            outgoing_messages.put(msg)

        elif msg.label == Label.ADD_SERVER:

            table_of_servers.append(
                Server(msg.source_name, msg.source_port)
            )

def setup_udp_socket():
    global server_socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_socket.bind((server_hostname, server_port))

def parse_server_list(args, start_index):

    remaining = args[start_index:]

    if len(remaining) % 2 != 0:
        raise ValueError("Server arguments must be in pairs (hostname port)")

    for i in range(0, len(remaining), 2):
        hostname = remaining[i]
        port = int(remaining[i + 1])

        if len(table_of_servers) >= MAX_SERVERS:
            print("Error: Maximum number of servers reached")
            break

        table_of_servers.append(Server(hostname, port))

def parse_command_line_arguments():
    args = sys.argv

    global proxy_hostname
    global proxy_port
    global server_hostname
    global server_port
    global leader_hostname
    global leader_port
    global is_leader

    # Minimum required args: script + proxy_name + proxy_port + 0 + leader_name + leader_port
    if len(args) < 5:
        print("Usage: python raft.py proxy_name proxy_port server_name server_port 1 [server_name server_port ...]")
        print("Usage: python raft.py proxy_name proxy_port server_name server_port 0 leader_name leader port [server_name server_port ...]")
        sys.exit(1)

    # The 0 after leader_port tell the program whether it's the leader or not
    # Extract proxy info
    proxy_hostname = args[1]
    proxy_port = int(args[2])

    # Extract server info
    server_hostname = args[3]
    server_port = int(args[4])

    if int(args[5]) == 1:
        is_leader = True

        print("Leader server is listening...")
        # Remaining args (servers start from index 5)
        remaining = args[6:]

        parse_server_list(remaining, 0)

    if int(args[5]) == 0:
        is_leader = False

        # The next hostname and port number is the leader

        print("Follower server is listening...")
        # Extract leader info
        leader_hostname = args[6]
        leader_port = int(args[7])

        table_of_servers.append(Server(leader_hostname, leader_port))

        # Remaining args (servers start from index 7)
        remaining = args[8:]

        parse_server_list(remaining, 0)


def main():

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

    # Start threads
    sender.start()
    receiver.start()
    keyboard.start()

    #
    # server = threading.Thread(target=main_server, args=())
    # heartbeat = threading.Thread(target=heartbeat_thread, args=())
    #
    # election = Thread(target=election_thread)
    # append_handler = Thread(target=append_handler_thread, args=())
    #

    #server.start()
    #heartbeat.start()


if __name__ == "__main__":
    main()
