//Header files//
#include<fstream>
#include<iostream>
#include<sstream>
#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<list>
#include<ctime>
#include<string>
#include<string.h>
#include<time.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include<map>
#include<queue>
#include<cstdatomic>
#include<algorithm>
#include<math.h>
#include<thread>
#include<error.h>
#include<ctime> 
//port to used for communication
#define PORT "50001"
//maping global data

#define PORT1 "50000"
volatile bool update=false;
float TIME_UNIT=100000.0;
volatile bool timer=false;

volatile bool exitt=false;

volatile bool witdraw=false;
//breakup strings are filled when we get data in readreceive write receive thread

std::string breakup[6];

std::string breakup1[6];
// (1) list array to have sets
std::list<int> sets[15];
std::list<int> setscopy[15];

//reader is true, when read is granted or else it is false
bool reader=false;

//writer is true, when the write permission is granted or else it is false
bool writer=false;

//write was successful on servers
bool saved=false;

//(2) map to store names of the server in the tree
std::map<int, std::string> servers;
std::map<int, std::string>::iterator servers_it;


//global list for all seven values received
std::list<int> value_received;
std::list<int>::iterator itr;
//creating lists for generating sets.
//as a client first part would be make lists for all the sets of the quorum, we can form form the given logical tree.

void CreateLists()
{
std::ifstream input("set.in");
  try
  {
     if(!input) 
       throw 1;
  }
    catch (int i)
    {
      std::cout<<"there was exception while opening the file set.in"<<std::endl;
      exit (0);
    }
//since we know, there are 15 sets, we need to read the for first 15 lines
int linecount(0);

 while(linecount!=15)
  {
    int element1(0), element2(0), element3(0), element4(0);
       if(linecount < 4)
         {
          input>>element1 >> element2 >> element3;
           sets[linecount].push_back(element1);
           sets[linecount].push_back(element2);
           sets[linecount].push_back(element3);
           linecount++;
         }
        else
          {
           input>>element1 >> element2 >> element3 >> element4;
           sets[linecount].push_back(element1);
           sets[linecount].push_back(element2);
           sets[linecount].push_back(element3);
           sets[linecount].push_back(element4);
           linecount++;
          } 
  }
/*
//let us see how it gets populated
std::list<int>::iterator it;
for(int i=0; i<15; i++)
 { 
  it=sets[i].begin();
     while(it!=sets[i].end())
        {  std::cout<< *it<<"   ";
           it++;   
         }
  std::cout<<std::endl; 
  }*/
}

void ClearSets()
{
for(int i=0;i<15;i++)
  {
    setscopy[i].clear();
  }
}

void CopySets()
{
std::list<int>::iterator it1;
std::list<int>::iterator it2;
  for(int i=0; i<15; i++)
   {
    it1=sets[i].begin();
      while(it1!=sets[i].end())
       {
        setscopy[i].push_back(*it1);
        it1++;
       }  
   }
/*
for(int i=0; i<15; i++)
 { 
  it2=setscopy[i].begin();
     while(it2!=setscopy[i].end())
        {  std::cout<< *it2<<"   ";//
           it2++;   
         }
  std::cout<<std::endl; 
  }
*/
}

//making the map for the servers.
void makemap()
 {
   std::string servernames[7]= { "net01.utdallas.edu", "net02.utdallas.edu", "net03.utdallas.edu", "net04.utdallas.edu", "net05.utdallas.edu", "net06.utdallas.edu", "net07.utdallas.edu" }; 
   for(int adding=0; adding <7 ; adding++)
       servers.insert(make_pair(adding+1, servernames[adding]));
 }


void dummymsg();


