#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX 1001

extern void client_customer(int );
extern void client_police(int );
extern void client_admin(int );

void init_zero(char buff[], int size){
    memset(buff, 0, size);
}

void show_error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, port, n;
    char buffer[MAX];
    
    struct sockaddr_in server_addr;
    struct hostent *server;

    if (argc < 3) {
       fprintf(stderr,"Usage: %s hostname port\n", argv[0]);
       exit(0);
    }

    port = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        show_error("ERROR in opening socket");
    
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR: Host not found\n");
        exit(0);
    }
    
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port);
    
    // connecting to server
    if (connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) 
        show_error("ERROR in connecting");
        
    char username[MAX];
    char password[MAX];
    char user_type;
    while(1)
    {
        // taking credentials from user   
        printf("Enter your credentials:\n");
        init_zero(username,MAX);
        printf("Enter Username: ");
        fgets(username, MAX, stdin);
            
        init_zero(password,MAX);
        printf("Enter Password: ");
        fgets (password, MAX, stdin);
        
        init_zero(buffer,MAX);
        strcat(buffer,username);
        strcat(buffer,"$#$");
        strcat(buffer,password);

        // sending it to server
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) 
            show_error("ERROR writing to socket");
        
        // false or exit or success (gives user type)
        init_zero(buffer,MAX);
        n = read(sockfd,buffer,MAX-1);
        if (n < 0) 
            show_error("ERROR reading from socket");
        
        if(!strcmp(buffer,"exit"))
        {
            printf("You entered the invalid credentials 3 times. Exiting...\n");
            return 0;
        }
        
        if(strcmp(buffer,"false"))
        {
            user_type = buffer[0];
            break;
        }
    }
    
    /* welcome to the bank */
    if(user_type=='C')
    {
        printf("Welcome Bank Customer.\n");
        client_customer(sockfd);
    }
    else if(user_type=='A')
    {
        printf("Welcome Bank Admin.\n");
        client_admin(sockfd);
    }
    else if(user_type=='P')
    {
        printf("Welcome Police.\n");
        client_police(sockfd);
    }

    // close the socket
    close(sockfd);  
    return 0;
}