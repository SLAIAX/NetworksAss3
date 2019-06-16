//////////////////////////////////////////////////////////////////////////////////////////////
// TCP CrossPlatform CLIENT v.1.0 (towards IPV6 ready)
// compiles using GCC 
//
//
// References: https://msdn.microsoft.com/en-us/library/windows/desktop/ms738520(v=vs.85).aspx
//             http://long.ccaba.upc.edu/long/045Guidelines/eva/ipv6.html#daytimeServer6
//             Andre Barczak's tcp client codes
//
// Author: Napoleon Reyes, Ph.D.
//         Massey University, Albany  
//
//////////////////////////////////////////////////////////////////////////////////////////////

#define USE_IPV6 true
#define DEFAULT_PORT "1234" 

#if defined __unix__ || defined __APPLE__
  #include <unistd.h>
  #include <errno.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netdb.h> //used by getnameinfo()
  #include <cstdio>
  #include <iostream>
  #include <time.h>
#elif defined _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h> //required by getaddrinfo() and special constants
  #include <stdlib.h>
  #include <stdio.h>
  #include <time.h>
  #include <cstdio>
  #include <iostream>
  #define WSVERS MAKEWORD(2,2) /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
                    //The high-order byte specifies the minor version number; 
                    //the low-order byte specifies the major version number.

  WSADATA wsadata; //Create a WSADATA object called wsadata. 
#endif
  #include "InfInt.h"

//////////////////////////////////////////////////////////////////////////////////////////////


enum CommandName{USER, PASS, SHUTDOWN};

using namespace std;
/////////////////////////////////////////////////////////////////////

void printBuffer(const char *header, char *buffer){
	cout << "------" << header << "------" << endl;
	for(unsigned int i=0; i < strlen(buffer); i++){
		if(buffer[i] == '\r'){
		   cout << "buffer[" << i << "]=\\r" << endl;	
		} else if(buffer[i] == '\n'){
		   cout << "buffer[" << i << "]=\\n" << endl;	
		} else {   
		   cout << "buffer[" << i << "]=" << buffer[i] << endl;
		}
	}
	cout << "---" << endl;
}

InfInt repeatSquare(InfInt  x, InfInt  e, InfInt  n) {

  InfInt y=1;//initialize y to 1, very important
  while (e >  0) {
    if (( e % 2 ) == 0) {
      x = (x*x) % n;
      e = e/2;
    }
    else {
      y = (x*y) % n;
      e = e-1;
    }
  }
  return y; //the result is stored in y
}

// Variables
InfInt nCA = 8633;
InfInt eCA = 7;
InfInt nSERV;
InfInt eSERV;


/////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]) {
//*******************************************************************
// Initialization
// What are the important data structures?
//*******************************************************************
	
   char portNum[NI_MAXSERV];

#if defined __unix__ || defined __APPLE__
   int s;
#elif defined _WIN32
   SOCKET s;
#endif

#define BUFFER_SIZE 2000 
//remember that the BUFFESIZE has to be at least big enough to receive the answer from the server
#define SEGMENT_SIZE 70
//segment size, i.e., if fgets gets more than this number of bytes it segments the message

   char send_buffer[BUFFER_SIZE],receive_buffer[BUFFER_SIZE];
   char temp_buffer[BUFFER_SIZE*5];
   int n;
   ssize_t bytes;
	
   char serverHost[NI_MAXHOST]; 
   char serverService[NI_MAXSERV];
	

#if defined __unix__ || defined __APPLE__
   //nothing to do here

#elif defined _WIN32
//********************************************************************
// WSSTARTUP
//********************************************************************

//********************************************************************
// WSSTARTUP
/*  All processes (applications or DLLs) that call Winsock functions must 
  initialize the use of the Windows Sockets DLL before making other Winsock 
  functions calls. 
  This also makes certain that Winsock is supported on the system.
*/
//********************************************************************
   int err;
  
   err = WSAStartup(WSVERS, &wsadata);
   if (err != 0) {
      WSACleanup();
    /* Tell the user that we could not find a usable */
    /* Winsock DLL.                                  */
      printf("WSAStartup failed with error: %d\n", err);
      exit(1);
   }
  
