//By Robert Gardner, Reuben Nyenhuis, Matt Hendrick, and John Taube
//2/25/2020
//The purpose of this code is act as the server end for an FTP application, by opening a socket,
//reading commands from the client, and acting upon the commands

/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
   gcc server2.c -lsocket
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>

//function prototypes

void dostuff(int); 
void list(int);
void fSend(int);
void fStore(int); //function will store a file in the directory of the server, pass socket number
void error(char *msg)
{
    perror(msg);
}

int run;
int clientcon;

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, clilen, pid,status;
     struct sockaddr_in serv_addr, cli_addr;

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     //Throws error if not a valid socket
     if (sockfd < 0) 
     {
        error("ERROR opening socket");
     }
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY; //inet_addr("192.168.110.7");
     serv_addr.sin_port = htons(6550); //Our port number
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
     {
        error("ERROR on binding");
     }
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
 
     run =1;
     clientcon=0;
     while (run) 
     {
        if(!clientcon)
	{
          newsockfd = accept(sockfd, 
          (struct sockaddr *) &cli_addr, (int *) &clilen);
          if (newsockfd < 0) 
	    {
              error("ERROR on accept"); //Not connected
	    }
	  else 
	    {
              clientcon=1; //we are now connected
	    }
	}

        pid = fork();
        if (pid < 0) 
	{
           error("ERROR on fork");
	}
        if (pid == 0)  
	{
           close(sockfd);
           dostuff(newsockfd);
           exit(1);
        }
        else
	{
	   wait (&status);
	   clientcon = status;
	}
	
     } /* end of while */
     return 0; /* we never get here */
}

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void dostuff (int sock)
{
   int n;
   char buffer[256];
      
   bzero(buffer,256);
   n = read(sock,buffer,255);
   if (n < 0) error("ERROR reading from socket");

   if(strlen(buffer)<1)
   {
	close(sock);
	exit(0);
   }
   printf("BUFFER:%s.\n",buffer);
   //Call the list function if LIST is selected
   if(strcmp(buffer,"LIST")==0)
   {
	list(sock);
   }
   //Call the retrieve function if retrieve is selected
   if (strcmp(buffer,"RETR")==0) 
   {
        write(sock, "RETR received",13);
        fSend(sock);
   }
   //store command:receive a file from client and store in server directory
   if (strcmp(buffer,"STORE")==0) {
        write(sock, "STORE received",14); //ack command(fixes consecutive write short message merge)
        fStore(sock); //run function to store file, pass socket number
   }
   //Disconnect from the server if QUIT is selected
   if(strcmp(buffer,"QUIT")==0){
	close(sock);
	clientcon=0;
	exit(0);
   }
}

void list(int sock){
   char files[1000];
   int n;
   DIR *d;
   struct dirent *dir;
   d = opendir(".");
   if(d)
   {
	while((dir = readdir(d)) != NULL)
	{
	    strcat(files,dir->d_name);
	    strcat(files,"\n");
	}
	closedir(d);
   }
   
   n = write(sock,files,strlen(files));
   if (n < 0) 
   {
       error("ERROR writing to socket");
   }
}

//Send file to server
void fSend (int sock)
{
  char cBuff[256];//communication buffer
  FILE *fPoint; //file pointer
  char fName[256];//file name
  unsigned long fSize=0;//file size

  bzero(fName,256);
  read(sock, fName, 255);//read file name
  fPoint = fopen(fName,"rb");//open text file
  
  if (fPoint == NULL)
  {
    printf("Error opening file.\n");
  }
  else
  {
    //send size of file
    fseek(fPoint,0, SEEK_END);
    fSize = ftell(fPoint);//get curent file pointer
    fseek(fPoint, 0, SEEK_SET);
    sprintf(cBuff,"%ld",fSize);
    write(sock, cBuff, strlen(cBuff));//send size of file
    bzero(cBuff,256);
    read(sock,cBuff,255);//get ack
    while(fgets(cBuff,255,fPoint) != NULL)
    {
      write(sock, cBuff, strlen(cBuff));//send file data
    }
    printf("File Sent.\n");
    fclose(fPoint);//close file
  }
}

//store file from client
void fStore (int sock)
{
  char fName[256];//file name buffer
  char rBuff[256];//receive buffer
  FILE *fPoint;//file pointer
  long int fSize = 0; //size of file
  long int count = 0;//count bytes of file delivered
  bzero(fName,256);//clear file name buffer
  read(sock, fName, 255);//read file name
  fPoint = fopen(fName, "wb");//open file with name in write binary mode
  if (fPoint == NULL) //if file open error
  {
    printf("Error opening file.\n"); //notify user of file open error
    write(sock, "Server file error",17);//file open error
  }
  else
  {
    write(sock, "Name received",13);//file name read/open ack
    bzero(rBuff,256);//clear receive buffer
    read(sock, rBuff, 255);//read file length
    //file length ack(fixes consecutive write short message merge)
    write(sock, "File length received",20);
    fSize = atol(rBuff);//convert file size from buffer into number
    //write through file until length of file completed.
    while(count<fSize)
    {
      bzero(rBuff,256);//clear receive buffer
      read(sock,rBuff,255);//read data
      count+=strlen(rBuff);//count data length
      fprintf(fPoint, "%s", rBuff);//put data in new file
    }
    printf("File received.\n");//notify user of completion
    fclose (fPoint);//close file
  }
	
}
