# Implementation of Replicated Log
## Replicated State Machines
System in which a group of processes maintain a totally-ordered log of the messages that each process is receiving from stdin.

# Process States
Each Process has a default state that contains the log with empty initialized values for the case a follower is ellected to leader

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

# Protocols
## Heartbeat Protocol
The Heartbeat Protocol ensures that followers know the leader is still active and prevents unnecessary leader elections.
### Participants
- Leader
- Follower
### Messages
- 
- The leader periodically sends heartbeat messages to all followers.
- Followers reset their heartbeat timeout timer when receiving a heartbeat.
- If a follower does not receive a heartbeat within a timeout period, it assumes the leader has failed.
- 
- Winner of Election Changes sate to Leader
- New Leader stoers general Log
  
## Election

## Log Update
- Follower Receives Input
- Sends Update Signal to Leader
- Leader Acks and Sends "Commit" Signal to Followers
- Leader Sends Updated Log to all followers

## Follower Safety
  
