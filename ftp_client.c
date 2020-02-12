#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
  int sockfd, portno, n, i, j, cnt;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    char connection[256];
    char args[3][50];

    int needconnection = 1;
    while(needconnection==1){
      printf("Type CONNECT 'servername' 'serverport' to connect to a server.\n");
      fgets(connection,255,stdin);
      connection[strlen(connection) - 1] = '\0';

      j=0;
      cnt=0;
      for(i=0;i<=strlen(connection);i++){
	if(connection[i]==' '){
	  args[cnt][j]='\0';
	  cnt++;
	  j=0;
	}
	else{
	  args[cnt][j]=connection[i];
	  j++;
	}
      }

      if(strcmp(args[0],"CONNECT")==0 && cnt == 2)
        needconnection=0;
      else printf("Incorrect input\n");
 
    }
    
    
    
    //if (argc < 3) {
    //fprintf(stderr,"usage %s hostname port\n", argv[0]);
    //exit(0);
    //}
    portno = atoi(args[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(args[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    return 0;
}
