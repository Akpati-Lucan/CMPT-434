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
from raft_header import Message, Server, Label

####################################################################################
# Global Variables

proxy_hostname = None
proxy_port = None
proxy_socket = None
outgoing_messages = Queue()
incoming_messages = Queue()

table_of_servers = []
MAX_SERVERS = 20

####################################################################################


def sender_thread():

    while True:
        #msg = outgoing_messages.get()
        user_msg = input("Enter message to send (or 'quit'): ")

        if user_msg.lower() == "quit":
            print("Exiting sender thread...")
            break

        if not table_of_servers:
            print("No leader available yet.")
            continue

        leader = table_of_servers[0]
        msg = Message(
            label="APPEND",
            seq_number=1,
            source_name=proxy_hostname,
            source_port=proxy_port,
            dest_name=leader.hostname,
            dest_port=leader.port,
            msg=user_msg,
            vote_for=0
        )
        try:
            serialized = pickle.dumps(msg)
            proxy_socket.sendto(serialized, (msg.dest_name, msg.dest_port))


        except Exception as e:
            print("Send error:", e)

        time.sleep(1)  # prevent flooding

def receiver_thread():
    global proxy_socket
    global outgoing_messages
    global incoming_messages

    while True:
        data, addr = proxy_socket.recvfrom(2048)
        try:
            msg = pickle.loads(data)
        except Exception as e:
            print("Error decoding message:", e)
            continue

        incoming_messages.put(msg)

def main_server():

    while True:

        msg = incoming_messages.get()

        print(f"Server got {msg.msg} from {msg.source_name}:{msg.source_port}")
        if msg.label == Label.NEW_SERVER:

            new_server = Server(msg.source_name, msg.source_port)

            table_of_servers.append(new_server)

            for server in table_of_servers:

                add_msg = Message(
                    Label.ADD_SERVER,
                    0,
                    msg.source_name,
                    msg.source_port,
                    server.hostname,
                    server.port
                )

                outgoing_messages.put(add_msg)

        else:

            outgoing_messages.put(msg)

def setup_udp_socket():
    global proxy_socket
    proxy_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    proxy_socket.bind((proxy_hostname, proxy_port))

    print("UDP server is listening...")


def parse_command_line_arguments():
    args = sys.argv

    global proxy_hostname
    global proxy_port

    # Minimum required args: script + proxy_name + proxy_port + leader_name + leader_port
    if len(args) < 5:
        print("Usage: python proxy.py proxy_name proxy_port leader_name leader_port [server_name server_port ...]")
        sys.exit(1)

    # Extract proxy info
    proxy_hostname = args[1]
    proxy_port = int(args[2])

    # Extract leader info
    leader_name = args[3]
    leader_port = int(args[4])

    # Add leader to table
    table_of_servers.append(Server(leader_name, leader_port, is_leader=True))

    # Remaining args (servers start from index 5)
    remaining = args[5:]

    # Must be pairs (hostname, port)
    if len(remaining) % 2 != 0:
        print("Error: Server arguments must be in pairs (hostname port)")
        sys.exit(1)

    for i in range(0, len(remaining), 2):
        hostname = remaining[i]
        port = int(remaining[i + 1])

        if len(table_of_servers) >= MAX_SERVERS:
            print("Error: Maximum number of servers reached")
            break

        table_of_servers.append(Server(hostname, port))


def main():
    parse_command_line_arguments()
    setup_udp_socket()

    # Create threads
    sender = threading.Thread(target=sender_thread, args=())
    receiver = threading.Thread(target=receiver_thread, args=())
    server = threading.Thread(target=main_server, args=())

    # Start threads
    sender.start()
    receiver.start()
    server.start()

if __name__ == "__main__":
    main()
