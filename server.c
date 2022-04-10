#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <stdlib.h>
#include <sys/sendfile.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX 1001

char *client_ip;
int client_port;

extern void server_customer(int, char*);
extern void server_police(int);
extern void server_admin(int);

void init_zero(char buff[], int size){
    memset(buff, 0, size);
}

void show_error(char *msg)
{
    perror(msg);
    exit(1);
}


int check_cred(int sockfd, char *user_type, char *cust_id)
{
    int n;
    char buffer[MAX];
    char *user, *pass;

    // reading username and password
    init_zero(buffer,MAX); 
    n = read(sockfd,buffer,MAX-1);
    if (n < 0) 
        show_error("ERROR reading from socket");
    
    // breaking it
    user = strtok(buffer, "$#$");
    pass = strtok(NULL, "$#$"); 
    
    user[strlen(user)-1] = '\0';
    pass[strlen(pass)-1] = '\0';

    if(strlen(user)==0 || strlen(pass)==0)
        return 0;

    // checking for validity
    FILE *fp = fopen("login_file.txt","r");
    if(fp == NULL)
        show_error("Error in opening login_file.");
    
    char *cred = NULL;
    size_t len = 0;
    
    while(getline(&cred,&len,fp)!=-1)
    {
        char *username = strtok(cred," ");
        char *password = strtok(NULL," ");
        char *usertype = strtok(NULL, " ");
        if(!strcmp(username,user)&&!strcmp(password,pass))
        {
            *user_type = usertype[0];
            strcat(cust_id, username); //-------------- INSERTING USER name in cust id
            // *cust_id = atoi(username);
            free(cred);
            fclose(fp);
            return 1;
        } 
    }
    fclose(fp);
    free(cred);
    return 0;
}


void login(int sockfd)
{
    int n;
    char buffer[MAX];
    
    int count=1;
    char user_type;
    char *cust_id;
    
    while(!check_cred(sockfd, &user_type, cust_id))
    {
        if(count>=3)
        {
            // sending exit
            init_zero(buffer,MAX);
            strcpy(buffer,"exit");
            n = write(sockfd,buffer,strlen(buffer));
            return;
        }

        fprintf(stdout, "Verification for client with ip address '%s' failed. \n", client_ip);            
        // sending false
        init_zero(buffer,MAX);
        strcpy(buffer,"false");
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) 
            show_error("ERROR writing to socket");
        count++;
    }
    
    fprintf(stdout, "Verification for client with ip address '%s' successful. \n", client_ip);            
    // sending user type
    init_zero(buffer,MAX);
    buffer[0] = user_type;
    buffer[1] = '\0';
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
        show_error("ERROR writing to socket");
    
    // calling corresponding function
    if(user_type == 'C')
        server_customer(sockfd,cust_id);
    else if(user_type == 'A')
        server_admin(sockfd);
    else if(user_type == 'P')
        server_police(sockfd);

    return;
}



int main(int argc, char *argv[])
{
    int sockfd, clientfd, port, client_len;
    int pid, process_id;
    int child_count = 0;
    int enable = 1;
    
    struct sockaddr_in server_addr, client_addr;
    
    if (argc < 2) 
    {
        fprintf(stderr,"ERROR: no port is provided.\n");
        exit(1);
    }
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) 
        show_error("ERROR in opening socket");
    
    init_zero((char *) &server_addr, sizeof(server_addr));
    
    port = atoi(argv[1]);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        show_error("Setting of socket as Reusable failed.");

    // binding socket to address
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) 
        show_error("ERROR in binding.");
    
    fprintf(stdout, "Binding done.\n");

    // socket listening
    listen(sockfd,5);
    fprintf(stdout, "Listening started with queue length 5.\n");

    client_len = sizeof(client_addr);
    
    while (1) 
    {
    	// accepting client request
        clientfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
        if (clientfd < 0) 
            show_error("ERROR in accepting connection.");
        pid = fork();
        if (pid == 0)  
        {
            close(sockfd);
            
            client_ip = inet_ntoa(client_addr.sin_addr);
            client_port = ntohs(client_addr.sin_port);
            fprintf(stdout, "Connection accepted for client with ip address '%s' on port '%d'. \n", client_ip, client_port);
            
            // handling client
            login(clientfd);
            
            close(clientfd);
            fprintf(stdout, "Connection closed for client with ip address '%s' on port '%d'. \n", client_ip, client_port);
            
            exit(0);
        }
        else if (pid < 0)
            show_error("ERROR in forking.");
        
        else 
        {
        	// closing client
            close(clientfd);
            // handling zombie process
            while (child_count) 
            {
                // Non-blocking wait 
                process_id = waitpid((pid_t) -1, NULL, WNOHANG); 
                if (process_id < 0) 
                    show_error("Error in cleaning zombie process.");
                else if(process_id == 0) 
                    break; 
                else 
                    child_count--; 
            }
        }
    } 
     return 0; 
}