# Name: Lucan Akpati
# NSID: jbs671
# Student Number: 11331253

# Name: Samuel Olukuewu
# NSID: nds091
# Student Number: 11323380

# Distributed Raft-Based Server System

This project implements a simplified distributed system using a Raft-like protocol with a proxy and multiple server nodes (leader and followers).

---

IMPORTANT NOTES

* All nodes run on localhost by default
  You do NOT need to manually specify "localhost" unless desired.

* Leader declaration rule (VERY IMPORTANT)

  * A node should ONLY declare itself as leader at startup.
  * If a node crashes, restarts, or joins an existing network, it must NOT declare itself as leader.
  * Instead, it must start using the "join" command to safely re-enter the cluster.

---

SYSTEM COMPONENTS

* Proxy: Routes messages between nodes
* Leader: Main coordinator node
* Followers: Replicate state and respond to the leader

---

RUNNING THE SYSTEM

---

1. THREE SERVERS (1 Proxy, 1 Leader, 1 Follower)

Start Proxy:
python proxy.py proxy_name proxy_port leader_name leader_port [server_name server_port ...]

Example:
python proxy.py proxy 9000 node1 8000 node2 10001

---

Start Leader:
python raft.py proxy_name proxy_port server_name server_port 1 [server_name server_port ...]

Example:
python raft.py proxy 9000 node1 8000 1 node2 10001

---

Start Follower:
python raft.py proxy_name proxy_port server_name server_port 0 leader_name leader_port [server_name server_port ...]

Example:
python raft.py localhost 9000 localhost 10001 0 localhost 8000

---

2. FOUR SERVERS (1 Proxy, 1 Leader, 2 Followers + Additional Node)

Start Proxy:
python proxy.py proxy_name proxy_port leader_name leader_port [server_name server_port ...]

Examples:
python proxy.py localhost 9000 localhost 8000 localhost 10001 localhost 10002
python proxy.py tux5 30001 tux5 30002 tux5 30003 tux5 30004

---

Start Leader:
python raft.py proxy_name proxy_port server_name server_port 1 [server_name server_port ...]

Examples:
python raft.py localhost 9000 localhost 8000 1 localhost 10001 localhost 10002
python raft.py tux5 30001 tux5 30002 1 tux5 30003 tux5 30004

---

Start Followers:

Follower with known leader:
python raft.py proxy_name proxy_port server_name server_port 0 leader_name leader_port [server_name server_port ...]

Examples:
python raft.py localhost 9000 localhost 10001 0 localhost 8000 localhost 10002
python raft.py tux5 30001 tux5 30003 0 tux5 30002 tux5 30004

python raft.py localhost 9000 localhost 10002 0 localhost 8000 localhost 10001
python raft.py tux5 30001 tux5 30004 0 tux5 30002 tux5 30003

---

JOINING AN EXISTING NETWORK (DYNAMIC NODE)

Use this when:

* A node crashes and restarts
* A node is new and joining an existing cluster

Command:
python raft.py proxy_name proxy_port server_name server_port 0 join

Examples:
python raft.py localhost 9000 localhost 10006 0 join
python raft.py tux5 30001 tux5 30005 0 join

---

SUMMARY OF CORRECT BEHAVIOR

* Start one leader only using flag "1"
* All other nodes must start as followers ("0")
* Any node joining after initialization must use:
  0 join
* Never start multiple leaders in the same cluster

---

COMMON MISTAKES

* Starting multiple leaders
* Restarting a crashed node as leader
* Forgetting to use "join" when re-entering the system
* Starting nodes before the proxy is running

---