//time function to make receive thread work for just 20 time units.
void timee()
{
usleep(TIME_UNIT * 20);
timer=true;
if(timer == true)
{
std::thread dummy(dummymsg);
dummy.join();
}
}
// get1 sockaddr, IPv4 or IPv6:
void *get1_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
 
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//making the request to the servers
void SendRequest( char *reqms, int serverid)
{std::cout<<"sending request to server "<<serverid<<std::endl; 
   //iterate the map to find the server name
    std::string servername;
    servers_it=servers.begin();
        while(servers_it!=servers.end())
           {
             if(servers_it->first == serverid)
                { servername=servers_it->second; } 
              servers_it++;
           }
    char *messageTo=new char[servername.length()];
    servername.copy(messageTo, servername.length());
    messageTo[servername.length()]='\0';
 
    int sockfd, numbytes; 
    struct addrinfo hints1, *servinf, *q;
    int rv;
    char s[INET6_ADDRSTRLEN];
 
    memset(&hints1, 0, sizeof hints1);
    hints1.ai_family = AF_UNSPEC;
    hints1.ai_socktype = SOCK_STREAM;
 
    if ((rv = getaddrinfo(messageTo, PORT1, &hints1, &servinf)) != 0) {
       std::cout<<"\nerror getting the addr info";
       return;
    }
 
    // loop through all the results and connect to the first we can
    for(q = servinf; q != NULL; q = q->ai_next) {
        if ((sockfd = socket(q->ai_family, q->ai_socktype,
                q->ai_protocol)) == -1) {
               std::cout<<"client: socket";
            continue;
        }
 
        if (connect(sockfd, q->ai_addr, q->ai_addrlen) == -1) {
            close(sockfd);
            std::cout<<"client: connect";
            continue;
        }
 
        break;
    }
 
    if (q == NULL) {
        std::cout<<"stderr,client: failed to connect\n";
        return;
    }
 
    inet_ntop(q->ai_family, get1_in_addr((struct sockaddr *)q->ai_addr), s, sizeof s);
   printf("client: connecting to %s\n", s);
   
    freeaddrinfo(servinf); // all done with this structure
    
     if((numbytes=send(sockfd,reqms,sizeof(reqms),0)) == -1)
   { std::cout<<"there was an error on forwarding the request to the other server"<<messageTo<<std::endl;
     return;  }   
 
 bzero(reqms,0);
 
    close(sockfd);
 
    return;
}





//this function is executed when the client expects to recive messages from the server for the read request it has issued.
void ReadReceiver()
{
std::cout<<"the read receiver thread started for 20 time units"<<std::endl;
//let us setup the receving socket
     int sockfdq, newsockfdq, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     sockfdq = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfdq < 0)
        std::cout<<"ERROR opening socket";
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 50001;//atoi(argv[2]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
              std::cout<<"ERROR on binding";
     listen(sockfdq,50);
    
 
//always listening on the port to read the messages
       while(timer!=true)
       {  
            char *buffer=new char[256];
            clilen = sizeof(cli_addr);
             
              newsockfdq = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
                 if (newsockfdq< 0)
                    std::cout<<"ERROR on accept";
 
                   bzero(buffer,256);
                  
                n = recv(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                 if (n < 0) std::cout<<"ERROR reading from socket";
                  std::cout<<"Message received by the RECEIVE THREAD in the BEGINING(Buffer) is  "<<buffer<<std::endl;
                  
           buffer[n]='\0';
           
           int count(0);
           int index(0);

         for(int i=0;i<6;i++) breakup[i]="";

           while(index != strlen(buffer))//buffer[index]!='\0')
            {
                if(buffer[index] == ' ' || buffer[index] == '\0')
		 { index++; count++; }
               else  { breakup[count] += buffer[index]; index++; }
                
         
            }
std::cout<<"the message received is "<<breakup;

//if message received is yes, then I need to become the master
         if(breakup[0] == "1")
            {//I have got the permission to read the the dataobject and the value is in the string breakup[2]
                reader=true;
              
            }
bzero(buffer,256);

       }
close(newsockfdq);
close(sockfdq);
}


