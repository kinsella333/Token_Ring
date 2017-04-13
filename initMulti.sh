#!/bin/bash

NODE_COUNT=$1
i=0
PERCENTAGE=100
PORT1=5000
PORT2=5000
PORT3=5000
DIF=1

tmux new-session -d -s node

let DIF=NODE_COUNT
let NODE_COUNT=NODE_COUNT-1
let PERCENTAGE=PERCENTAGE-100/NODE_COUNT
let PORT2=PORT2+NODE_COUNT
let PORT3=PORT2

while [ $i -lt $NODE_COUNT ]; do
  tmux send-keys -t 'node':0 './node ' "$PORT1" ' ' "$PORT2" Enter
  sleep 1
  tmux splitw -t 'node':0 -v -p $PERCENTAGE
  let PORT2=PORT1
  let PORT1=PORT1+1
  let i=i+1
  let DIF=DIF-1
  let PERCENTAGE=100-100/DIF
done

let PORT1=PORT1-1
tmux send-keys -t 'node':0 './node ' "$PORT3" ' ' "$PORT1" ' ' Enter
tmux attach -t node
