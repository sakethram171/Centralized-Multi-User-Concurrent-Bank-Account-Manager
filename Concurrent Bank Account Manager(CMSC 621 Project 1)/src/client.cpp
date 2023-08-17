#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <arpa/inet.h>
#include <vector>
#include <cstring>
#define PORT 8000
using namespace std;

//stream object for logging
fstream logfile;

void sendTransactionsToServer(int sockfd, char (&writeBuff)[])
{
	//This function sends the transactions from Transactions.txt to the server for processing
	
	
	//Variable to store transaction account, transaction amount, transaction Time Stamp
	int transAcct, transAmt, transTS;
	string transType;
	
	//fsteam objects for opening and writing into files.
	fstream transactionsfile;

	
	transactionsfile.open("./Transactions.txt");
	
	//Opening a log file to log all the events
	logfile.open("./logfile.txt", ofstream::app);
	
	//Opening Transactions.txt file to read the set of transactions and sending it one by one(line by line) to client
	if(transactionsfile.is_open())
	{
			//reading until Transactions.txt is empty
			while( transactionsfile >>transTS>>transAcct>>transType>>transAmt)
	    	{
			//Appending all the values in individual strings to send it to server.
	    	string transaction= to_string(transAcct) + " " + transType + " " + to_string(transAmt) + " ";
			
			//logging each transaction
	    	logfile <<transaction<<endl;
			
			//copying character array to string to send it via send() function -- send() only accepts string messages to pass.
	    	strcpy(writeBuff,transaction.c_str());
			cout<<"\nsending trans : "<<transaction<<endl;
			
			//logging
			logfile<<"\nFrom Client : sending trans : "<<transaction<<" to server"<<endl;;
			
			//sending a transaction to a file
	    	send(sockfd,writeBuff,strlen(writeBuff),0);
			
			cout<<"\nsent :"<<transaction<<endl;
			
			//logging
			logfile<<"\nFrom Client : Transaction :"<<transaction<<" sent to server"<<endl;;
			
	    	sleep(1);
	    	}
	 }
	 
	 //closing fstream objects
	 logfile.close();
	 transactionsfile.close();
}

int main()
{
	
	//initializing variables
	int optvalue =1;
	char writeBuff[1024];
	int sockfd, connfd; 
	struct sockaddr_in serveradd, client;
	
	//Opening a log file to log all the events
	logfile.open("./logfile.txt", ofstream::app);
	
	//creating and holding a stream socket file descriptor
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	//sets options associated with a socket.
	setsockopt((sockfd), SOL_SOCKET, SO_REUSEPORT, &optvalue, sizeof(optvalue));
	
	//Verifiying if socket is created successfully or not
	if (sockfd == -1)
	{
		cout<<"Failed to create a socket\n";
		exit(0);
	}

	//Set all bytes to ZERO
	bzero(&serveradd, sizeof(serveradd));

	//assigning family, address and port to serveradd
	serveradd.sin_family = AF_INET;
	//assigning local IP address
	serveradd.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	serveradd.sin_port = htons(PORT);

	// connecting the client socket to server socket
	connfd = connect(sockfd, (struct sockaddr *) &serveradd, sizeof(serveradd));
	
	//checking if connection was succeeded
	if (connfd!= 0)
	{
		cout<<"Connection with the server failed.\n";
		exit(0);
	}
	else
	{
		cout<<"Wohoooo! Connected to the server \n";
	}
	
	//function to  send each transactions to server individually
	sendTransactionsToServer(sockfd,writeBuff);
	
	//closing logstream object
	 logfile.close();

	 //closing the socket
	 close(sockfd);
	    
	    return 0;
}