void ReadWriteReceiver()
{
std::cout<<"the read receiver thread started for 20 time units"<<std::endl;
//let us setup the receving socket
     int sockfdq, newsockfdq, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     sockfdq = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfdq < 0)
        std::cout<<"ERROR opening socket";
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 50001;//atoi(argv[2]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
              std::cout<<"ERROR on binding";
     listen(sockfdq,50);
    

 int messagereceived(0);
 while (exitt!=true)
 {
 //std::cout<<std::endl<<"clearing sets"<<std::endl;
 ClearSets();
 //std::cout<<"copying the sets"<<std::endl;


 CopySets();
 

//std::cout<<"message received "<<messagereceived<<std::endl;
if(timer==true) { ; } 
//always listening on the port to read the messages
       while(timer!=true)
       {  
            char *buffer=new char[256];
            clilen = sizeof(cli_addr);
             
              newsockfdq = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
                 if (newsockfdq< 0)
                    std::cout<<"ERROR on accept";
 
                   bzero(buffer,256);
                  
                n = recv(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                 if (n < 0) std::cout<<"ERROR reading from socket";
                  std::cout<<"Message received by the RECEIVE THREAD in the BEGINING(Buffer) is  "<<buffer<<std::endl;
                  
           buffer[n]='\0';
           
           int count(0);
           int index(0);

         for(int i=0;i<6;i++) breakup[i]="";

           while(index != strlen(buffer))//buffer[index]!='\0')
            {
                if(buffer[index] == ' ' || buffer[index] == '\0')
		 { index++; count++; }
               else  { breakup[count] += buffer[index]; index++; }
                
         
            }
std::cout<<"the message received is "<<buffer;

//if message received is yes, then I need to become the master
         if(breakup[0] == "1")
            {//I have got the permission to read the the dataobject and the value is in the string breakup[2]
                reader=true;
				std::ofstream opt("values.in", std::ios::app);
				opt<<"the data object read is "<<breakup[1]<<" and the value read is "<<breakup[2]<<std::endl;
				opt.close();
              
            }
			
			
			
         if(breakup[0] == "2")
            { 
                  messagereceived++;
			std::cout<<std::endl<<"approval recevied from one server "<<std::endl;
                 //we need to remove the server from all the quorum sets we have
                 for(int i=0;i<15;i++)
                     {
		      // std::stringstream srvid(breakup[1]);
		       int ide= atoi(breakup[1].c_str());
		      // srvid>>ide;

                         setscopy[i].remove(ide); 
                     }
               
                //I have got the permission to write on the dataobject from the server  breakup[2]
               // reader=true;
               //if I have got more than 3 messages I need to checkif any of the sets(quorum) have become empty or not
             if(messagereceived >=3)
                {
                 for(int i=0;i<15;i++)
                  {
                     if(setscopy[i].size() == 0) //i have got the permission for write
                          {  writer=true;  messagereceived=0; timer=true; } 
                  }
                }
              
            }
bzero(buffer,256);

       }
}	   
	   
close(newsockfdq);
close(sockfdq);
}


void WithdrawCommitReceiver()
{
std::cout<<"starting the withdrawt receive thread"<<std::endl;
//let us setup the receving socket
     int sockfdq, newsockfdq, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     sockfdq = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfdq < 0)
        std::cout<<"ERROR opening socket";
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 50005;//atoi(argv[2]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
              std::cout<<"ERROR on binding";
     listen(sockfdq,50);
 int counter(0);   
 int counter1(0);
 int messagecount=0; //number of messages received. if I receive 7 messages, that means value has been updated by all seven servers
//always listening on the port to read the messages

while(exitt != true)
{

      // while(counter!=7 || counter1 !=7)
	  if(saved == true || witdraw ==true) { ; } 
	  while(saved!=true || witdraw != true)
       {  
	      n=0;
            char *buffer=new char[256];
            clilen = sizeof(cli_addr);
             
              newsockfdq = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
                 if (newsockfdq< 0)
                    std::cout<<"ERROR on accept";
 
                   bzero(buffer,256);
                  
                n = recv(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                 if (n < 0) std::cout<<"ERROR reading from socket";
                  std::cout<<"Message received by the RECEIVE THREAD in the BEGINING(Buffer) is  "<<buffer<<std::endl;
                  
            
 
           buffer[n]='\0';
           
           int count(0);
           int index(0);
	   for(int i=0;i<6;i++) { breakup1[i]=""; breakup[i]=""; }
           while(index != strlen(buffer))//buffer[index]!='\0')
            {
                if(buffer[index] == ' ' || buffer[index] == '\0')
		 { index++; count++; }
                else { breakup1[count] += buffer[index]; index++; }
                
         
            }
std::cout<<"the message received is "<<buffer;
if(breakup1[0]=="5")  {counter++;
  if(counter==7) { witdraw=true; counter=0; }
}
//std::cout<<"the value of dataobject received is == "<<value_received[count];
//if message received is yes, then I need to become the master
         if(breakup1[0] == "3")
            {
			//std::stringstream a(breakup1[2]);
messagecount++;
int ab=atoi(breakup1[2].c_str());
//a>>ab;
value_received.push_back(ab);
                 //we increase the variable count by 1, if count and messagecount are equal==>>write was successful
                counter1++; 
            
         if(counter1 == 7 || messagecount >= 7)
           { //request saved on the servers.   
		     std::cout<<" the Item is saved on the servers with the value "<<breakup1[2]<<std::endl;
               saved=true;
			counter1=0;messagecount=0;   
              // return ;
           }
          }
bzero(buffer,256);

       }
}
close(newsockfdq);
close(sockfdq);
}