//********************************************************************
/* Confirm that the WinSock DLL supports 2.2.        */
/* Note that if the DLL supports versions greater    */
/* than 2.2 in addition to 2.2, it will still return */
/* 2.2 in wVersion since that is the version we      */
/* requested.                                        */
//********************************************************************
    printf("\n\n<<<TCP (CROSS-PLATFORM, IPv6-ready) CLIENT, by nhreyes>>>\n");  

    if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        exit(1);
    }
    else{
         
        printf("\nThe Winsock 2.2 dll was initialised.\n");
    }

#endif


//********************************************************************
// set the socket address structure.
//********************************************************************
struct addrinfo *result = NULL;
struct addrinfo hints;
int iResult;

memset(&hints, 0, sizeof(struct addrinfo));


if(USE_IPV6){
   hints.ai_family = AF_INET6;  
} else { //IPV4
   hints.ai_family = AF_INET;
}

hints.ai_socktype = SOCK_STREAM;
hints.ai_protocol = IPPROTO_TCP;
	
//*******************************************************************
//	Dealing with user's arguments
//*******************************************************************
	
	//if there are 3 parameters passed to the argv[] array.
   if (argc == 3){ 
	    sprintf(portNum,"%s", argv[2]);
	    iResult = getaddrinfo(argv[1], portNum, &hints, &result);
	} else {
	    printf("USAGE: ClientWindows IP-address [port]\n"); //missing IP address
		sprintf(portNum,"%s", DEFAULT_PORT);
		printf("Default portNum = %s\n",portNum);
		printf("Using default settings, IP:127.0.0.1, Port:1234\n");
		iResult = getaddrinfo("127.0.0.1", portNum, &hints, &result);
	}
	
	if (iResult != 0) {
		 printf("getaddrinfo failed: %d\n", iResult);
#if defined _WIN32
         WSACleanup();
#endif  
		 return 1;
   }	 
		
//*******************************************************************
//CREATE CLIENT'S SOCKET 
//*******************************************************************

#if defined __unix__ || defined __APPLE__
  	s = -1;
#elif defined _WIN32
 	s = INVALID_SOCKET;
#endif

	s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	
#if defined __unix__ || defined __APPLE__
  	if (s < 0) {
      printf("socket failed\n");
      freeaddrinfo(result);
  	}
#elif defined _WIN32
  //check for errors in socket allocation
  	if (s == INVALID_SOCKET) {
      printf("Error at socket(): %d\n", WSAGetLastError());
      freeaddrinfo(result);
      WSACleanup();
      exit(1);//return 1;
  	}
#endif

