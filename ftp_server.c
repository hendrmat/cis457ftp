/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
   gcc server2.c -lsocket
*/

// Adding list functionality soon, establishing hendrick branch

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void dostuff(int); /* function prototype */
void fSend(int);
void error(char *msg)
{
    perror(msg);
    exit(1);
}

void showlist(char* list)
{
    int num = 0;
    char line[num];
    FILE *list = fopen("list.txt","r");
    if (list == NULL) {
        printf("No files in this directory");
    }
    while(!eof(list) {
        line[num] = fgets(dirname);
        num++;
    }
    fclose(list);
    send(sock, dirbuffer, 256, 0);
}
            
int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen, pid, status, run;
     struct sockaddr_in serv_addr, cli_addr;
  
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0){ 
        error("ERROR opening socket");
     }
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY; //inet_addr("192.168.110.7");
     serv_addr.sin_port = htons(6550);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) {
              error("ERROR on binding");
     }
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     run = 1;
     while (run) {
         newsockfd = accept(sockfd, 
               (struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0) { 
             error("ERROR on accept");
         }
         pid = fork();
         if (pid < 0){
             error("ERROR on fork");
         }
         if (pid == 0)  {
             close(sockfd);
             dostuff(newsockfd);
             exit(0);
         }
         else {
             wait (&status);
             close(newsockfd);
             run = 0;
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
   printf("Here is the message: %s\n",buffer);
   n = write(sock,"I got your message",18);
   if (n < 0) error("ERROR writing to socket");
}

void fSend (int sock)
{
   char fName[256];
   char fBuff[256];
   FILE *fPoint;
   read(sock, fName, 256);
   if (fPoint == NULL)
   {
      printf("Error opening file.\n");
   }
   else
   {
      while(read(sock, fBuff, 256) > 0)
      {
         fprintf(fPoint, "%s", fBuff);
      }
      printf("File received.");
   }
   fclose (fPoint);
}
