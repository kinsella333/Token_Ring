# final_proj

<h2>Run Instructions</h2>
<p>To run simple 3 node chain run ./init.sh</p>
<p>To run n node chain run ./initMulti.sh n where n is number of nodes to run</p>
<p>To run each node individually run ./node (Client Port) (Server Port)</p>
<p>The first node can have any client port value that is available and the server port should be the port you would like the final client node to have. The subsequent nodes
after the first should match server port to the previous node's client port.</p>
