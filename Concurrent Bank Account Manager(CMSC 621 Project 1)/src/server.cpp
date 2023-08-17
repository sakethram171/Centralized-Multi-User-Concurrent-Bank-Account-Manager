#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <fstream>
#include <vector>
#include <mutex>

using namespace std;

#define MAX 99999
#define PORT 8000


int accountsCreated =0;
int threadCount = 0;

//globalCount to calculate interest based on no of transactions.
int globalCount = 0;
std::mutex mx;

//stream object for logging
fstream logfile;


//Class to store the account information like account number, acount balance, account holder name.
class Account
{
public:
	int acctNumber, acctBalance;
	string acctHolderName;
};

//Class to store the Trasactions.txt information 
class Transactions
{
public:
	int transAcct, transAmt;
	string transType;
};

Account ClientAcct[100];

void calculateInterest(int globalC)
{
	
	//Opening a log file to log all the events
	logfile.open("./logfile.txt", ofstream::app);
	
	// This function is used to add interest to the client's accounts
	//0.01% interest is added for every 120 transactions. we can change this interval accordingly.
	//This will compulsorily add interest once at first trasaction. we can also modify this
	if ((globalC%120)==0)
	{
	mx.lock();
	
	int totalAccounts = accountsCreated;
	cout<<"\n------------Adding Interest to accounts for every 1000 trasactions-------------------\n"<<endl;
    logfile<<"\n------------Adding Interest to accounts for every 1000 trasactions-------------------\n";
	
	
	for(int y = 0; y<totalAccounts; y++)
		{
			ClientAcct[y].acctBalance = ClientAcct[y].acctBalance*1.1;
			cout<<"10% Interest added to account "<<ClientAcct[y].acctNumber<<". Updated balance  is "<<ClientAcct[y].acctBalance<<"USD"<<endl;
			logfile<<"10% Interest added to account "<<ClientAcct[y].acctNumber<<". Updated balance is "<<ClientAcct[y].acctBalance<<"USD"<<endl;
	
		}
	cout<<"---------------------------------------------------------------------------------------------------------------"<<endl;
	mx.unlock();
	}
	
	logfile.close();
}



//Every time  a new thread gets created , it executes the below function
void * threadFunction (void * connfd)
{
	//Opening a log file to log all the events
	logfile.open("./logfile.txt", ofstream::app);
	
	//initializing all the required variables
	int totalAccounts = accountsCreated;
	int validAcctNumber = 0;
	int rd;
	char readBuff[1024]={0};
	string str;
	Transactions trans;	
	std::string delimiter = " ";
	size_t pos = 0;
	std::string word;
	vector <string> vec;
		
	
	//logging
	printf("\nWaiting for the request from client\n");
	logfile<<"\nWaiting for the request from client\n";
	
	
	//reading each transaction sent by the client into readBuff
	while(rd = read( (long)connfd , readBuff, 1024) > 0)
	{
		cout<<"\nReceived transcation from client "<<threadCount<<":"<<readBuff<<endl;
		
		//logging
		logfile<<"\nReceived transcation from client "<<threadCount<<":"<<readBuff<<endl;
		
		//storing as string to split into words
		str = string(readBuff);
		
		//storing individual words into a vector
		while ((pos = str.find(delimiter)) != std::string::npos)
		{
			word = str.substr(0, pos);
			vec.push_back(word);
			str.erase(0, pos + delimiter.length());
		}

		//copying transaction details from vector to an object
		trans.transAcct = atoi(vec.at(0).c_str());
		trans.transType = vec.at(1);
		trans.transAmt = atoi(vec.at(2).c_str());
		
		//clearing buffer after storing each transaction
		readBuff[1024]={0};
		
		//locking the critical section to not allow other clients/threads modify account information when one thread is modifying
		mx.lock();
		
		for(int itr= 0; itr<totalAccounts; itr++)
		{
			//Checking if the account from Transactions.txt exists. If yes, then we update the balance based on the transaction type.
			if(trans.transAcct == ClientAcct[itr].acctNumber)
			{
				validAcctNumber = 1;
					if(trans.transType == "d")
					{
						ClientAcct[itr].acctBalance = ClientAcct[itr].acctBalance+trans.transAmt;
						cout<<trans.transAmt<<" USD credited to account number: "<<ClientAcct[itr].acctNumber<<". Final account balance is "<<ClientAcct[itr].acctBalance<<" USD"<<endl;
						
						//logging status of deposit
						logfile<<"SUCCESS: "<<trans.transAmt<<" USD successfully credited to account number: "<<ClientAcct[itr].acctNumber<<". updated account balance is "<<ClientAcct[itr].acctBalance<<" USD"<<endl;
					}

					 if (trans.transType == "w") 
					 {
						if (trans.transAmt > ClientAcct[itr].acctBalance)
						{
							cout<<"Insufficient balance, please try amount lesser than your account balance."<<" Current account balance is:"<<ClientAcct[itr].acctBalance<<" USD"<<endl;
							
							//logging status of withdrawal
							logfile<<"Transaction FAILED: "<<"Insufficient balance"<<" Current account balance is:"<<ClientAcct[itr].acctBalance<<" USD"<<endl;
						}
						else
						{
							ClientAcct[itr].acctBalance  = ClientAcct[itr].acctBalance - trans.transAmt;
							cout<<trans.transAmt<<" USD debited from account number: "<<ClientAcct[itr].acctNumber<<". Final account balance is "<<ClientAcct[itr].acctBalance<<" USD"<<endl;
							
							//logging status of withdrawal
							logfile<<"SUCCESS: "<<trans.transAmt<<" USD successfully debited from account number: "<<ClientAcct[itr].acctNumber<<". Updated account balance is "<<ClientAcct[itr].acctBalance<<" USD"<<endl;
						}
					 }
				
			}
		//unlocking the critical section after updating the account info
		mx.unlock();
				
			
		}
		//if given account is not found in the accounts information, notify user
		if (!validAcctNumber)
		{
			cout<<"\nPlease enter a valid account number"<<endl;
			logfile<<"\nPlease enter a valid account number"<<endl;
		}
		
		//clearing vector vec to remove previous transaction details
		vec.clear();
		
		//calculating interest per 1000 transactions
		calculateInterest(globalCount++);
		
			
	}
	
		
	logfile.close();
		
	return 0;
}





