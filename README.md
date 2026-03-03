# Distance Vector Network Simulator

This project implements a simple **Distance Vector (DV) routing protocol simulator** in C.  
It allows multiple router processes to exchange routing information and dynamically update their routing tables.

---

## Running the Simulator

The simulator is executed via the command line with the following format:

```bash
./d_vec <self_port> <neighbour1_port> <cost1> [<neighbour2_port> <cost2> ...]