//this is the receive thread that would be executed when the client is waiting for the messages from the server for its write request
void WriteReceiver()
{
//let us first create a copy of sets we have arranged from the logical tree
std::cout<<"the write receiver thread is started "<<std::endl;
CopySets();
//let us make an iterator
std::list<int>::iterator setIT;

//let us setup the receving socket
     int sockfdq, newsockfdq, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     sockfdq = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfdq < 0)
        std::cout<<"ERROR opening socket";
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 50002;//atoi(argv[2]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
              std::cout<<"ERROR on binding";
     listen(sockfdq,50);
    
int messagereceived(0); 

//always listening on the port to read the messages
       while(timer!=true)
       {  
            char *buffer=new char[256];
            clilen = sizeof(cli_addr);
             
              newsockfdq = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
                 if (newsockfdq< 0)
                    std::cout<<"ERROR on accept";
 
                   bzero(buffer,256);
                  
                n = recv(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                 if (n < 0) std::cout<<"ERROR reading from socket";
                  std::cout<<"Message received by the RECEIVE THREAD in the BEGINING(Buffer) is  "<<buffer<<std::endl;
               messagereceived++;
                  
           buffer[n]='\0';
           
           int count(0);
           int index(0);
for(int i=0;i<6;i++) breakup[i]="";
           while(index != strlen(buffer))//buffer[index]!='\0')
            {
                if(buffer[index] == ' ' || buffer[index] == '\0')
		 { index++; count++; }
                else { breakup[count] += buffer[index]; index++; }
                
         
            }
std::cout<<"the message received is "<<breakup;


         if(breakup[0] == "2")
            {
                 //we need to remove the server from all the quorum sets we have
                 for(int i=0;i<15;i++)
                     {
		       std::stringstream srvid(breakup[1]);
		       int ide;
		       srvid>>ide;

                         setscopy[i].remove(ide); 
                     }
               
                //I have got the permission to write on the dataobject from the server  breakup[2]
               // reader=true;
               //if I have got more than 3 messages I need to checkif any of the sets(quorum) have become empty or not
             if(messagereceived >=3)
                {
                 for(int i=0;i<15;i++)
                  {
                     if(setscopy[i].size() == 0) //i have got the permission for write
                          {  writer=true; } 
                  }
                }
              
            }
bzero(buffer,256);

       }
close(newsockfdq);
close(sockfdq);
}




void CommitReceiver()
{
std::cout<<"starting the commit receiver "<<std::endl;
//let us setup the receving socket
     int sockfdq, newsockfdq, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     sockfdq = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfdq < 0)
        std::cout<<"ERROR opening socket";
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 50003;//atoi(argv[2]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
              std::cout<<"ERROR on binding";
     listen(sockfdq,50);
 int count(0);   
 int messagecount=0; //number of messages received. if I receive 7 messages, that means value has been updated by all seven servers
//always listening on the port to read the messages
       while(saved!=true)
       {  
            char *buffer=new char[256];
            clilen = sizeof(cli_addr);
             
              newsockfdq = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
                 if (newsockfdq< 0)
                    std::cout<<"ERROR on accept";
 
                   bzero(buffer,256);
                  
                n = recv(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                 if (n < 0) std::cout<<"ERROR reading from socket";
                  std::cout<<"Message received by the RECEIVE THREAD in the BEGINING(Buffer) is  "<<buffer<<std::endl;
                  
             messagecount++;
 
           buffer[n]='\0';
           
           int count(0);
           int index(0);
	   for(int i=0;i<6;i++) breakup[i]="";
           while(index != strlen(buffer))//buffer[index]!='\0')
            {
                if(buffer[index] == ' ' || buffer[index] == '\0')
		{index++; count++; }
                else { breakup[count] += buffer[index]; index++; }
		
                
         
            }
std::cout<<"the message received is "<<breakup;
std::stringstream a(breakup[2]);
int ab;
a>>ab;
value_received.push_back(ab);
//std::cout<<"the value of dataobject received is == "<<value_received[count];
//if message received is yes, then I need to become the master
         if(breakup[0] == "3")
            {
                 //we increase the variable count by 1, if count and messagecount are equal==>>write was successful
                count++; 
            }
         if(count == 7 && messagecount >= 7)
           { //request saved on the servers.   
               saved=true;
              // return ;
           }
bzero(buffer,256);

       }
close(newsockfdq);
close(sockfdq);
}

