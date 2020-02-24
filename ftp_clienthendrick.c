#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <unistd.h>

char args[3][50];
int datacount = 0;

void parse(char* str){
    int j = 0;
    int i = 0;
    datacount = 0;

    for(i = 0; i < strlen(str); i++) {
        if (str[i] == ' ') {
            args[datacount][j] = '\0';
            datacount++;
            j = 0;
        }
        else {
            args[datacount][j] = str[i];
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
    char fBuff[256];//file contents
    FILE *fPoint; //file pointer
    unsigned long fSize = 0; //file size

    int run = 1;
    while (run) {
        int needconnection = 1;
        while (needconnection) {
            printf("Type CONNECT 'servername' 'serverport' to connect" 
            " to a server.\n");
            fgets(input, 255, stdin);
            input[strlen(input) - 1] = '\0';
            parse(input);
            
            if(strcmp(args[0],"CONNECT") == 0 && datacount == 3) {
                 portno = atoi(argv[2]);
                 sockfd = socket(AF_INET, SOCK_STREAM, 0);
                 if (sockfd < 0){ 
                     perror("ERROR opening socket");
                 }
                 server = gethostbyname(argv[1]);
                 if (server == NULL) {
                     fprintf(stderr,"ERROR, no such host\n");
                 }
                 bzero((char *) &serv_addr, sizeof(serv_addr));
                 serv_addr.sin_family = AF_INET;
    		 bcopy((char *)server->h_addr, 
                     (char *)&serv_addr.sin_addr.s_addr,server->h_length);
                 serv_addr.sin_port = htons(portno);
                 if (connect(sockfd,(struct sockaddr *)&serv_addr,
                     sizeof(serv_addr)) < 0)
                 {                     
                     perror("ERROR connecting");
                 }
                 else {
                     needconnection = 0;
                 }
            }
            else {
                 printf("Incorrect input\n");
            }
        }
        
        printf("Connection has been established.\n");
        int inconnection = 1;
        while (inconnection) {

            printf("Please input command: ");
            fgets(input, 255, stdin);
            input[strlen(input) - 1] = '\0';
            parse(input);

            if(strcmp(args[1],"LIST") == 0 && datacount == 1) {
                 printf("List files in current directory of server\n");
		 n = write(sockfd,"LIST",strlen("LIST"));
    		 if (n < 0) {
                     perror("ERROR writing to socket");
		 }	 
		 bzero(buffer,256);
		 n = read(sockfd,buffer,1000);
		 if (n < 0) {
		     perror("ERROR reading from socket");
		 }	 
		 printf("%s\n",buffer);
            }
            else if(strcmp(args[1],"RETRIEVE") == 0 && datacount == 2) {
                 printf("Get file from server\n");
            }
            else if(strcmp(args[1],"STORE") == 0 && datacount == 2) {
                 printf("Send file to server to store.\n");
                 fPoint = fopen(args[1], "r"); //open text file
                 if (fPoint = NULL) {
                     printf("Error opening file.\n");
                 }
                 else {
                     //signal server for operation
                     write(sockfd, "STORE", 5);
	             bzero(cBuff, 256);
		     read(sockfd, cBuff, 255);
                     write(sockfd, args[1], sizeof(args[1]));
		     //check file name received/opened
		     bzero(cBuff, 256);
	             read(sockfd, cBuff, 255);
		 }
		 if (strcmp(cBuff, "Name Received") == 0) 
		 {
		     //send size of file
	             fseek(fPoint, 0, SEEK_END);
		     fSize = ftell(fPoint); //get current file pointer
		     fseek(fPoint, 0, SEEK_SET);
		     sprintf(cBuff, "%d", fSize);
		     write(sockfd, cBuff, strlen(cBuff));//send size of file
		     bzero(cBuff, 256);
	             read(sockfd, cBuff, 255); //get ack
                     while(fgets(fBuff, 256, fPoint) != NULL) {
                         write(sockfd, fBuff, sizeof(fBuff));
                     }
                     printf("File Sent.\n");
                 }
		 else
		 {
	             printf("Operation cancelled. Server-side file error detected.\n");
		 }
                 fclose(fPoint);//close file
            }
            else if(strcmp(args[1],"QUIT") == 0 && datacount == 1) {
                 printf("Close Socket Connection.\n");
                 n = write(sockfd,"QUIT", strlen("QUIT"));
                 if (n < 0){ 
		     perror("ERROR writing to socket");
                 }
                 inconnection = 0;
                 close(sockfd);
            }						
            else {
                printf("Incorrect input\n");
            }
         }
    }
    return 0;
}  
