#!/bin/bash

tmux new-session -d -s node
tmux send-keys -t 'node':0 './node 5000 5002' Enter
sleep 1
tmux splitw -t 'node':0 -v -p 67
tmux send-keys -t 'node':0 './node 5001 5000' Enter
sleep 1
tmux splitw -t 'node':0 -v -p 50
tmux send-keys -t 'node':0 './node 5002 5001' Enter
tmux attach -t node
