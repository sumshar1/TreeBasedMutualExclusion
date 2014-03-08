TreeBasedMutualExclusion
========================


Problem summary

This is an project to be done in groups of two students and you are expected to demonstrate its operation either to
the instructor or the TA.
You are required to implement the following tree-based voting protocol for replica consistency. Let there be seven
servers, S1; S2; : : : S7, that maintain replicas of data objects. The seven servers are logically arranged as a balanced
binary tree with S1 being the root, and servers S2i and S2i+1 being the left and right children, respectively, of server
Si. There are four data objects, D0; D1; D2; and D3 of type integer. Each server maintains copies of the four data
objects. Initially, all the replicas of an object are consistent, with Dj initialized to j, where 0  j  3. There are five
clients, C0; C1; : : : ;C4. All communication channels are FIFO.
The protocol for data access is as follows:
1. A client, Ci can perform the following operations on a data object Dj :
 Read
 Write
For a read operation, client Ci sends a request of the form REQUEST(Ci, Dj ) to a randomly chosen server. For
a write operation, client Ci sends a request of the form REQUEST(Ci, Dj , v) to all seven servers in the logical
tree. Having sent a request, the client starts an AWAITING GRANT timer. The purpose of the read operation is
to retrieve the value of Dj , while the purpose of the write operation is to add v to the current value of Dj , where
v is an integer. The value of the AWAITING GRANT timer is equal to 20 time units.
2. A server maintains the following LOCK information about each data object:
 NOT LOCKED: the object has not been locked by any client,
 READ LOCKED: a read request for this object has been granted to at least one client, and
 WRITE LOCKED: a write request for this object has been granted to client.
Initially, state of each LOCK is set to NOT LOCKED. A READ LOCK COUNT is also associated with each
LOCK and is initialized to 0. Here is how the server responds to requests from client Ci for a given data object:
 If data object’s lock is in the NOT LOCKED state and a READ request is received: the request is granted,
lock is set to READ LOCKED and READ LOCK COUNT is set to 1.
 If data object’s lock is in the NOT LOCKED state and a write request is received: the request is granted,
and lock is set to WRITE LOCKED.
 If data object’s lock is in the READ LOCKED state and a READ request is received: the request is granted,
lock stays set to READ LOCKED and READ LOCK COUNT is increased by 1.
 If data object’s lock is in the READ LOCKED state and a WRITE request is received: the request is not
granted immediately. Instead, the request is placed in a FIFO queue corresponding to the data object, lock
stays set to READ LOCKED.
 If data object’s lock is in the WRITE LOCKED state and a READ request is received: the request is not
granted immediately. Instead, the request is placed in a FIFO queue corresponding to the data object, lock
stays set to WRITE LOCKED.

 If data object’s lock is in the WRITE LOCKED state and a WRITE request is received: the request is not
granted immediately. Instead, the request is placed in a FIFO queue corresponding to the data object, lock
stays set to WRITE LOCKED.
Whenever a READ request is granted by a server, the server sends the value of the corresponding data object in
its grant message.
3. If client Ci’s READ request has been granted by the requested server before the expiry of its AWAITING GRANT
timer then the client does the following: (i) first Ci waits for a period of time referred to as the HOLD TIME,
then (ii) client Ci sends a READ COMMIT message to the granting server, and (iii) client Ci locally records
the value received from the server. The HOLD TIME is equal to 1 time unit.
4. If client Ci’s WRITE request has been granted by the tree of servers before the expiry of its AWAITING GRANT
timer then the client does the following: (i) first Ci waits for a period of time referred to as the HOLD TIME,
then (ii) client Ci sends a WRITE COMMIT message to all servers. The HOLD TIME is equal to 1 time unit.
Granting of the WRITE request by the tree is recursively defined as:
(a) The WRITE request has been granted by the root of the tree, and either the left or the right subtree, OR
(b) The WRITE request has been granted by the left subtree and the right subtree.
If a subtree has only one server, then the granting of WRITE request by that subtree is equivalent to obtaining a
grant from that server.
5. On receiving the WRITE COMMIT message from Ci, all the servers perform the access operation indicated in
the corresponding REQUEST message, send an acknowledgement to Ci. If the server was WRITE LOCKED
by Ci’s request then the server gets unlocked on performing the data update operation, i:e:, the LOCK is set
to NOT LOCKED. Now, it can grant the request at the head of the queue of pending requests for the newly
updated object.
6. On receiving the READ COMMIT message from Ci for a data object, the server decreases the corresponding
READ LOCK COUNT by 1. If, as a result of this decrease, READ LOCK COUNT becomes zero, the LOCK
for that data object is set to NOT LOCKED and the server can grant the request at the head of the queue of
pending requests for the object..
7. If a requesting client’s AWAITING GRANT timer expires before it receives WRITE permission from the tree,
or a READ permission for a specific server then the client withdraws its request by sending a corresponding
WITHDRAW message to all servers in case of a WRITE, or to the requested server in case of a READ, and
increments the number of unsuccessful read/write accesses by one. The variable to store the number of unsuccessful
read accesses and unsuccessful write accesses are maintained locally at each client, and are initialized to
zero.
8. On receiving a WITHDRAWmessage from Ci, servers perform the same operation as on receiving the READ COMMIT
or WRITE COMMIT message, except for performing the data access operation.
If you believe that this protocol may result in writes to a data object being performed at different serves in different
order, add safeguards to prevent such a possibility. You need to instrumentyour code such that if these safeguards get
triggered, a corresponding message is displayed on the screen.
1 Operation
1. A client can have at most one pending access request at a time. The time between the completion (successful
or unsuccessful) of the previous access and the next access attempted by a client is uniformly distributed in the
range [5,10] time units. Use the same distribution for the initial access. When a client wishes to perform a
READ or WRITE operation it arbitrarily selects one of the five data objects for the operation and initiates the
protocol as described above. To determine the type of operation, the client randomly generates a number in
the range 1 through 10 (inclusive). If the number is equal to 1, the requested operation is a WRITE operation.
Otherwise, the requested operation is a READ operation.
2. In your experiments all communication should be performed using IP stream sockets.
3. Execute your experiment until a total of 50 updates have been attempted.
4. For the experiment report the following:
(a) For every data object, do all replicas of the object go through exactly the same sequence of updates?
(b) The number of successful and unsuccessful READ and WRITE accesses by each client.
(c) The total number of messages exchanged.
(d) For the successful READ accesses, the minimum, maximum, and average time between issuing a READ
request and receiving permission from the requested server.
(e) For the successful WRITE accesses, the minimum, maximum, and average time between issuing a WRITE
request and receiving permission from the server tree.
5. Repeat the experiment with the HOLD TIME set to 0:1, 0:5, 1:5, 2:0, and 5:0 time units.
6. What is the impact, if any, of the value of HOLD TIME on the performance of the protocol?