//*******************************************************************
//CONNECT
//*******************************************************************
	
	 if (connect(s, result->ai_addr, result->ai_addrlen) != 0) {
        printf("connect failed\n");
		freeaddrinfo(result);
#if defined _WIN32
        WSACleanup();
#endif 
   	    exit(1);
   } else {
		char ipver[80];
		
		// Get the pointer to the address itself, different fields in IPv4 and IPv6
		if (result->ai_family == AF_INET)
		{
			// IPv4
			strcpy(ipver,"IPv4");
		}
		else if(result->ai_family == AF_INET6)
		{
			// IPv6
			strcpy(ipver,"IPv6");
		}
			
		printf("\nConnected to <<<SERVER>>> with IP address: %s, %s at port: %s\n", argv[1], ipver,portNum);

#if defined __unix__ || defined __APPLE__     
       int returnValue;
#elif defined _WIN32      
       DWORD returnValue;
#endif

		memset(serverHost, 0, sizeof(serverHost));
	    memset(serverService, 0, sizeof(serverService));

        returnValue=getnameinfo((struct sockaddr *)result->ai_addr, /*addrlen*/ result->ai_addrlen,
               serverHost, sizeof(serverHost),
               serverService, sizeof(serverService), NI_NUMERICHOST);

		if(returnValue != 0){

#if defined __unix__ || defined __APPLE__     
           printf("\nError detected: getnameinfo() failed with error\n");
#elif defined _WIN32      
           printf("\nError detected: getnameinfo() failed with error#%d\n",WSAGetLastError());
#endif       
	       exit(1);

	    } else{
		   printf("\nConnected to <<<SERVER>>> extracted IP address: %s, %s at port: %s\n", serverHost, ipver,/* serverService */ portNum);  //serverService is nfa
	    }
		//--------------------------------------------------------------------------------
		
	}

	printf("\nRECEIVING PUBLIC KEY FROM SERVER...\n");
	// Receive nSERV and eSERV
	memset(receive_buffer, 0, strlen(receive_buffer));
	n = 0;
	while (1) {
	bytes = recv(s, &receive_buffer[n], 1, 0);

#if defined __unix__ || defined __APPLE__  
		     if ((bytes == -1) || (bytes == 0)) {
	            printf("recv failed\n");
	         	exit(1);
	         }   
      
#elif defined _WIN32      
             if ((bytes == SOCKET_ERROR) || (bytes == 0)) {
	            printf("recv failed\n");
	         	exit(1);
	         }
#endif

	         
	         if (receive_buffer[n] == '\n') {  /*end on a LF*/
	            receive_buffer[n] = '\0';
	            break;
	         }
	         if (receive_buffer[n] != '\r') n++;   /*ignore CR's*/
	}
	      
	
	printf("\nDECRYPTING PUBLIC KEY FROM SERVER...\n");
	// DECRYPT eSERV and nSERV

	memset(&temp_buffer, 0, BUFFER_SIZE*5);
	char * token;
	token = strtok(receive_buffer, ",");
	int i = 0;
	while(token != NULL){
	    long temp = atol(token);
	    char c = (repeatSquare(temp, eCA, nCA)).toInt();
	    temp_buffer[i] = c;
	    token = strtok(NULL, ",");
	    i++;
    }
    char eTemp[120];
    char nTemp[120];

    sscanf(temp_buffer, "%s %s", eTemp, nTemp);
    string temp = eTemp;
    eSERV = temp;
    temp = nTemp;
    nSERV = temp;
	// ACK receipt
	printf("\nACKNOWLEDGING RECEIPT OF KEY...\n");

	// Generate nonce

	srand(time(NULL));
	int nonce = rand() % 255;
	int random = nonce;

	// Send nonce
	memset(&send_buffer, 0, BUFFER_SIZE);
	InfInt EncryptedNonce = repeatSquare(nonce, eSERV, nSERV);
	sprintf(send_buffer, "%s\r\n", (EncryptedNonce.toString()).c_str());
	bytes = send(s, send_buffer, strlen(send_buffer), 0);
	printf("\nSENDING NONCE...\n");
	
	// RECV ACK for nonce
	printf("\nWAITING FOR SERVER ACK...\n");
	memset(receive_buffer, 0, BUFFER_SIZE);
	      n = 0;
	      while (1) {
	//*******************************************************************
	//RECEIVE
	//*******************************************************************
	         bytes = recv(s, &receive_buffer[n], 1, 0);

#if defined __unix__ || defined __APPLE__  
		     if ((bytes == -1) || (bytes == 0)) {
	            printf("recv failed\n");
	         	exit(1);
	         }   
      
#elif defined _WIN32      
             if ((bytes == SOCKET_ERROR) || (bytes == 0)) {
	            printf("recv failed\n");
	         	exit(1);
	         }
#endif

	         if (receive_buffer[n] == '\n') {  /*end on a LF*/
	            receive_buffer[n] = '\0';
	            break;
	         }
	         if (receive_buffer[n] != '\r') n++;   /*ignore CR's*/
	      }

	      printf("\nACK FOR NONCE RECEIVED...\n");


	
