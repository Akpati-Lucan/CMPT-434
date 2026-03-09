# Implementation of Replicated Log
## Replicated State Machines
System in which a group of processes maintain a totally-ordered log of the messages that each process is receiving from stdin.

# Process States
Each Process has a default state that contains the log with empty initialized values for the case a follower is ellected to leader.
Process messages are passed through a proxy server.

# Leader State
The tasks performed by the leader
- Read Input from STDIN
- Adds input to log and sends update to followers
- Handles case of follower crash
- Periodically Send Heartbeat to Followers

# Follower State
The tasks performed by the follower
- Read Input from STDIN
- Adds input to log and sends update to Leader
- Periodically Check Leader's Heartbeat
- If Heartbeat fails, declare election
- Caller of election receives votes from Followers
- Print Their Own Log to Stdin

# Communication Model
All processes communicate indirectly through a proxy server. No process sends messages directly to another process.
The proxy is responsible for forwarding messages while optionally simulating:
- Message delays
- Message loss
- Message reordering
- Temporary disconnections
This allows testing the robustness of the replicated log system under unreliable network conditions.

# Protocols
## Heartbeat Protocol
The Heartbeat Protocol ensures that followers know the leader is still active and prevents unnecessary leader elections.
### Participants
- Leader
- Follower
### Message Flow
- Leader → Proxy → Followers
- Followers → Proxy → Leader
### Protocol Steps
1. The leader periodically sends a heartbeat message to the proxy.
2. The proxy forwards the heartbeat to all followers.
3. When a follower receives the heartbeat:
- It resets its heartbeat timeout timer.
- It sends a HEARTBEAT_ACK message to the proxy.
4. The proxy forwards the acknowledgement to the leader.
5. If a follower does not receive a heartbeat within the timeout period:
  - It assumes the leader has failed.
  - It initiates the Election Protocol.

## Election Protocol
The Election Protocol selects a new leader if the current leader fails.
### Participants
- Candidate
- Followers
- Proxy Server
### Message Flow
- Candidate → Proxy → All Other Nodes
- Nodes → Proxy → Candidate
### Protocol Steps
1. A follower detects heartbeat timeout.
2. The follower becomes a candidate and increments its term.
3. The proxy forwards the request to all other nodes.
4. When a node receives a VOTE_REQUEST, it grants a vote if:
- It has not already voted in the current term.
- The candidate’s log is up-to-date.
5. The node sends a response to the proxy.
6. The proxy forwards responses to the candidate.
7. If the candidate receives votes from a majority of nodes, it becomes the leader.
8. The new leader immediately begins sending heartbeat messages through the proxy.

## Log Update
- Follower Receives Input
- Sends Update Signal to Leader
- Leader Acks and Sends "Commit" Signal to Followers
- Leader Sends Updated Log to all followers

## Follower Safety
  