void receiveAck()
{
std::cout<<"starting the receive ACK "<<std::endl;
//let us setup the receving socket
     int sockfdq, newsockfdq, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     sockfdq = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfdq < 0)
        std::cout<<"ERROR opening socket";
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 50004;//atoi(argv[2]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
              std::cout<<"ERROR on binding";
     listen(sockfdq,50);
 int count(0);   
 int messagecount=0; //number of messages received. if I receive 7 messages, that means value has been updated by all seven servers
//always listening on the port to read the messages
       while(count!=7)
       {  
            char *buffer=new char[256];
            clilen = sizeof(cli_addr);
             
              newsockfdq = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
                 if (newsockfdq< 0)
                    std::cout<<"ERROR on accept";
 
                   bzero(buffer,256);
                  
                n = recv(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                 if (n < 0) std::cout<<"ERROR reading from socket";
                  std::cout<<"Message received by the RECEIVE THREAD in the BEGINING(Buffer) is  "<<buffer<<std::endl;
                  
            
 
           buffer[n]='\0';
           
           int count(0);
           int index(0);
	   for(int i=0;i<6;i++) breakup[i]="";
           while(index!= strlen(buffer))//buffer[index]!='\0')
            {
                if(buffer[index] == ' ' || buffer[index] == '\0')
		{ index++ ; count++;
		}
		else
                 { breakup[count] += buffer[index]; index++; }
                
         
            }
std::cout<<"the message received is as ACK is "<<breakup;
if(breakup[0] == "4")  count++;
bzero(buffer,256);

       }
close(newsockfdq);
close(sockfdq);
}



void WithdrawReceiver()
{
std::cout<<"starting the withdrawt receive thread"<<std::endl;
//let us setup the receving socket
     int sockfdq, newsockfdq, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     sockfdq = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfdq < 0)
        std::cout<<"ERROR opening socket";
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 50005;//atoi(argv[2]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
              std::cout<<"ERROR on binding";
     listen(sockfdq,50);
 int count(0);   
 int messagecount=0; //number of messages received. if I receive 7 messages, that means value has been updated by all seven servers
//always listening on the port to read the messages
       while(count!=7)
       {  
            char *buffer=new char[256];
            clilen = sizeof(cli_addr);
             
              newsockfdq = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
                 if (newsockfdq< 0)
                    std::cout<<"ERROR on accept";
 
                   bzero(buffer,256);
                  
                n = recv(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                 if (n < 0) std::cout<<"ERROR reading from socket";
                  std::cout<<"Message received by the RECEIVE THREAD in the BEGINING(Buffer) is  "<<buffer<<std::endl;
                  
            
 
           buffer[n]='\0';
           
           int count(0);
           int index(0);
	   for(int i=0;i<6;i++) breakup[i]="";
           while(index != strlen(buffer))//buffer[index]!='\0')
            {
                if(buffer[index] == ' ' || buffer[index] == '\0')
		 { index++; count++; }
                else { breakup[count] += buffer[index]; index++; }
                
         
            }
std::cout<<"the message received is "<<breakup;
if(breakup[0]=="5")  count++;

bzero(buffer,256);

       }
close(newsockfdq);
close(sockfdq);
}

void statuscheck()
{
std::cout<<"starting the status update thread "<<std::endl;
//let us setup the receving socket
     int sockfdq, newsockfdq, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     sockfdq = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfdq < 0)
        std::cout<<"ERROR opening socket";
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 44002;//atoi(argv[2]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
              std::cout<<"ERROR on binding";
     listen(sockfdq,50);
 int count(0);   
 int messagecount=0; //number of messages received. if I receive 7 messages, that means value has been updated by all seven servers
//always listening on the port to read the messages
       while(update !=true)
       {  
            char *buffer=new char[256];
            clilen = sizeof(cli_addr);
             
              newsockfdq = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
                 if (newsockfdq< 0)
                    std::cout<<"ERROR on accept";
 
                   bzero(buffer,256);
                  
                n = recv(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                 if (n < 0) std::cout<<"ERROR reading from socket";
                  std::cout<<"Message received by the RECEIVE THREAD in the BEGINING(Buffer) is  "<<buffer<<std::endl;
                  
            
 
           buffer[n]='\0';
           
           int count(0);
           int index(0);
	   for(int i=0;i<6;i++) breakup[i]="";
           while(index != strlen(buffer))//buffer[index]!='\0')
            {
                if(buffer[index] == ' ' || buffer[index] == '\0')
                 { index++; count++; }
                else { breakup[count] += buffer[index]; index++; }
                
         
            }
std::cout<<"the message received is "<<breakup;
    if(breakup[0]=="9")
         { 
            count++;
           if(count==7)
             {   update =true; //time to exit the computation
exitt=true;         // return ;
             }
         }
bzero(buffer,256);

       }
close(newsockfdq);
close(sockfdq);
}