//*******************************************************************
//Get input while user don't type "."
//*******************************************************************
	printf("\n--------------------------------------------\n");
	printf("you may now start sending commands to the <<<SERVER>>>\n");
	
	printf("\nType here:");
	memset(&temp_buffer,0,BUFFER_SIZE);
    if(fgets(temp_buffer,SEGMENT_SIZE,stdin) == NULL){
		printf("error using fgets()\n");
		exit(1);
	}
    
	while ((strncmp(temp_buffer,".",1) != 0)) {
		   temp_buffer[strlen(temp_buffer)-1]='\0';//strip '\n'
		   printf("Message length: %d \n",(int)strlen(temp_buffer));



	//*******************************************************************
	//OUR ADDITIONS
	//*******************************************************************

		   char binary_buffer[BUFFER_SIZE*5];
		   memset(binary_buffer, 0, BUFFER_SIZE*5);
		   
		   binary_buffer[0] = '\0';
		   for(int i = 0; i < strlen(temp_buffer); i++){
		   	 	char a = temp_buffer[i] ^ random;
		   	 	InfInt tempEncrypt = repeatSquare(a, eSERV, nSERV);
		   	 	char tempString[80];
		   	 	memset(tempString, 0, 80);
		   	 	sprintf(tempString, "%s ", (tempEncrypt.toString()).c_str());
		   	 	strcat(binary_buffer, tempString);
		   	 	//
		   	 	char randomChar = tempString[0];
		   	 	random = randomChar;
		   }
	      
	       	strcat(binary_buffer, "");
	       	strcat(binary_buffer,"\r\n");
	       	sprintf(send_buffer, "%s", binary_buffer);

	//*******************************************************************
	//SEND
	//*******************************************************************

	       bytes = send(s, send_buffer, strlen(send_buffer),0);
	       printf("\nMSG SENT     --->>>: %s\n",send_buffer); //line sent 
	       
	       

#if defined __unix__ || defined __APPLE__     
          if (bytes == -1) {
	         printf("send failed\n");
    		 exit(1);
	      }
#elif defined _WIN32      
      	  if (bytes == SOCKET_ERROR) {
	         printf("send failed\n");
    		 WSACleanup();
	      	 exit(1);
	      }
#endif
	      memset(receive_buffer, 0, BUFFER_SIZE);
	      n = 0;
	      while (1) {
	//*******************************************************************
	//RECEIVE
	//*******************************************************************
	         bytes = recv(s, &receive_buffer[n], 1, 0);

#if defined __unix__ || defined __APPLE__  
		     if ((bytes == -1) || (bytes == 0)) {
	            printf("recv failed\n");
	         	exit(1);
	         }   
      
#elif defined _WIN32      
             if ((bytes == SOCKET_ERROR) || (bytes == 0)) {
	            printf("recv failed\n");
	         	exit(1);
	         }
#endif

	         if (receive_buffer[n] == '\n') {  /*end on a LF*/
	            receive_buffer[n] = '\0';
	            break;
	         }
	         if (receive_buffer[n] != '\r') n++;   /*ignore CR's*/
	      }
	      
	      printf("MSG RECEIVED --->>>: %s\n",receive_buffer);
			
		  //get another user input
	      memset(&temp_buffer, 0, BUFFER_SIZE);
          printf("\nType here:");
	      if(fgets(temp_buffer,SEGMENT_SIZE,stdin) == NULL){
		     printf("error using fgets()\n");
		     exit(1);
	     }
	}
	printf("\n--------------------------------------------\n");
	printf("<<<CLIENT>>> is shutting down...\n");

//*******************************************************************
//CLOSESOCKET   
//*******************************************************************
#if defined __unix__ || defined __APPLE__
    close(s);//close listening socket
#elif defined _WIN32
    closesocket(s);//close listening socket
    WSACleanup(); /* call WSACleanup when done using the Winsock dll */  
#endif
   return 0;
}

