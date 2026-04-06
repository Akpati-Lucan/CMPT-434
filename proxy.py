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
from raft_header import Message, Server, Label

####################################################################################
# Global Variables

keepRunning = True # Global Variable to stop threads
electing = False # Global Variable to indicate if an election is in progress

hostname = 'localhost'
proxy_name = None
proxy_port = None
proxy_socket = None
outgoing_messages = Queue()
incoming_messages = Queue()

table_of_nodes = []
MAX_SERVERS = 20

####################################################################################

def print_server_table():
    global table_of_nodes
    global proxy_name, proxy_port

    # === Node Info ===
    print("\n=== Current Proxy Info ===")
    print(f"{proxy_name}:{proxy_port}")

    # === Server Table ===
    print("\n=== Server Table ===")
    print(f"{'Name':<20} {'Port':<10} {'Is Leader':<10}")
    print("-" * 45)

    for server in table_of_nodes:
        leader_status = "Yes" if server.is_leader else "No"
        print(f"{server.name:<20} {server.port:<10} {leader_status:<10}")

    print("-" * 45)

####################################################################################

def sender_thread():
    while keepRunning:
        try:
            msg = outgoing_messages.get(timeout=0.5)
            serialized = pickle.dumps(msg)
            proxy_socket.sendto(serialized, (hostname, msg.dest_port))
        except Empty:
            continue
        except Exception as e:
            print("Send error:", e)

def receiver_thread():
    while keepRunning:
        try:
            data, addr = proxy_socket.recvfrom(2048)
            msg = pickle.loads(data)
            incoming_messages.put(msg)

        except socket.timeout:
            continue  # lets loop re-check keepRunning

        except Exception as e:
            print("Receiver error:", e)


def keyboard_thread():
    global keepRunning

    while keepRunning:
        user_input = input("Enter message to send: ")

        if user_input == "exit":
            keepRunning = False
            break

def main_server():
    global keepRunning
    while keepRunning:

        try:
            msg = incoming_messages.get(timeout=0.5)
        except Empty:
            continue  # lets loop re-check keepRunning


        #print(f"Proxy got \"{msg.msg}\" with label {msg.label.name} from {msg.source_name}:{msg.source_port}")

        if msg.label == Label.NEW_SERVER:
            new_server = Server(msg.source_name, msg.source_port)
            table_of_nodes.append(new_server)

            for server in table_of_nodes:
                add_msg = Message(
                    Label.ADD_SERVER,
                    0,
                    msg.source_name,
                    msg.source_port,
                    server.name,
                    server.port
                )
                outgoing_messages.put(add_msg)

        if msg.label == Label.NEW_LEADER:
            # set the flag of the old leader to zero
            for node in table_of_nodes:
                if node.is_leader:
                    node.is_leader = False
            # Set the flag in the sever table of the new leader to 1
            for node in table_of_nodes:
                if (node.name == msg.source_name) and (node.port == msg.source_port):
                    node.is_leader = True
            outgoing_messages.put(msg)
        else:
            outgoing_messages.put(msg)

def setup_udp_socket():
    global proxy_socket
    proxy_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    proxy_socket.bind((hostname, proxy_port))
    proxy_socket.settimeout(0.5)

def parse_command_line_arguments():
    args = sys.argv

    global proxy_name
    global proxy_port

    # Minimum required args: script + proxy_name + proxy_port + leader_name + leader_port
    if len(args) < 5:
        print("Usage: python proxy.py proxy_name proxy_port leader_name leader_port [server_name server_port ...]")
        sys.exit(1)

    # Extract proxy info
    proxy_name = args[1]
    proxy_port = int(args[2])

    # Extract leader info
    leader_name = args[3]
    leader_port = int(args[4])

    # Add leader to table
    table_of_nodes.append(Server(leader_name, leader_port, is_leader=True))

    # Remaining args (servers start from index 5)
    remaining = args[5:]

    # Must be pairs (hostname, port)
    if len(remaining) % 2 != 0:
        print("Error: Server arguments must be in pairs (hostname port)")
        sys.exit(1)

    for i in range(0, len(remaining), 2):
        name = remaining[i]
        port = int(remaining[i + 1])

        if len(table_of_nodes) >= MAX_SERVERS:
            print("Error: Maximum number of servers reached")
            break

        table_of_nodes.append(Server(name, port))

def main():
    parse_command_line_arguments()
    print_server_table()
    setup_udp_socket()

    # Create threads
    sender = threading.Thread(target=sender_thread, args=())
    receiver = threading.Thread(target=receiver_thread, args=())
    server = threading.Thread(target=main_server, args=())
    keyboard = threading.Thread(target=keyboard_thread, args=())

    # Start threads
    sender.start()
    receiver.start()
    server.start()
    keyboard.start()

if __name__ == "__main__":
    main()
