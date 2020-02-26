//By Robert Gardner, Reuben Nyenhuis, Matt Hendrick, and John Taube
//2/25/2020
//The purpose of this code is act as the client end for an FTP application, by reading in commands
//and acting on them
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

void fStore(int,char *);

char args[3][50];
int datacount=0;

//Function used to parse user input
void parse(char* str)
{
  int j=0;
  int i=0;
  datacount=0;
  //Checks for spaces to properly parse arguments
  for(i=0;i<=strlen(str);i++)
  {
    if(str[i]==' ')
    {
      args[datacount][j]='\0';
      datacount++;
      j=0;
    }
    else
    {
      args[datacount][j]=str[i];
      j++;
    }
  }
      datacount++;
}


int main(int argc, char *argv[])
{
  int sockfd, portno, n;

  struct sockaddr_in serv_addr;
  struct hostent *server;

  char buffer[256];
  char input[256];
  char cBuff[256];//communication buffer
  FILE *fPoint; //file pointer
  unsigned long fSize=0;//file size
  int run = 1;
 
  //Run this loop while the user is not yet connected 
  while(run){      
    int needconnection = 1;
    while(needconnection){
    printf("Type CONNECT 'servername' 'serverport' to" 
        " connect to a server.\n");
    fgets(input,255,stdin);
    input[strlen(input) - 1] = '\0';
    parse(input);

    if(strcmp(args[0],"CONNECT")==0 && datacount== 3)
    {
       portno = atoi(args[2]);
       sockfd = socket(AF_INET, SOCK_STREAM, 0);
       if (sockfd < 0) 
	 perror("ERROR opening socket");
	server = gethostbyname(args[1]);
	if (server == NULL) 
	   fprintf(stderr,"ERROR, no such host\n");
	 
	 bzero((char *) &serv_addr, sizeof(serv_addr));
	 serv_addr.sin_family = AF_INET;
	 bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,
		server->h_length);
	 serv_addr.sin_port = htons(portno);
         //Throw error if connect attempt does not work
	 if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
	    perror("ERROR connecting");
	 else needconnection=0; //We are now connected
    }
    else printf("Incorrect input\n"); 
 
  }

  printf("Connection has been established.\n");
  int inconnection=1; //Set to connected
  //Run this loop while we are connected
  while(inconnection)
  {
    printf("Please input command: ");
    fgets(input,255,stdin);
    input[strlen(input) - 1] = '\0';
    parse(input);

    //This command will list directory's files on the screen
    if(strcmp(args[0],"LIST")==0 && datacount== 1)
    {
      printf("Files in current directory of server:\n");
      n = write(sockfd,"LIST",strlen("LIST"));
      if (n < 0) 
        perror("ERROR writing to socket"); //Print if empty directory
      
      bzero(buffer,256);
      n = read(sockfd,buffer,1000);
      if (n < 0) 
        perror("ERROR reading from socket");
      printf("%s\n",buffer);
    }
    
    //Retrieve files from the server
    else if(strcmp(args[0],"RETRIEVE")==0 && datacount== 2)
    {
      printf("Get file from server\n");
      write(sockfd, "RETR",4);
      bzero(cBuff,256);
      read(sockfd,cBuff,255);
      write(sockfd, args[1], sizeof(args[1]));//send file name to server
      fStore(sockfd,args[1]);
    }
	
    //send file to server for storage
    else if(strcmp(args[0],"STORE")==0 && datacount== 2)
    {
      printf("Send file to server to store.\n");
      fPoint = fopen(args[1],"rb");//open text file, in read binary mode
      if (fPoint == NULL) //if file open error
      {
	printf("Error opening file.\n"); //notify user of file open error
      }
      else
      {
	//signal server for operation
	write(sockfd, "STORE",5); //notify server of command
	bzero(cBuff,256); //clear commmunication buffer
        //read ack from server (fixes consecutive write short msg merge)
	read(sockfd,cBuff,255); 
	write(sockfd, args[1], sizeof(args[1]));//send file name to server
        //check file name received/opened
	bzero(cBuff,256); //clear communication buffer
        //read ack for receiving/opening of file on server side
	read(sockfd,cBuff,255); 
	//check message confirming correct opening
        if(strcmp(cBuff,"Name received")==0)
        {
          //send size of file
          fseek(fPoint,0, SEEK_END); //seek end of file
          fSize = ftell(fPoint); //get curent file pointer
          fseek(fPoint, 0, SEEK_SET); //seek start of file
          sprintf(cBuff,"%ld",fSize); //put size of file in buffer
          write(sockfd, cBuff, strlen(cBuff));//send size of file
          bzero(cBuff,256); //clear communication buffer
          //get ack for size (fixes consecutive write short msg merge)
          read(sockfd,cBuff,255);
   	  //write data to server until end of file
          while(fgets(cBuff,255,fPoint) != NULL)
          {
            write(sockfd, cBuff, strlen(cBuff));//send file data
          }
            printf("File Sent.\n"); //notify user of file sent
        }
        else
        {
          //notify user of server-side error
          printf("Operation cancelled. Server-side file error detected.\n");
        }
        fclose(fPoint);//close file
      }
	}
	else if(strcmp(args[0],"QUIT")==0 && datacount== 1)
        {
	  printf("Close Socket Connection.\n");
	  n = write(sockfd,"QUIT",strlen("QUIT"));
    	  if (n < 0) 
            perror("ERROR writing to socket");
	   inconnection=0; //Set to disconnected
	   close(sockfd); //Close connection
	}
	else printf("Incorrect input\n");
      }
      
    }
    
    return 0;
}

void fStore (int sock, char* fName)
{
  printf("STORE FUNCTION accessed. FIlename %s\n",fName);
  char rBuff[256];//receive buffer
  FILE *fPoint;//file pointer
  long int fSize = 0; //size of file
  long int count = 0;//count bytes of file delivered

  fPoint = fopen(fName, "wb");//open file with name
  if (fPoint == NULL)
  {
    printf("Error opening file.\n");
    write(sock, "Server file error",17);//file open error
  }
  else
  {
    bzero(rBuff,256);
    read(sock, rBuff, 255);//read file length
    write(sock, "File length received",20);//file length ack
    fSize = atol(rBuff);
    //write through file until length of file completed.
    while(count<fSize)
    {
      bzero(rBuff,256);
      read(sock,rBuff,255);
      count+=strlen(rBuff);
      fprintf(fPoint, "%s", rBuff);
    }
    printf("File received.\n");
    fclose (fPoint);//close file
  }
}	