void receiveAck1()
{
std::cout<<"starting the receive ACK1 thread"<<std::endl;
//let us setup the receving socket
     int sockfdq, newsockfdq, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     sockfdq = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfdq < 0)
        std::cout<<"ERROR opening socket";
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 50006;//atoi(argv[2]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
              std::cout<<"ERROR on binding";
     listen(sockfdq,50);
 int count(0);   
 
       while(count!=1)
       {  
            char *buffer=new char[256];
            clilen = sizeof(cli_addr);
             
              newsockfdq = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
                 if (newsockfdq< 0)
                    std::cout<<"ERROR on accept";
 
                   bzero(buffer,256);
                  
                n = recv(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                 if (n < 0) std::cout<<"ERROR reading from socket";
                  std::cout<<"Message received by the RECEIVE THREAD in the BEGINING(Buffer) is  "<<buffer<<std::endl;
                  
            
 
           buffer[n]='\0';
           
           int count(0);
           int index(0);
	   for(int i=0;i<6;i++) breakup[i]="";
           while(index != strlen(buffer))//buffer[index]!='\0')
            {
                if(buffer[index] == ' ' || buffer[index] == '\0')
		{ index++; count++; }
		else
                 { breakup[count] += buffer[index]; index++; }
                
         
            }
std::cout<<"the message received is as ACK is "<<breakup;
if(breakup[0]=="6")
           count++;

bzero(buffer,256);

       }
close(newsockfdq);
close(sockfdq);

}


void receiveAck2()
{
std::cout<<"starting the receive ACK2 Thread "<<std::endl;
//let us setup the receving socket
     int sockfdq, newsockfdq, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     sockfdq = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfdq < 0)
        std::cout<<"ERROR opening socket";
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 50007;//atoi(argv[2]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
              std::cout<<"ERROR on binding";
     listen(sockfdq,50);
 int count(0);   
 
       while(count!=1)
       {  
            char *buffer=new char[256];
            clilen = sizeof(cli_addr);
             
              newsockfdq = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
                 if (newsockfdq< 0)
                    std::cout<<"ERROR on accept";
 
                   bzero(buffer,256);
                  
                n = recv(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                 if (n < 0) std::cout<<"ERROR reading from socket";
                  std::cout<<"Message received by the RECEIVE THREAD in the BEGINING(Buffer) is  "<<buffer<<std::endl;
                  
            
 
           buffer[n]='\0';
           
           int count(0);
           int index(0);
	   for(int i=0;i<6;i++) breakup[i]="";
           while(index != strlen(buffer))//buffer[index]!='\0')
            {
                if(buffer[index] == ' ' || buffer[index] == '\0')
		{ index++; count++; }
		else
                 { breakup[count] += buffer[index]; index++; }
                
         
            }
std::cout<<"the message received is as ACK is "<<breakup;
if(breakup[0]=="7")
           count++;

bzero(buffer,256);

       }
close(newsockfdq);
close(sockfdq);

}


