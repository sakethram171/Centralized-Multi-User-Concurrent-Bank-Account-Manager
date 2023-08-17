**<h1>Centralized Multi-User Concurrent Bank Account Manager</h1>**


<h2>Project goal :</h2>
The project aims to create a centralized bank server for concurrent client transactions. The server has information about the individual user accounts and their account balances.
Multiple clients can connect to the server simultaneously and perform transactions like deposits
and withdrawals. The server accepts such concurrent requests successfully and performs
transactions by maintaining data consistency.

## Server design:
● Server initially stores the user-account information via an input file. The input file
Records.txt will have the new user-account information along with the initial balances.<br>
● Server program then creates a communication socket and listens for incoming
requests.<br>
● Whenever any client sends a transaction to perform, the server will process each
transaction and update the balance associated with the requested account based on the
transaction type. Similarly, all the transactions from that particular client are processed
individually.<br>
● For each client, a new thread is created and executes all the transactions related
to that client.<br>
● Upon executing transactions from one client, it again listens on the socket for any other
incoming connections for clients to accept. Therefore, each client is handled one by one.<br>
● The server also runs a program that calculates interest on account balances for all the
accounts and then adds the interest to the existing balance. Currently, the server adds
0.01% interest for every 120 transactions(assuming it takes 60 seconds to perform 120
transactions). We can further modify this by considering proper timestamps.<br>
● All connection and error messages are logged in the logfile.txt file in the
project folder.<br>

## Client design:
● Client program initially creates a socket for communicating with the server.<br>
● Once connected with the server, it then sends individual transactions to the server for
execution.<br>
● The required transactions can be provided in the Transactions.txt file, which the
client program reads and sends across the server.<br>
● Though the input file has multiple transactions, the client will send one transaction at a
time for execution.<br>
