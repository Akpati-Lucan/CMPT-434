# Project Proposal  
## Distributed Log Replication System using Raft and a Proxy Router

**Authors**

- **Lucan Akpati**  
  NSID: jbs671  
  Student Number: 11331253  

- **Samuel Olukuewu**  
  NSID: nds091  
  Student Number: 11323380  


---

# 1. Project Overview

This project implements a **simplified distributed consensus and log replication system inspired by the Raft protocol**. The system consists of multiple server nodes and a proxy server that handles message routing between them. Each server maintains a replicated log and participates in **leader election, heartbeat monitoring, and log replication** to ensure consistency across the distributed system.

The primary objective of the project is to demonstrate how distributed nodes can coordinate with one another to maintain a **consistent log of events even in the presence of failures**.

The proxy server acts as a **centralized communication relay** that forwards messages between servers.

The implementation will be written in **Python**, using:

- **UDP sockets** for communication  
- **Multithreading** for concurrent tasks  
- **Message queues** for asynchronous message handling  


---

# 2. System Architecture

The system consists of two main components:

1. **Server Nodes**
2. **Proxy Server**

## Server Nodes

Server nodes perform the main logic of the distributed system. Their responsibilities include:

- Leader election
- Heartbeat monitoring
- Log replication
- Message processing

At any given time:

- One server acts as the **leader**
- All other servers act as **followers**

The leader coordinates updates and ensures consistency across the distributed log.

## Proxy Server

The proxy server acts as a **message router**.

Instead of servers communicating directly with each other, they send all messages to the proxy. The proxy then forwards the messages to the correct destination server.

This architecture:

- Simplifies network communication
- Centralizes message routing
- Reduces complexity in the server implementation


---

# 3. Implementation Plan

The implementation will be completed in **multiple stages** so that each component can be tested independently.

---

## Stage 1: Data Structures and Message Format

The first stage defines the core data structures used by the system.

### Message Structure

The message structure stores all information required for communication between servers, including:

- Message type (label)
- Source server
- Destination server
- Log sequence number
- Message contents
- Vote information for elections

### Server Structure

The server structure stores information about each server in the network:

- Hostname
- Port number
- Leader status

### Server Table

A **server table** will maintain information about all nodes currently participating in the distributed system.

---

## Stage 2: Network Communication

The second stage implements the **network communication layer** using UDP sockets.

Each server and proxy will:

1. Open a UDP socket
2. Listen for incoming messages
3. Send messages through the network

Messages will be:

- **Serialized before transmission**
- **Deserialized after reception**

### Communication Threads

Two threads will manage networking:

**Receiver Thread**

- Listens for incoming messages
- Places received messages into a processing queue

**Sender Thread**

- Retrieves messages from an outgoing queue
- Sends them through the UDP socket

Separating networking from processing improves system efficiency and prevents blocking operations.

---

## Stage 3: Proxy Message Routing

The proxy server acts as the **communication hub** for the entire system.

Its responsibilities include:

- Receiving messages from servers
- Determining the appropriate destination
- Forwarding messages to the destination server

### Handling New Servers

When a new server joins:

1. The proxy adds the server to its **server table**
2. The proxy notifies all existing servers about the new node

For all other message types, the proxy simply **forwards the message unchanged**.

---

## Stage 4: Leader Election

Leader election ensures that **one server coordinates the system at any time**.

If a server does not receive heartbeat messages from the leader within a specified timeout:

1. The server starts an **election**
2. Vote requests are sent to all other servers
3. Servers respond with their vote

If a server receives votes from a **majority of nodes**, it becomes the **new leader** and notifies the cluster.

This mechanism allows the system to **recover automatically from leader failure**.

---

## Stage 5: Heartbeat Monitoring

The leader periodically sends **heartbeat messages** to all follower servers.

These messages indicate that the leader is still active.

If followers stop receiving heartbeat messages:

- They assume the leader has failed
- They initiate a **new election**

Heartbeat monitoring allows the system to detect failures quickly and maintain availability.

---

## Stage 6: Log Replication

Log replication ensures that all servers maintain **consistent copies of the system log**.

When a new log entry is created:

1. The leader sends the entry to all followers
2. Followers store the entry and send acknowledgments
3. Once a **majority of servers acknowledge the entry**, the leader commits the entry
4. The leader instructs followers to commit the entry as well

This ensures **strong consistency across the distributed system**.

---

## Stage 7: Server Synchronization

If a follower falls behind the leader’s log sequence:

1. The follower sends an update request
2. The leader sends missing log entries
3. The follower updates its log until it matches the leader

This ensures that all servers **eventually reach the same state**.

---

# 4. Multithreading Design

Each server runs several threads concurrently.

| Thread | Responsibility |
|------|------|
| Receiver Thread | Handles incoming network messages |
| Sender Thread | Sends outgoing messages |
| Heartbeat Thread | Periodically sends heartbeat signals |
| Election Thread | Handles leader election |
| Keyboard Thread | Accepts user log input |
| Main Processing Thread | Processes incoming messages |

Using separate threads allows the system to handle **communication, elections, and replication simultaneously**.

---

# 5. Evaluation Tasks

The following tasks will be used to verify that the system works correctly.

### Task 1: Basic Communication
Verify that servers can send and receive messages through the proxy.

### Task 2: Server Registration
Start multiple servers and confirm that the proxy correctly registers them.

### Task 3: Leader Election
Stop the current leader and verify that a new leader is automatically elected.

### Task 4: Heartbeat Detection
Ensure that followers detect missing heartbeat signals and initiate elections.

### Task 5: Log Replication
Submit log entries and confirm that they are replicated across all servers.

### Task 6: Commit Verification
Ensure that log entries are committed only after receiving acknowledgments from a majority of servers.

### Task 7: Server Synchronization
Start a server after the system is already running and verify that it synchronizes its log with the leader.

### Task 8: Fault Recovery
Simulate server failures and confirm that the system continues operating correctly.

---

# 6. Expected Outcome

At the end of the project, the system should demonstrate a working **distributed log replication protocol** that includes:

- Leader election
- Heartbeat monitoring
- Log replication
- Proxy-based message routing
- Fault tolerance

Multiple server nodes will communicate through the proxy, elect a leader, and maintain a **consistent distributed log**.

---

# 7. Conclusion

This project demonstrates important concepts in **distributed systems**, including consensus, fault detection, and data replication.

By separating the **proxy routing logic** from the **server logic** and using multithreading for concurrent operations, the system simulates a realistic distributed environment.

The staged implementation plan and evaluation tasks ensure that the system can be **tested incrementally and validated throughout development**.