int main(int argc,char *argv[])
{
//starting read write receiver thread
std::thread RW(ReadWriteReceiver);
std::thread WC(WithdrawCommitReceiver);
int Client_id=atoi(argv[1]);
std::stringstream ss;
ss<<Client_id;
std::string CLid=ss.str();
CreateLists();
makemap();
std::thread check(statuscheck);
int readsuccess(0);
int readfail(0);
int writesuccess(0);
int writefail(0);

 /* initialize random seed: */
  srand (time(NULL));

int totalmessage(0);
//volatile bool update=fasle;

//update will be turned true when 50 updates are perfomed on the servers.
while(update != true)
{
//first we need to select an operation to perform on the server.
//1==>>write  & rest of numbers represent read.
int operation=rand() % 10 + 1;//this will generate numbers from 1 to 10
int object=rand() % 4;   //this is the object number on which the operation would be performed at the server side(from range 0-3)
std::stringstream sd;
sd<<object;
std::string OBJ=sd.str();
   if(operation != 1) //this is read operation
    {
    std::cout<<"operation selected is READ"<<std::endl;
     //the message to be send to the servers
      std::string msg="0 0 " +CLid +" "+OBJ;
      char *message=new char [msg.length()];
      msg.copy(message,msg.length());
      message[msg.length()]='\0'; 
      //randomly select a server to read from
      int serverid=rand() % 7 + 1;
      totalmessage++;
      //send the request
      //std::cout<<"opened Send read request thread "<<std::endl;
	  //std::thread Receive(ReadReceiver);
      std::thread Request(SendRequest, std::ref(message),serverid);
      //std::cout<<"opened reply receive thread for clients READ request"<<std::endl;
      
      //std::cout<<"starting the timer"<<std::endl;
      std::thread timer1(timee);
      Request.join();
      timer1.join();
      //Receive.join(); 
      timer=false;
       totalmessage++;
            if(reader == true) //permission to read is granted
            {
	    std::cout<<"Read Permission Granted "<<std::endl;
             usleep(TIME_UNIT);//wait for the hold_time to take any action
             readsuccess++;
             std::ofstream printing("values.out", std::ios::app);
             if(!printing) std::cout<<"The file is not present 'values.out' "<<std::endl;
               else
                 { // printing<<"the item read is "<< breakup[1]<< " and the value is : "<<breakup[2]; printing.close();
				 }
             reader=false;//making it false again so that readreceive function can modify it again
                  //now I need to send the commit message to the server from where I got the request fulfilled
             std::string output="0 1 "+CLid +" "+OBJ;
             char *message1=new char [output.length()];
             output.copy(message1, output.length());
             message1[output.length()]='\0';
	     std::cout<<"sending the commit message for the READ request"<<std::endl;
		 //std::thread Ack(receiveAck1);
             std::thread commit(SendRequest,std::ref(message1),serverid);
             commit.join();
	     //std::cout<<"opening receive comits reply thread "<<std::endl;
             
            // Ack.join();
           }
             
            else //reader==false and i wasn't permission to read the message, therefore, need to withdraw my read request
             {
	     std::cout<<"the READ request was not granted "<<std::endl;
              usleep(TIME_UNIT);
              readfail++;
              std::ofstream printing("values.in", std::ios::app);
               if(!printing) std::cout<<"The file is not present 'values.out' "<<std::endl;
                 else
                    { printing<<"the read for item " <<breakup[1] <<" failed, as read permission was not granted by server: "<<serverid<<std::endl;
 
                     printing.close(); 
                    } 
             std::string output="0 2 "+CLid +" "+OBJ;
             char *message1=new char [output.length()];
             output.copy(message1, output.length());
             message1[output.length()]='\0';
	     std::cout<<"sending the WithDraw Message for Read "<<std::endl;
		 //std::thread Ack(receiveAck2);
             std::thread withdraw(SendRequest,std::ref(message1),serverid);
             withdraw.join();
	   //  std::cout<<"receive read withdraw ACK"<<std::endl;
             
           //  Ack.join();
           
             }
      
    }

   else if(operation == 1) //this is a write operation
    {
    std::cout<<"WRITE request selected"<<std::endl;
      std::string msg="1 0 " +CLid +" "+OBJ+ " "+"1";
      char *message=new char [msg.length()];
      msg.copy(message,msg.length());
      message[msg.length()]='\0'; 
 //     std::thread Receive; 
                   std::thread timerr;
		   std::cout<<"starting the send request thread for all servers "<<std::endl;
     //I need to send out request to all the servers.
	 // Receive= std::thread(WriteReceiver); 
       for(int server=0; server <7; server ++)
         {   std::thread Request(SendRequest,std::ref(message),(server+1)) ;
             if(server==0)  {   //we can start the receive thread for 20 time units 
	             std::cout<<"Starting the write receive thread "<<std::endl;
                         
                          timerr = std::thread(timee);
                            }
             Request.join(); 
         }
      timerr.join();
      //Receive.join();
      totalmessage+=7;
      timer=false;
 
      if(writer == true) //permission to write is granted
            {
	         totalmessage+=7;
			int toadd=rand() % 9 + 1;
			//convert int to str
			std::stringstream tadd;
			tadd<<toadd;
			std::string adding=tadd.str();
	    std::cout<<"the write request has been grated "<<std::endl;
             usleep(TIME_UNIT);//wait for the hold_time to take any action
            //now need to send the commit message to all the servers
            writer=false;//making it false again so that writereceive function can modify it again
             std::string output="11 "+CLid +" "+OBJ+" "+adding;
             char *message1=new char [output.length()];
             output.copy(message1, output.length());
             message1[output.length()]='\0';
            // std::thread commitwrite;
	       std::cout<<"starting the COMMIT WRITE thread for all servers"<<std::endl;
		  // commitwrite =std::thread(CommitReceiver); 
               for(int server=0; server <7; server ++)
                {
                  std::thread commit1(SendRequest,std::ref(message1),server+1);

                    if(server==0)  { std::cout<<"starting commit receive write thread"<<std::endl; }
                  commit1.join();
                }
				//sleep(1);
            // commitwrite.join();//   ---->>>not sure if we need to add this here, as the fucntion returns 
                while(saved != true)  ;
                      if(saved==true)
                        {  std::cout<<"the UPDATE was successful, the value of data_object is "<<breakup[2]<<std::endl;  
			 writesuccess++;
			 totalmessage+=7;
                          //write performed on the server, the value was received in breakup[2]
                            saved=false; //for future purposes
                           //let us open a file and write the output on the file
                              std::ofstream printing("values.in", std::ios::app);
                                if(!printing) std::cout<<"The file is not present 'values.out' "<<std::endl;
                                  else
                                     { printing<<"The write for item " << object <<"; the value written on servers is "<<breakup1[2]<<std::endl;
 
                                         printing.close(); 
                                     } 
                        }
           
           }
             
            else //writer==false and i wasn't permission to write the message, therefore, need to withdraw my write request
             {
                 writefail++;
              usleep(TIME_UNIT);
              std::ofstream printing("values.out", std::ios::app);
               if(!printing) std::cout<<"The file is not present 'values.out' "<<std::endl;
                 else
                    { printing<<"the read for item " << object <<" failed, as read permission was not granted by servers"<<std::endl;
 
                     printing.close(); 
                    } 
		    std::cout<<"write has failed, need to withdraw the request"<<std::endl;
             std::string output="1 2 "+CLid +" "+OBJ;
             char *message1=new char [output.length()];
             output.copy(message1, output.length());
             message1[output.length()]='\0';
   //          std::thread wdraw;
	     std::cout<<"starting the withdraw message for write request "<<std::endl;
		  //wdraw = std::thread(WithdrawReceiver); 
              for(int server=0; server <7; server ++)
                {
                  std::thread withdraw(SendRequest,std::ref(message1),server+1);
                    if(server==0) {std::cout<<"starting withdraw write receiver "<<std::endl; }
                  withdraw.join();
                }
		totalmessage+=7;
          //  wdraw.join();  
  while(witdraw!=true);
  witdraw=false;  
          }     


        
    }
   else
    { ; } 


std::cout<<"total Failed read attempts "<<readfail<<std::endl;
std::cout<<"total Failed write attempts "<<writefail<<std::endl;
std::cout<<"total Successful read attempts "<<readsuccess<<std::endl;
std::cout<<"total Successful write attempts "<<writesuccess<<std::endl;
std::cout<<"total number of messages "<<totalmessage<<std::endl;

}


return 0;
}
void dummymsg()
{ 
char hostname[40];
if(gethostname(hostname,40) == -1)
   perror("failed to get the name");
   
   char *messageTo=hostname;

   char msg[2]={'f','\0'};
    int sockfd, numbytes; 
    struct addrinfo hints1, *servinf, *q;
    int rv;
    char s[INET6_ADDRSTRLEN];
 
    memset(&hints1, 0, sizeof hints1);
    hints1.ai_family = AF_UNSPEC;
    hints1.ai_socktype = SOCK_STREAM;
 
    if ((rv = getaddrinfo(messageTo, PORT, &hints1, &servinf)) != 0) {
       std::cout<<"\nerror getting the addr info";
       return;
    }
 
    // loop through all the results and connect to the first we can
    for(q = servinf; q != NULL; q = q->ai_next) {
        if ((sockfd = socket(q->ai_family, q->ai_socktype,
                q->ai_protocol)) == -1) {
               std::cout<<"client: socket";
            continue;
        }
 
        if (connect(sockfd, q->ai_addr, q->ai_addrlen) == -1) {
            close(sockfd);
            std::cout<<"client: connect";
            continue;
        }
 
        break;
    }
 
    if (q == NULL) {
        std::cout<<"stderr,client: failed to connect\n";
        return;
    }
 
    inet_ntop(q->ai_family, get1_in_addr((struct sockaddr *)q->ai_addr), s, sizeof s);
   printf("client: connecting to %s\n", s);
   
    freeaddrinfo(servinf); // all done with this structure
    
     if((numbytes=send(sockfd,msg,sizeof(msg),0)) == -1)
   { std::cout<<"there was an error on forwarding the request to the other server"<<messageTo<<std::endl;
     return;  }   
 
 bzero(msg,0);
 
    close(sockfd);
 
    return;
}

