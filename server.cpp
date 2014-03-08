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
#define PORT1 "44002"
#define PORT2 "24001"

#define PRT "50001"
#define PRT1 "50002"
#define PRT2 "50003"
#define PRT3 "50005"
#define PRT4 "50006"
#define PRT5 "50007"
typedef struct element
{
int client_id;
int data_object;
int type;
int permission;
}Element;


int updatecount(0); //we want it to get updated 50 times.

//list for storing the requests

std::list<Element> requests;
std::list<Element>::iterator it;

//there are 4 data objects
int data_object[4]; 

//We need locks of three types:
//1) NOT_LOCKED  of the boolean type
//2) READ_LOCKED of the boolean type
//3) WRITE_LOCKED of the boolean type

bool NOT_LOCKED[4]; //initially set to false
bool READ_LOCKED[4];
bool WRITE_LOCKED[4];

int READ_COUNT[4];

std::map<int, std::string> clients;
std::map<int, std::string>::iterator clients_it;


//making the map for the servers.
void makemap()
 {
   std::string clientnames[5]= { "net08.utdallas.edu", "net09.utdallas.edu", "net10.utdallas.edu", "net11.utdallas.edu", "net12.utdallas.edu" }; 
   for(int adding=0; adding <5 ; adding++)
       clients.insert(make_pair(adding+1,clientnames[adding]));
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
void ReadSend( char *reqms, int clientid)
{ std::cout<<"started send thread with message  "<<reqms<<"   to client  "<<clientid<<std::endl;
   //iterate the map to find the server name
    std::string clientname;
    clients_it=clients.begin();
        while(clients_it!=clients.end())
           {
             if(clients_it->first == clientid)
                { clientname=clients_it->second; } 
              clients_it++;
           }
    char *messageTo=new char[clientname.length()];
    clientname.copy(messageTo, clientname.length());
    messageTo[clientname.length()]='\0';
 
    int sockfd, numbytes; 
    struct addrinfo hints1, *servinf, *q;
    int rv;
    char s[INET6_ADDRSTRLEN];
 
    memset(&hints1, 0, sizeof hints1);
    hints1.ai_family = AF_UNSPEC;
    hints1.ai_socktype = SOCK_STREAM;
 
    if ((rv = getaddrinfo(messageTo, PRT, &hints1, &servinf)) != 0) {
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


//making the request to the servers
void WriteSend( char *reqms, int clientid)
{ std::cout<<"started send thread with message  "<<reqms<<"   to client  "<<clientid<<std::endl;
   //iterate the map to find the server name
    std::string clientname;
    clients_it=clients.begin();
        while(clients_it!=clients.end())
           {
             if(clients_it->first == clientid)
                { clientname=clients_it->second; } 
              clients_it++;
           }
    char *messageTo=new char[clientname.length()];
    clientname.copy(messageTo, clientname.length());
    messageTo[clientname.length()]='\0';
 
    int sockfd, numbytes; 
    struct addrinfo hints1, *servinf, *q;
    int rv;
    char s[INET6_ADDRSTRLEN];
 
    memset(&hints1, 0, sizeof hints1);
    hints1.ai_family = AF_UNSPEC;
    hints1.ai_socktype = SOCK_STREAM;
 
    if ((rv = getaddrinfo(messageTo, PRT, &hints1, &servinf)) != 0) {
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

//making the request to the servers
void RecvCommit( char *reqms, int clientid)
{ std::cout<<"started send thread with message  "<<reqms<<"   to client  "<<clientid<<std::endl;
   //iterate the map to find the server name
    std::string clientname;
    clients_it=clients.begin();
        while(clients_it!=clients.end())
           {
             if(clients_it->first == clientid)
                { clientname=clients_it->second; } 
              clients_it++;
           }
    char *messageTo=new char[clientname.length()];
    clientname.copy(messageTo, clientname.length());
    messageTo[clientname.length()]='\0';
 
    int sockfd, numbytes; 
    struct addrinfo hints1, *servinf, *q;
    int rv;
    char s[INET6_ADDRSTRLEN];
 
    memset(&hints1, 0, sizeof hints1);
    hints1.ai_family = AF_UNSPEC;
    hints1.ai_socktype = SOCK_STREAM;
 
    if ((rv = getaddrinfo(messageTo, PRT3, &hints1, &servinf)) != 0) {
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

//making the request to the servers
void RecvWith( char *reqms, int clientid)
{ std::cout<<"started send thread with message  "<<reqms<<"   to client  "<<clientid<<std::endl;
   //iterate the map to find the server name
    std::string clientname;
    clients_it=clients.begin();
        while(clients_it!=clients.end())
           {
             if(clients_it->first == clientid)
                { clientname=clients_it->second; } 
              clients_it++;
           }
    char *messageTo=new char[clientname.length()];
    clientname.copy(messageTo, clientname.length());
    messageTo[clientname.length()]='\0';
 
    int sockfd, numbytes; 
    struct addrinfo hints1, *servinf, *q;
    int rv;
    char s[INET6_ADDRSTRLEN];
 
    memset(&hints1, 0, sizeof hints1);
    hints1.ai_family = AF_UNSPEC;
    hints1.ai_socktype = SOCK_STREAM;
 
    if ((rv = getaddrinfo(messageTo, PRT3, &hints1, &servinf)) != 0) {
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


//making the request to the servers
void ReadRecvACK1( char *reqms, int clientid)
{ std::cout<<"started send thread with message  "<<reqms<<"   to client  "<<clientid<<std::endl;
   //iterate the map to find the server name
    std::string clientname;
    clients_it=clients.begin();
        while(clients_it!=clients.end())
           {
             if(clients_it->first == clientid)
                { clientname=clients_it->second; } 
              clients_it++;
           }
    char *messageTo=new char[clientname.length()];
    clientname.copy(messageTo, clientname.length());
    messageTo[clientname.length()]='\0';
 
    int sockfd, numbytes; 
    struct addrinfo hints1, *servinf, *q;
    int rv;
    char s[INET6_ADDRSTRLEN];
 
    memset(&hints1, 0, sizeof hints1);
    hints1.ai_family = AF_UNSPEC;
    hints1.ai_socktype = SOCK_STREAM;
 
    if ((rv = getaddrinfo(messageTo, PRT4, &hints1, &servinf)) != 0) {
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

//making the request to the servers
void ReadWithACK2( char *reqms, int clientid)
{ std::cout<<"started send thread with message  "<<reqms<<"   to client  "<<clientid<<std::endl;
   //iterate the map to find the server name
    std::string clientname;
    clients_it=clients.begin();
        while(clients_it!=clients.end())
           {
             if(clients_it->first == clientid)
                { clientname=clients_it->second; } 
              clients_it++;
           }
    char *messageTo=new char[clientname.length()];
    clientname.copy(messageTo, clientname.length());
    messageTo[clientname.length()]='\0';
 
    int sockfd, numbytes; 
    struct addrinfo hints1, *servinf, *q;
    int rv;
    char s[INET6_ADDRSTRLEN];
 
    memset(&hints1, 0, sizeof hints1);
    hints1.ai_family = AF_UNSPEC;
    hints1.ai_socktype = SOCK_STREAM;
 
    if ((rv = getaddrinfo(messageTo, PRT5, &hints1, &servinf)) != 0) {
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
void sendTerminate( char *reqms, int clientid)
{ 
   //iterate the map to find the server name
    std::string clientname;
    clients_it=clients.begin();
        while(clients_it!=clients.end())
           {
             if(clients_it->first == clientid)
                { clientname=clients_it->second; } 
              clients_it++;
           }
    char *messageTo=new char[clientname.length()];
    clientname.copy(messageTo, clientname.length());
    messageTo[clientname.length()]='\0';
 
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




int main(int argc, char *argv[])
{
int servid=atoi(argv[1]);
//getting clients in the datastructure
makemap();

//intially data_objects are all zero;
for(int i=0; i<4 ; i++)
     data_object[i]=0;

//locks are all false
for(int i=0; i<4;i++)
{
NOT_LOCKED[i]=false; 
READ_LOCKED[i]=false;
WRITE_LOCKED[i]=false;
READ_COUNT[i]=0;
}

//now as a server, we need a recv function here...

   int sockfdq, newsockfdq, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     sockfdq = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfdq < 0)
        std::cout<<"ERROR opening socket";
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 50000;//atoi(argv[2]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfdq, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
              std::cout<<"ERROR on binding";
     listen(sockfdq,50);
int wrt(0);
//std::string breakup[6]; 
int update_atempt=0;
       while(update_atempt!=50)
       { n=0; 
       std::string breakup[6];
	   for(int i=0;i<6;i++) breakup[i]="";
            char *buffer=new char[256];
            clilen = sizeof(cli_addr);
             
              newsockfdq = accept(sockfdq, (struct sockaddr *) &cli_addr, &clilen);
                 if (newsockfdq< 0)
                    std::cout<<"ERROR on accept";
 
                   bzero(buffer,256);
                  
                n = recv(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                 if (n < 0) std::cout<<"ERROR reading from socket";
                  std::cout<<"Message received by the RECEIVE THREAD on the server  is  "<<buffer<<std::endl;
                  
            
 
           buffer[n]='\0';
           
           int count(0);
           int index(0);
           while(index <=strlen(buffer) )//buffer[index]!='\0')
            {
                if(buffer[index] == ' ' || buffer[index] == '\0') 
		  {
		   index++; count++;
		  }
                else { breakup[count] += buffer[index]; index++; }
                
         
            }
std::cout<<breakup[0];

std::cout<<breakup[1];

std::cout<<breakup[2];

std::cout<<breakup[3];

      // std::cout<<"the message received is "<<buffer;
       bzero(buffer,256);
   
std::stringstream dd(breakup[3]);
//data object for which the request has arrived.
                int DO;
//		dd>>DO;
   DO=atoi(breakup[3].c_str());
std::cout<<"data_object  "<<DO<<std::endl; 
	 //now if it is a read request.
	 
	 
                 if(breakup[0] == "11") //it is a commit message
		  {   DO=atoi(breakup[2].c_str());
		         int adding=atoi(breakup[3].c_str());
		        data_object[DO]+=adding;
                    //then we need to remove the element from the list maintained.
                    Element ele;
		     //std::stringstream ci(breakup[2]);
		                        int cid=atoi(breakup[1].c_str());
							                      ele.client_id=cid;
												                    int data=atoi(breakup[2].c_str());
																                          ele.data_object=data;																					                        int ty=atoi(breakup[0].c_str()); 																						                      ele.type=ty;
                    ele.permission=1;


						   std::cout<<"*******update number*******"<<updatecount++<<std::endl<<std::endl;
 //prepare message to forward to the client
                std::stringstream vald;
                vald << data_object[DO];
                std::string val=vald.str();	
                vald.str(""); vald.clear();				
         			   std::string messagetoclient="3 "+(breakup[2])+ " "+ val;
				   
		           std::cout<<"message forwarded to the client with response to commit request  "<<messagetoclient<<std::endl;
		           bzero(buffer, 256);
		           messagetoclient.copy(buffer, messagetoclient.length());
		          //send out the value to the client as approval
		        
                std::thread send(RecvCommit,std::ref(buffer),cid);
		send.join();
		//            n = send(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
		  //       if (n < 0) std::cout<<"ERROR writing to socket";

 
                     if(requests.size() > 0)
                   { std::list<Element>::iterator it;
                     it=requests.begin();
                      while(it!=requests.end())
                       {
                          if(it->client_id == ele.client_id && it->data_object==ele.data_object && it->type==ele.type && it->permission == ele.permission) 
                           {
                          
                     requests.erase(it); break; }
                       it++;
                       }
                    } 
                   //send back the ack, and then need to add the value to the data_object in question.
                  // data_object[DO]++;
 		 WRITE_LOCKED[DO]=false;
                   std::ofstream output("output.txt", std::ios::app);
                   output<<"the data object updated is "<<DO <<" and its value is "<< data_object[DO]<<std::endl;
                   output.close();
                  
                   if(WRITE_LOCKED[DO] == false && READ_COUNT[DO]==0)
                       { NOT_LOCKED[DO]=false; READ_LOCKED[DO]==false; }
                        
                  if(WRITE_LOCKED[DO]==false) //   if(NOT_LOCKED[DO]==false) //For the same data object, we need to send the request
                      {
                         Element ele;
                        bool check=false;
                        ///////////send approval
                        if(requests.size() > 0)
                           {    std::list<Element>::iterator it;
                                it=requests.begin();
                                while(it!=requests.end())
                                 {
                                   if(it->data_object == DO)
                                    {
                                     ele.client_id =it->client_id;
                                     ele.data_object=it->data_object;
                                     ele.type=it->type;
                                     ele.permission=1;
                                     it->permission=1;
                                     check=true;
                                     break;
                                    }
                                  it++;
                                 }
                                if(check==true)
                                 { check=false;
                                    std::string message;
                                    if(ele.type==0) //its a read request
                                     { std::stringstream ss,sd;
				      ss<<ele.data_object;
				      sd<<data_object[DO];
					  
				               message= "1 "+ss.str() + " "+sd.str();
					  READ_COUNT[DO]++; NOT_LOCKED[DO]=true; READ_LOCKED[DO]=true;
  
                                       char *mess=new char[message.length()];
                                      message.copy(mess,message.length());
									  std::cout<<"sending the approval for next read on receiving the commit message, message to next client is "<<message<<std::endl;
                                      std::thread request(ReadSend,std::ref(mess),ele.client_id);
                                      request.join();
                                     }
                                    else
                                     {
				        std::stringstream ss,sd;
					//ss<<ele.data_object ;
					ss<<servid;
					sd<<data_object[DO];
				         message= "2 "+ss.str() + " "+sd.str();
                                       WRITE_LOCKED[DO]=true; NOT_LOCKED[DO]=true;
                                       char *mess=new char[message.length()];
                                      message.copy(mess,message.length());
									  std::cout<<"sending the approval for next write on receiving the commit message, message to next client is "<<message<<std::endl;
                                      std::thread request(WriteSend,std::ref(mess),ele.client_id);
                                      request.join();
                                     
                                     }
                                 //send the message

                                 }


                           }
       





                      }
               
		  }
	 
	 
	    if(breakup[0]=="0")
	      {
                 if(breakup[1]=="0")
                 {
                  std::cout<<"got a read request"<<std::endl; 
                   //initially when the data object is not locked.
                    if(NOT_LOCKED[DO] == false) //permission could be granted
                    {
                    READ_COUNT[DO]++; 
                    READ_LOCKED[DO]=true;
                     //request goes to the list
                   Element ele;
		   int cid=atoi(breakup[2].c_str());
		   ele.client_id=cid;
		   int data=atoi(breakup[3].c_str());
		   ele.data_object=data;
		   int ty=atoi(breakup[0].c_str());
		   ele.type=ty;
                   ele.permission=1;
		   std::cout<<"element pushed for read"<<std::endl;
		   std::cout<<std::endl<<"C_id"<<cid<<"  Data Object "<<data<<" Type "<<ty<<std::endl; 
                   requests.push_back(ele);

                   //prepare message to forward to the client
		   std::stringstream sss;
		   sss<<data_object[DO];
                   std::string messagetoclient="1 "+breakup[3] +" " + sss.str();
				   sss.str("");
				   sss.clear();
                   std::cout<<"message forwarded to the client with response to read request, when NOT_LOCKED=false  "<<messagetoclient<<std::endl;
                   bzero(buffer, 256);
                   messagetoclient.copy(buffer, messagetoclient.length());
                  //send out the value to the client as approval
		  std::cout<<"sending the approval to read request"<<std::endl;
                 std::thread send(ReadSend,std::ref(buffer),cid);
		 send.join();
               //     n = send(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
               //  if (n < 0) std::cout<<"ERROR writing to socket";
                    bzero(buffer, 256);
                
                  }
                // else   //not_locked is true.
                  // {  //this means not_locked is true and read_locked is also true, and read_count is also >0
                        else if( WRITE_LOCKED[DO] == false || (READ_LOCKED[DO]==true && WRITE_LOCKED[DO]==false)) //I can give more permissions for the data_object
                         {
                           READ_COUNT[DO]++;
                          Element ele;
                          int cid=atoi(breakup[2].c_str());
	                  ele.client_id=cid;
	                  int data=atoi(breakup[3].c_str());
													                           ele.data_object=data;
																				                   																				                      int ty=atoi(breakup[0].c_str());																							                    ele.type=ty;

                       
                          ele.permission=1;
                        
                          requests.push_back(ele);
                         std::cout<<std::endl<<"C_id"<<cid<<"  Data Object "<<data<<" Type "<<ty<<std::endl<<"Pushed to the list"<<std::endl;
                     //prepare message to forward to the client

		   std::stringstream sds;
		   sds<<data_object[DO];
                   std::string messagetoclient="1 "+(breakup[3]) +" "+sds.str();
				   sds.str("");
				   sds.clear();
                   std::cout<<"message forwarded to the client with response to read request, when WRITE_LOCKED=false  "<<messagetoclient<<std::endl;
                   bzero(buffer, 256);
                   messagetoclient.copy(buffer, messagetoclient.length());
                  //send out the value to the client as approval
		  std::cout<<"sending the message to the client "<<std::endl;
                std::thread send(ReadSend,std::ref(buffer),cid);
		send.join();
               //          n = send(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                 //        if (n < 0) std::cout<<"ERROR writing to socket";
                    bzero(buffer, 256);
                        }
                    
                else   if(WRITE_LOCKED[DO]==true) //write_locked is true, I cant provide permission for this.
                         {
                          //put the request to the queue, and dont send any message as of now the client.
                          Element ele;
			                      int cid=atoi(breakup[2].c_str());
     			                    ele.client_id=cid;
		                          int data=atoi(breakup[3].c_str());
	                                ele.data_object=data;
                                int ty=atoi(breakup[0].c_str());
																											                                            ele.type=ty;
                          ele.permission=0;
                         std::cout<<std::endl<<"C_id"<<cid<<"  Data Object "<<data<<" Type "<<ty<<std::endl<<"Pushed to the list"<<std::endl;
                          requests.push_back(ele);
                         }
			 else { ; }
                   
                }
                if(breakup[1] == "1" ) //its a read commit message
                   {
		   std::cout<<"commit message received by the server "<<std::endl;
                     //server decrease the readcount by one for a particular data object.
                    // int DO=std::stoi(breakup[3]);
                     READ_COUNT[DO]--;
                      if(READ_COUNT[DO] == 0) 
                           {READ_LOCKED[DO]=false; }
                      if(READ_COUNT[DO] == 0 && WRITE_LOCKED[DO] == false) //time to set status to not locked
                       { NOT_LOCKED[DO]=false; READ_LOCKED[DO]=false ;}
           
                    //need to remove the item from the list.
                    Element ele;
		                        int cid=atoi(breakup[2].c_str());
			                      ele.client_id=cid;
			                   int data=atoi(breakup[3].c_str());
			                ele.data_object=data;	
                        int ty=atoi(breakup[0].c_str());																							                      ele.type=ty;
                    ele.permission=1;
			       if(requests.size() > 0)
                   { std::list<Element>::iterator it;
                     it=requests.begin();
                      while(it!=requests.end())
                       {
                          if(it->client_id == ele.client_id && it->data_object==ele.data_object && it->type==ele.type && it->permission == ele.permission) 
                           {
                          std::cout<<"Item poped after receiving the COMMIT MESSAGE "<<std::endl;
                     requests.erase(it); break; }
                       it++;
                       }
                    }
                     //prepare message to forward to the client as an ACK
                  // std::string messagetoclient="6 33";
                  // std::cout<<"message forwarded to the client with response to read requests COMMIT message "<<messagetoclient<<std::endl;
                   bzero(buffer, 256);
                  // messagetoclient.copy(buffer, messagetoclient.length());
                  //send out the value to the client as approval
             // std::cout<<"sending read commint message "<<std::endl;              
                //std::thread send(ReadRecvACK1,std::ref(buffer),cid);
		//send.join();
                   // n = send(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                // if (n < 0) std::cout<<"ERROR writing to the socket";
               //   bzero(buffer, 256);
                //now send request to next in the list...... for the same dataobject
                       
                     if(WRITE_LOCKED[DO] == false)
                     {
                        Element ele;
                        bool check=false;
                        ///////////send approval
                        if(requests.size() > 0)
                           {    std::list<Element>::iterator it;
                                it=requests.begin();
                                while(it!=requests.end())
                                 {
                                   if(it->data_object == DO)
                                    {
                                     ele.client_id =it->client_id;
                                     ele.data_object=it->data_object;
                                     ele.type=it->type;
                                     ele.permission=1;
                                     it->permission=1;
                                     check=true;
                                     break;
                                    }
                                  it++;
                                 }
                                if(check==true)
                                 {  check=false;
                                    std::string message;
                                    if(ele.type==0) //its a read request
                                     { 
				          std::stringstream ss,sd;
					  ss<<data_object[DO];
					  sd<<ele.data_object;

				           message= "1 "+sd.str() + " "+ ss.str();
						   ss.str("");ss.clear(); sd.str("");sd.clear();
                                       std::cout<<"sending request to the next client for the same data object with the message "<<message<<std::endl;
                                       char *mess=new char[message.length()];
                                       READ_COUNT[DO]++; 
                                       NOT_LOCKED[DO]=true; 
                                       READ_LOCKED[DO]=true;
                                      message.copy(mess,message.length());
                                      std::thread request(ReadSend,std::ref(mess),ele.client_id);
                                      request.join();
                                     }
                                    else
                                     {std::stringstream ss,sd;
				        ss<<data_object[DO];
				        sd<<servid;
						
				         message= "2 "+sd.str() + " "+ss.str();
						   ss.str("");ss.clear(); sd.str("");sd.clear();
						  std::cout<<"sending request to the next client for the same data object(WRITE) with the message "<<message<<std::endl;
                                       WRITE_LOCKED[DO]=true; NOT_LOCKED[DO]=true;
                                       char *mess=new char[message.length()];
                                      message.copy(mess,message.length());
                                      std::thread request(WriteSend,std::ref(mess),ele.client_id);
                                      request.join();
                                     
                                     }
                                 //send the message

                                 }

                           }
                     }
                      
                   }
                if(breakup[1] == "2" ) //its a read withdraw message
                   {
                      //i need to remove the item from the
                      Element ele;
		                          int cid=atoi(breakup[2].c_str());
				           ele.client_id=cid;
				          int data=atoi(breakup[3].c_str());
				        ele.data_object=data;																		                          int ty=atoi(breakup[0].c_str());																						                        ele.type=ty;
                      ele.permission=0;
               
                       if(requests.size() > 0)
                   { std::list<Element>::iterator it;
                     it=requests.begin();
                      while(it!=requests.end())
                       {
                          if(it->client_id == ele.client_id && it->data_object==ele.data_object && it->type==ele.type && (it->permission == ele.permission || it->permission == 1)) 
                           {
                          if(it->permission == 1) 
                           { READ_COUNT[DO]-- ;
                               if(READ_COUNT[DO] == 0 ) READ_LOCKED[DO]=false;
                               if(READ_COUNT[DO]==0 && WRITE_LOCKED[DO]==false) 
                               {READ_LOCKED[DO]=false ; NOT_LOCKED[DO]=false; }
                           }
                     requests.erase(it); break; }
                       it++;
                       }
                    }
                      
                      //std::string messagetoclient="7 33";
                     // bzero(buffer,256);
		     // std::cout<<"sending the withdraw approval "<<std::endl;
                     // messagetoclient.copy(buffer,messagetoclient.length());
 
              //  std::thread send(ReadWithACK2,std::ref(buffer),cid);
		//send.join();
                //          n = send(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
                // if (n < 0) std::cout<<"ERROR writing to the socket";

                         if(WRITE_LOCKED[DO] == false)
                     {
                        Element ele;
                        bool check=false;
                        ///////////send approval
                        if(requests.size() > 0)
                           {    std::list<Element>::iterator it;
                                it=requests.begin();
                                while(it!=requests.end())
                                 {
                                   if(it->data_object == DO)
                                    {
                                     ele.client_id =it->client_id;
                                     ele.data_object=it->data_object;
                                     ele.type=it->type;
                                     ele.permission=1;
                                     it->permission=1;
                                     check=true;
                                     break;
                                    }
                                  it++;
                                 }
                                if(check==true)
                                 { check=false;
                                    std::string message;
                                    if(ele.type==0) //its a read request
                                     {  std::stringstream ss,sd;
				        sd<<ele.data_object;
				        ss<<data_object[DO];
				        message= "1 "+sd.str() + " "+ss.str();
						  ss.str("");ss.clear(); sd.str("");sd.clear();
                                          READ_COUNT[DO]++; NOT_LOCKED[DO]=true; READ_LOCKED[DO]=true;
                                       char *mess=new char[message.length()];
				       std::cout<<"sending approval for read on receving a withdraw (READ) "<<message<<std::endl;
                                      message.copy(mess,message.length());
                                      std::thread request(ReadSend,std::ref(mess),ele.client_id);
                                      request.join();
                                     }
                                    else
                                     { std::stringstream ss,sd;
				     ss<<servid;
				     sd<<data_object[DO];
				      message= "2 "+ss.str()+ " "+sd.str();
					    ss.str("");ss.clear(); sd.str("");sd.clear();
                                           WRITE_LOCKED[DO]=true; NOT_LOCKED[DO]=true;
                                       char *mess=new char[message.length()];
                                      message.copy(mess,message.length());
				      std::cout<<"sending approval for write on receing a withdraw (READ) "<<message<<std::endl;
                                      std::thread request(WriteSend,std::ref(mess),ele.client_id);
                                      request.join();
                                     
                                     }
                                 //send the message

                                 }

                           }
                     }
                  
                   }
	      }


         //now if its a write request
            if(breakup[0]== "1")
             {
	   //  std::cout<<"WRITE ATTEMPT-====>>>"<<wrt;
	   //  update_atempt++;
			 std::cout<<"the write  ";
                //initially when the data object is not locked.
                 if(breakup[1] == "0") //its a request message
                  { wrt++;
		  update_atempt++;
				    std::cout<<"number " << wrt <<"request was received by the server "<<std::endl;
                      if(WRITE_LOCKED[DO] == false || (WRITE_LOCKED[DO]==false && READ_LOCKED[DO]==true) ||(WRITE_LOCKED[DO]==false && READ_LOCKED[DO]==true)) // I can grant write approval to the server.
                       {
                           NOT_LOCKED[DO]=true;
                           WRITE_LOCKED[DO]=true;
                          
                         
                            //request goes to the list
		           Element ele;
			                       int cid=atoi(breakup[2].c_str());
								                     ele.client_id=cid;
										                   int data=atoi(breakup[3].c_str());
																	                         ele.data_object=data;
																						                       int ty=atoi(breakup[0].c_str());																							                     ele.type=ty;
		           ele.permission=1;
		           requests.push_back(ele);
                  std::cout<<"request pushed to the queue "<<std::endl;
		           //prepare message to forward to the client
			   std::stringstream ss,sdf;
			   sdf<<data_object[DO];
			   ss<<servid;
		           std::string messagetoclient="2 "+ss.str()+" "+sdf.str();
		          
		           bzero(buffer, 256);
		           messagetoclient.copy(buffer, messagetoclient.length());
		          //send out the value to the client as approval
		        
			std::cout<<"sending the approval message to client "<<messagetoclient<<std::endl;
                std::thread send(WriteSend,std::ref(buffer),cid);
		send.join();
	//	            n = send(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
	//	         if (n < 0) std::cout<<"ERROR writing to socket";

                        
		            
		      }
                      else //not_locked ==true
                       {   std::cout<<"request received when not_locked[DO] ==true, so just adding it to the list "<<std::endl;
                          if( WRITE_LOCKED[DO] == true) //just add the request to the list.
                           {
                                //request goes to the list
		           Element ele;
			                       int cid=atoi(breakup[2].c_str());
								                     ele.client_id=cid;
													                   int data=atoi(breakup[3].c_str());
																	                         ele.data_object=data;
			
																						                       int ty=atoi(breakup[0].c_str());																							                     ele.type=ty;
		           ele.permission=0;
		           requests.push_back(ele);
                           }
                       }



                  }
		   
		 if(breakup[1] == "2") //its a withdraw message
		  {
                    
                    //then we need to remove the element from the list maintained.
                    Element ele;
		     int cid=atoi(breakup[2].c_str());
							                      ele.client_id=cid;
												                    int data=atoi(breakup[3].c_str());
																                          ele.data_object=data;																	                        int ty=atoi(breakup[0].c_str());																						                      ele.type=ty;
                    ele.permission=0;
                     if(requests.size() > 0)
                   { std::list<Element>::iterator it;
                     it=requests.begin();
                      while(it!=requests.end())
                       {
                          if(it->client_id == ele.client_id && it->data_object==ele.data_object && it->type==ele.type && (it->permission == ele.permission ||it->permission ==1 )) 
                           {
                          std::cout<<"removing the request for which withdraw has been received "<<std::endl;
                     requests.erase(it); break; }
                       it++;
                       }
                    }
 if(WRITE_LOCKED[DO]==true) WRITE_LOCKED[DO]=false;
//prepare message to forward to the client
if(READ_LOCKED[DO]==true && WRITE_LOCKED[DO]==false) NOT_LOCKED[DO]=false;
		           std::string messagetoclient="5 "+(breakup[2]);
		           std::cout<<"message forwarded to the client with response to withdraw request  "<<messagetoclient<<std::endl;
		           bzero(buffer, 256);
		           messagetoclient.copy(buffer, messagetoclient.length());
		          //send out the value to the client as approval
		        
                std::thread send(RecvWith,std::ref(buffer),cid);
		send.join();
		//            n = send(newsockfdq,buffer,255, 0);//read(newsockfdq,buffer,strlen(buffer));
		  //       if (n < 0) std::cout<<"ERROR writing to socket";

                        if(WRITE_LOCKED[DO] == false)
                     {
                        ///////////send approval
                         Element ele;
                        bool check=false;
                        ///////////send approval
                        if(requests.size() > 0)
                           {    std::list<Element>::iterator it;
                                it=requests.begin();
                                while(it!=requests.end())
                                 {
                                   if(it->data_object == DO)
                                    {
                                     ele.client_id =it->client_id;
                                     ele.data_object=it->data_object;
                                     ele.type=it->type;
                                     ele.permission=1;
      			             it->permission=1;
                                     check=true;
                                     break;
                                    }
                                  it++;
                                 }
                                if(check==true)
                                 { check=true;
                                    std::string message;
                                    if(ele.type==0) //its a read request
                                     {  
				        std::stringstream ss,sd;
					ss<<ele.data_object ;
					sd<<data_object[DO];
				     message= "1 "+ss.str() + " "+sd.str();
 					  READ_COUNT[DO]++; NOT_LOCKED[DO]=true; READ_LOCKED[DO]=true;
 
                                       char *mess=new char[message.length()];
                                      message.copy(mess,message.length());
									   std::cout<<"sending approval for next read after receiving a withdraw message with the message "<<mess<<std::endl;
                                      std::thread request(ReadSend,std::ref(mess),ele.client_id);
                                      request.join();
                                     }
                                    else
                                     {
				        std::stringstream ss,sd;
					//ss<<ele.data_object ;
					ss<<servid;
					sd<<data_object[DO];
				      message= "2 "+ss.str()+ " "+sd.str();
 					 WRITE_LOCKED[DO]=true; NOT_LOCKED[DO]=true;
  
                                       char *mess=new char[message.length()];
                                      message.copy(mess,message.length());
									   std::cout<<"sending approval for next write after receiving a withdraw message with the message "<<mess<<std::endl;
                                      std::thread request(WriteSend,std::ref(mess),ele.client_id);
                                      request.join();
                                     
                                     }
                                 //send the message

                                 }

                           }
                     }
		  }
             }

       }

close(newsockfdq);
close(sockfdq);

if(update_atempt == 50 )
 {
          std::string message = "9";
          char *mess=new char[message.length()];
          message.copy(mess,message.length());
         for(int i=0;i<4;i++)
         {
          std::thread terminate(sendTerminate,std::ref(mess),i+1);
          terminate.join(); 
         }
 }
return 0;
} 
