# Implementation of Replicated Log

## Replicated State Machines
A replicated state machine is a system in which a group of processes maintain a **totally-ordered log** of the messages that each process receives from `stdin`.

Each process maintains its own copy of the log. The system ensures that the order of log entries is consistent across all processes. Any two processes queried for their logs will return either:

- Identical logs, or
- Logs where one is a prefix of the other.

This guarantees consistency across the distributed system.

---

# Process States

Each process begins in a **default follower state**.  
The process maintains a log initialized with empty values, allowing a follower to transition to a leader state if elected.

All processes maintain the following information:

- Local replicated log
- Current leader ID
- Current election term
- Heartbeat timeout timer

Process messages are passed through a **proxy server** rather than being sent directly between processes.

---

# Leader State

The leader is responsible for coordinating updates to the replicated log.

### Tasks performed by the leader

- Read input from `STDIN`
- Append input messages to the leader's log
- Send log updates to followers
- Handle cases where a follower crashes or becomes unreachable
- Periodically send **heartbeat messages** to followers
- Commit log entries once a majority of followers acknowledge them

---

# Follower State

Followers maintain replicated logs and monitor the leader.

### Tasks performed by the follower

- Read input from `STDIN`
- Forward input messages to the leader
- Receive and append log updates from the leader
- Periodically check for the leader's heartbeat
- If heartbeat fails, declare an election
- Participate in voting during leader election
- Print their own log to `stdout` when requested

---

# Communication Model

All processes communicate **indirectly through a proxy server**.  
No process sends messages directly to another process.

```
Process A → Proxy Server → Process B
```

The proxy is responsible for forwarding messages while optionally simulating unreliable network conditions such as:

- Message delays
- Message loss
- Message reordering
- Temporary disconnections

This allows testing the robustness of the replicated log system under unreliable network conditions.

---

# Protocols

## Heartbeat Protocol

The **Heartbeat Protocol** ensures that followers know the leader is still active and prevents unnecessary leader elections.

### Participants

- Leader
- Followers

### Message Flow

```
Leader → Proxy → Followers
Followers → Proxy → Leader
```

### Protocol Steps

1. The leader periodically sends a `HEARTBEAT` message to the proxy.
2. The proxy forwards the heartbeat to all followers.
3. When a follower receives the heartbeat:
   - It resets its heartbeat timeout timer.
   - It sends a `HEARTBEAT_ACK` message to the proxy.
4. The proxy forwards the acknowledgement to the leader.
5. If a follower does not receive a heartbeat within the timeout period:
   - It assumes the leader has failed.
   - It initiates the **Election Protocol**.

---

## Election Protocol

The **Election Protocol** selects a new leader if the current leader fails.

### Participants

- Candidate
- Followers
- Proxy Server

### Message Flow

```
Candidate → Proxy → All Other Nodes
Nodes → Proxy → Candidate
```

### Protocol Steps

1. A follower detects a heartbeat timeout.
2. The follower becomes a **candidate** and increments its election term.
3. The candidate sends a `VOTE_REQUEST` message to the proxy.
4. The proxy forwards the request to all other nodes.
5. When a node receives a `VOTE_REQUEST`, it grants a vote if:
   - It has not already voted in the current term.
   - The candidate’s log is at least as up-to-date as its own.
6. The node sends a `VOTE_RESPONSE` message to the proxy.
7. The proxy forwards responses to the candidate.
8. If the candidate receives votes from a **majority of nodes**, it becomes the new **leader**.
9. The new leader immediately begins sending **heartbeat messages** through the proxy.

---

## Log Update Protocol

The **Log Update Protocol** ensures that all processes maintain a consistent and totally ordered log.

### Steps

1. A follower receives input from `stdin`.
2. The follower sends a **log update request** to the leader through the proxy.

```
Follower → Proxy → Leader
```

3. The leader receives the message and appends it to its local log.
4. The leader sends an `APPEND_ENTRY` message to all followers through the proxy.
5. Followers append the entry to their local logs and send an acknowledgment.

```
Follower → Proxy → Leader (ACK)
```

6. Once the leader receives acknowledgments from a **majority of followers**, the entry becomes committed.
7. The leader sends a `COMMIT_ENTRY` message to all followers.
8. Followers mark the entry as committed in their logs.

---

## Follower Safety

Followers enforce rules to maintain **log consistency**.

- Followers only append log entries received from the current leader.
- Followers reject log updates if the previous log index does not match their log.
- If inconsistencies occur, the follower requests missing log entries from the leader.
- Followers update their logs to match the leader’s committed log.

These rules ensure that:

- Logs remain **totally ordered**
- Logs across processes are either **identical or prefix-consistent**
- The system remains consistent even during failures or network disruptions.

---

# Summary

This replicated log system ensures:

- **Consistent ordering of messages**
- **Fault tolerance to process crashes**
- **Leader election and recovery**
- **Robustness under unreliable network conditions**

The use of a **proxy server** allows realistic testing of distributed system behavior under unreliable communication environments.