void loadNewRecords()
{
	// Loads and creates new accounts given in the Records.txt file.
	fstream recordsfile;
	int id = 0;
	
	//Opening a log file to log all the events
	logfile.open("./logfile.txt", ofstream::app);
	
	//creating fstream on Records.txt file to read and store the account information
	recordsfile.open("./Records.txt");
	
	if(recordsfile.is_open())
	{
		
		//read each line from Records.txt file and store them in Account class object
		while( recordsfile >>ClientAcct[id].acctNumber>>ClientAcct[id].acctHolderName>>ClientAcct[id].acctBalance)
		{
			cout<<"Account:"<<ClientAcct[id].acctNumber<<" created for user:"<<ClientAcct[id].acctHolderName<<" with balance:"<<ClientAcct[id].acctBalance<<endl;
			
			logfile<<"Account:"<<ClientAcct[id].acctNumber<<" creation successful for user: "<<ClientAcct[id].acctHolderName<<" with balance:"<<ClientAcct[id].acctBalance<<endl;
			id++;
			accountsCreated++;
	    }
		
	}
	
	recordsfile.close();
	
	//closing logfile 
	logfile.close();

}




int main()
{

	int optvalue=1;
	int sockfd, connfd, len;
	struct sockaddr_in serveradd, client;
	pthread_t threads[99999];
	
	//Function to read data from Records.txt file and store it as individual accounts
	loadNewRecords();

	// creating and holding a stream socket file descriptor
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	//Verifiying if socket is created successfully or not
	if (sockfd == -1)
	{
		cout<<"Failed to create a socket\n";
		exit(0);
	}
	
	//Set all bytes to ZERO
	bzero(&serveradd, sizeof(serveradd));


    //sets options associated with a socket.
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optvalue, sizeof(optvalue)))
	    {
	        cout<<"\n Error in setsockopt"<<endl;
	        exit(0);
	    }
		
	//assigning family, address and port to serveradd
	serveradd.sin_family = AF_INET;
	serveradd.sin_addr.s_addr = htonl(INADDR_ANY);
	serveradd.sin_port = htons(PORT);

	// Binding the  address to the socket
	int val;
	val = bind(sockfd, (struct sockaddr *) &serveradd, sizeof(serveradd));
	
	//verifying socket bind
	if (val!= 0)
	{
		cout<<"\nFailed to bind the socket"<<endl;
		exit(0);
	}
	else
		cout<<"Socket successfully binded..\n";

	// Listen on the socket
	val = (listen(sockfd, 5));
	if (val != 0) {
		cout<<"\nListening failed!\n";
		exit(0);
	}
	else
		cout<<"\nServer listening now..\n";
	

	//accept incoming connections till forever
	while(1)
	{
		
		len = sizeof(client);
		cout<<"\nAccepting after listen\n";
		connfd = accept(sockfd, (struct sockaddr *)&client,(socklen_t*) &len);
		
		//creating new thread whenever a client wants to connect to a server and make transactions
		//every newly created thread will perform the tasks mentioned in the threadFunction
		pthread_create(&threads[threadCount],0,threadFunction,(void*) (long) connfd);
		
		//pthread_join(threads[threadCount],NULL);
		cout<<"\nClient "<<threadCount<<" connected!"<<endl;
		threadCount++;
	}
	
	if(connfd<0)
		cout<<"\nAccept Failed!"<<endl;
	else
		cout<<"\nAccept Sucess"<<endl;
	

	//closing the newly created socket
	close(sockfd);
}

