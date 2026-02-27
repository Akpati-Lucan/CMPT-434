#!/bin/bash

# Run Makefile which is in the same folder
make

# Ports for the test
ROUTER1_PORT=30001
ROUTER2_PORT=30002
ROUTER3_PORT=30003

# Start Router 1 (neighbor 2 and 3)
./d_vec $ROUTER1_PORT $ROUTER2_PORT 1 $ROUTER3_PORT 2 &
PID1=$!

# Start Router 2 (neighbor 1 and 3)
./d_vec $ROUTER2_PORT $ROUTER1_PORT 1 $ROUTER3_PORT 1 &
PID2=$!

# Start Router 3 (neighbor 1 and 2)
./d_vec $ROUTER3_PORT $ROUTER1_PORT 2 $ROUTER2_PORT 1 &
PID3=$!

echo "Routers started. PIDs: $PID1, $PID2, $PID3"
echo "Press Ctrl+C to stop the test"

# Wait indefinitely
wait

#./d_vec 30001 30002 2 30003 3