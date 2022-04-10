#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define MAX 1001

// common functions
extern void show_error(char *);
extern char *client_ip;
extern void init_zero(char buff[], int size);

void server_police(int sockfd)
{
    int n;
    char buffer[MAX];
    char filename[100],op[MAX],id[MAX];

    // reading flag
    init_zero(buffer,MAX);
    n = read(sockfd,buffer,MAX-1);
    if (n < 0) 
        show_error("ERROR reading from socket");
    
    while(buffer[0]=='y')
    {
    	int check = 0;
        // reading command
        init_zero(buffer,MAX);
        n = read(sockfd,buffer,MAX-1);
        if (n < 0) 
            show_error("ERROR reading from socket");
        
        // breaking command into operation and user_id
        char *ptr = strtok(buffer,"#$#");
        strcpy(op,ptr);
        ptr = strtok(NULL,"#$#");
        strcpy(id,ptr);

        op[strlen(op)-1] = '\0';
        id[strlen(id)-1] = '\0';
        

        // checking for validity of user_id
        char *cred = NULL;
        size_t len = 0;
        
        FILE *fp = fopen("login_file.txt","r");
        if(fp == NULL)
            show_error("Error in opening login_file.");
        
        while(getline(&cred,&len,fp)!=-1)
        {
            char *username = strtok(cred," ");
            strtok(NULL," ");
            char *usertype = strtok(NULL, " ");
            
            if(!strcmp(username,id))
            {
                check=1;
                if(usertype[0]=='C')
                {
                    check=2;
                }
                break;
            } 
        }
        free(cred);
        fclose(fp);
        
        // sending false
        if(check==0 || check==1 || (strcmp(op,"balance") && strcmp(op,"mini_statement")))
        {
            if(check == 0){
                fprintf(stdout, "Request from client with ip '%s' declined. \n", client_ip);    
                init_zero(buffer,MAX);
                strcpy(buffer,"0");
                n = write(sockfd,buffer,strlen(buffer));  
                if (n < 0) 
                    show_error("ERROR writing to socket");
            }
            else if(check == 1){
                fprintf(stdout, "Request from client with ip '%s' declined. \n", client_ip);    
                init_zero(buffer,MAX);
                strcpy(buffer,"1");
                n = write(sockfd,buffer,strlen(buffer));  
                if (n < 0) 
                    show_error("ERROR writing to socket");
            }
            else if((strcmp(op,"balance") && strcmp(op,"mini_statement"))){
                fprintf(stdout, "Request from client with ip '%s' declined. \n", client_ip);    
                init_zero(buffer,MAX);
                strcpy(buffer,"2");
                n = write(sockfd,buffer,strlen(buffer));  
                if (n < 0) 
                    show_error("ERROR writing to socket");
            }
        }
        else
        {
            if(!strcmp(op,"balance"))
            {
            	// sending true
                init_zero(buffer,MAX);
                strcpy(buffer,"true");
                n = write(sockfd,buffer,strlen(buffer));  
                if (n < 0) 
                    show_error("ERROR writing to socket");
                
                // delimeter string
                init_zero(buffer,MAX);
                n = read(sockfd,buffer,MAX-1);
                if (n < 0) 
                    show_error("ERROR reading from socket");  
                
                show_balance(sockfd,id);
            }
            else if(!strcmp(op,"mini_statement"))
            {
                // sending true
                init_zero(buffer,MAX);
                strcpy(buffer,"true");
                n = write(sockfd,buffer,strlen(buffer));  
                if (n < 0) 
                    show_error("ERROR writing to socket");
                
                // delimeter string
                init_zero(buffer,MAX);
                n = read(sockfd,buffer,MAX-1);
                if (n < 0) 
                    show_error("ERROR reading from socket");  
                
                show_statement(sockfd,id); 
            }
        }
        /* Reading flag */
        init_zero(buffer,MAX);
        n = read(sockfd,buffer,MAX-1);
        if (n < 0) 
            show_error("ERROR reading from socket");
    }
    
}

void show_balance(int sockfd,char *cust_id)
{
    int n;
    char filename[MAX];
    sprintf(filename,"%s",cust_id);
    strcat(filename,".txt");
    
    FILE *fp = fopen(filename,"r");
    if(fp == NULL)
        show_error("Error in opening user file for balance.");
    
    char *transaction = NULL;
    size_t len = 0;
    char *balance;

    while(getline(&transaction,&len,fp)!=-1)
    {
        strtok(transaction," ");
        strtok(NULL, " ");
        balance = strtok(NULL, " ");
    }
    
    fprintf(stdout, "Sending balance of customer '%s' to client with ip '%s'. \n", cust_id, client_ip);
    // balance
    n = write(sockfd,balance,strlen(balance));
    if (n < 0) 
        show_error("ERROR writing to socket");
    
    free(transaction);
    fclose(fp);
}

void show_statement(int sockfd, char *cust_id)
{
    int n;
    char filename[MAX];
    sprintf(filename,"%s",cust_id);
    strcat(filename,".txt");
    struct stat file_stat;
    char buffer[MAX];

    
    int fd = open(filename, O_RDONLY);
    if(fd == -1)
        show_error("Error in opening user file for mini_statement");
    
    // finding stats of file
    if (fstat(fd, &file_stat) < 0)
        show_error("Error in getting statistics of file.");

    // writing size of file
    init_zero(buffer,MAX);
    sprintf(buffer, "%d", (int)file_stat.st_size);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
        show_error("ERROR writing to socket");
    
    // delimeter string
    init_zero(buffer,MAX);
    n = read(sockfd,buffer,MAX-1);
    if (n < 0) 
        show_error("ERROR reading from socket");
    
    // sending mini statement        
    fprintf(stdout, "Sending mini statement of customer '%s' to client with ip '%s'. \n", cust_id, client_ip);
    while (1) 
    {
        init_zero(buffer,MAX);
        int bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read == 0) 
            break;
        if (bytes_read < 0) 
            show_error("ERROR reading from file.");
        
        void *ptr = buffer;
        while (bytes_read > 0) {
            int bytes_written = write(sockfd, ptr, bytes_read);
            if (bytes_written <= 0) 
                show_error("ERROR writing to socket");
            bytes_read -= bytes_written;
            ptr += bytes_written;
        }
    }
    close(fd);         
}


void server_customer(int sockfd,char *cust_id)
{
    int n;
    char buffer[MAX];
    char id[MAX];
    // sprintf(id,"%d",cust_id);
    strcat(id, cust_id);

    /* Reading flag */
    init_zero(buffer,MAX);
    n = read(sockfd,buffer,MAX-1);
    if (n < 0) 
        show_error("ERROR reading from socket");
    
    while(buffer[0]=='y')
    {   
        // reading command
        init_zero(buffer,MAX);
        n = read(sockfd,buffer,MAX-1);
        if (n < 0) 
            show_error("ERROR reading from socket");
        
        buffer[strlen(buffer)-1] = '\0';
        
        if(!strcmp(buffer,"balance"))
        {
            // sending true
            init_zero(buffer,MAX);
            strcpy(buffer,"true");
            n = write(sockfd,buffer,strlen(buffer));  
            if (n < 0) 
                show_error("ERROR writing to socket");
            
            // delimeter string
            init_zero(buffer,MAX);
            n = read(sockfd,buffer,MAX-1);
            if (n < 0) 
                show_error("ERROR reading from socket");
            
            show_balance(sockfd,id);
        }
        else if(!strcmp(buffer,"mini_statement"))
        {
            // sending true
            init_zero(buffer,MAX);
            strcpy(buffer,"true");
            n = write(sockfd,buffer,strlen(buffer));  
            if (n < 0) 
                show_error("ERROR writing to socket");
            
            // delimeter string
            init_zero(buffer,MAX);
            n = read(sockfd,buffer,MAX-1);
            if (n < 0) 
                show_error("ERROR reading from socket");  
            
            show_statement(sockfd,id); 
        }
        else
        {
            fprintf(stdout, "Request from client with ip '%s' declined. \n", client_ip);    
            // sending false
            init_zero(buffer,MAX);
            strcpy(buffer,"false");
            n = write(sockfd,buffer,strlen(buffer));  
            if (n < 0) 
                show_error("ERROR writing to socket");
        } 
        /* Reading flag */
        init_zero(buffer,MAX);
        n = read(sockfd,buffer,MAX-1);
        if (n < 0) 
            show_error("ERROR reading from socket");
    }
}

int is_valid(char *amount)
{
	// checking validity of amount
	int i;
	int count=0;
	for(i=0;amount[i];i++)
	{
		if(amount[i]=='.')
		{
			count++;
			if(count>1)
				return 0;
		}		
		else if(amount[i]<='9'&&amount[i]>='0')
			continue;
		else
			return 0;
	}
	return 1;
}



void credit_amount(char *id, char *amount,char *trans)
{
	char filename[MAX];
	sprintf(filename,"%s.txt",id);

	FILE *fp = fopen(filename,"r");
    if(fp == NULL)
        show_error("Error in opening user file for balance.");
    
    char *transaction = NULL;
    size_t len = 0;
    char *balance;

    while(getline(&transaction,&len,fp)!=-1)
    {
        strtok(transaction," ");
        strtok(NULL, " ");
        balance = strtok(NULL, " ");
    }

    double amt, amt_cred;
    sscanf(balance, "%lf", &amt);

    free(transaction);
    fclose(fp);

    // crediting amount
    sscanf(amount, "%lf", &amt_cred);
    amt += amt_cred;

    fp = fopen(filename,"a");
    if(fp == NULL)
        show_error("Error in opening user file for crediting.");
	
	time_t c_t = time(NULL);
	struct tm tm = *localtime(&c_t);
	fprintf(fp,"\n%.2d-%.2d-%.4d %s %f", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, trans, amt);
	fclose(fp);
}


int debit_amount(char *id, char *amount, char *trans)
{
	char filename[MAX];
	sprintf(filename,"%s.txt",id);

	FILE *fp = fopen(filename,"r");
    if(fp == NULL)
        show_error("Error in opening user file for balance.");
    
    char *transaction = NULL;
    size_t len = 0;
    char *balance;

    while(getline(&transaction,&len,fp)!=-1)
    {
        strtok(transaction," ");
        strtok(NULL, " ");
        balance = strtok(NULL, " ");
    }
    
    double amt, req_amt;
    sscanf(balance, "%lf", &amt);

    free(transaction);
    fclose(fp);
 
    sscanf(amount, "%lf", &req_amt);
    if(amt<req_amt)
    	return 0;

    // debiting amount
    amt -= req_amt;

    fp = fopen(filename,"a");
    if(fp == NULL)
        show_error("Error in opening user file for debiting.");
	
	time_t c_t = time(NULL);
	struct tm tm = *localtime(&c_t);
	fprintf(fp,"\n%.2d-%.2d-%.4d %s %f", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, trans, amt);
	fclose(fp);            
	return 1;
}

void server_admin(int sockfd)
{
    int n;
    char buffer[MAX];
    char id[MAX], trans[MAX], amount[MAX];

    /* Reading flag */
    init_zero(buffer,MAX);
    n = read(sockfd,buffer,MAX-1);
    if (n < 0) 
        show_error("ERROR reading from socket");
    
    while(buffer[0]=='y')
    {   
        // reading command
        init_zero(buffer,MAX);
        n = read(sockfd,buffer,MAX-1);
        if (n < 0) 
            show_error("ERROR reading from socket");
        
        init_zero(amount, MAX);
        init_zero(id, MAX);
        init_zero(trans, MAX);

        char *ptr = strtok(buffer,"#$#");
        strcpy(id,ptr);
        ptr = strtok(NULL, "#$#");
        strcpy(trans,ptr);
        ptr = strtok(NULL, "#$#");
        strcpy(amount,ptr);

        id[strlen(id)-1] = '\0';
        trans[strlen(trans)-1] = '\0';
        amount[strlen(amount)-1] = '\0';
        
        // checking for validity of user_id
        char *cred = NULL;
        size_t len = 0;
        int check = 0;
        FILE *fp = fopen("login_file.txt","r");
        if(fp == NULL)
            show_error("Error in opening login_file.");
        
        while(getline(&cred,&len,fp)!=-1)
        {   
            // taking username
            char *username = strtok(cred," ");
            // skipping password 
            strtok(NULL," ");
            // taking user type
            char *usertype = strtok(NULL, " ");
            
            if(!strcmp(username,id))
            {
                check=1;
                if(usertype[0]=='C')
                {
                    check=2;
                }
                break;
            } 
        }
        free(cred);
        fclose(fp);

        if(check==0 || check==1 || (strcmp(trans,"credit") && strcmp(trans,"debit")) || !is_valid(amount))
        {
            // sending show_error code
            if(check == 0){
                // invalid userid
                fprintf(stdout, "Request from client with ip '%s' declined. \n", client_ip);
                init_zero(buffer,MAX);
                strcpy(buffer,"0");
                n = write(sockfd,buffer,strlen(buffer));  
                if (n < 0) 
                    show_error("ERROR writing to socket");
            }
            else if(check == 1){
                // invalid usertype
                fprintf(stdout, "Request from client with ip '%s' declined. \n", client_ip);
                init_zero(buffer,MAX);
                strcpy(buffer,"1");
                n = write(sockfd,buffer,strlen(buffer));  
                if (n < 0) 
                    show_error("ERROR writing to socket");
            }
            else if((strcmp(trans,"credit") && strcmp(trans,"debit"))){
                // invalid operation
                fprintf(stdout, "Request from client with ip '%s' declined. \n", client_ip);
                init_zero(buffer,MAX);
                strcpy(buffer,"2");
                n = write(sockfd,buffer,strlen(buffer));  
                if (n < 0) 
                    show_error("ERROR writing to socket");
            }
            else if(!is_valid(amount))
            {
                // invalid amount
                fprintf(stdout, "Request from client with ip '%s' declined. \n", client_ip);
                init_zero(buffer,MAX);
                strcpy(buffer,"3");
                n = write(sockfd,buffer,strlen(buffer));  
                if (n < 0) 
                    show_error("ERROR writing to socket");
            }
        }
        else
        {
        	if(!strcmp(trans,"credit"))
            {
            	credit_amount(id,amount,trans);
                fprintf(stdout, "Credit request from client with ip '%s' for customer '%s' successfully executed. \n", client_ip, id );
            	// sending true
                init_zero(buffer,MAX);
                strcpy(buffer,"true");
                n = write(sockfd,buffer,strlen(buffer));  
                if (n < 0) 
                    show_error("ERROR writing to socket");
            }
            else if(!strcmp(trans,"debit"))
            {
            	int f = debit_amount(id, amount, trans);
                // sending true
                if(f==1)
                {
                	fprintf(stdout, "Debit request from client with ip '%s' for customer '%s' successfully executed. \n", client_ip, id );
                    init_zero(buffer,MAX);
	                strcpy(buffer,"true");
	                n = write(sockfd,buffer,strlen(buffer));  
	                if (n < 0) 
	                    show_error("ERROR writing to socket");
                }
                else
                {
                	// insufficient amount
                	fprintf(stdout, "Debit request from client with ip '%s' declined. \n", client_ip);
                    init_zero(buffer,MAX);
	                strcpy(buffer,"deficit");
	                n = write(sockfd,buffer,strlen(buffer));  
	                if (n < 0) 
	                    show_error("ERROR writing to socket");
                } 
            }
        }
        /* Reading flag */
        init_zero(buffer,MAX);
        n = read(sockfd,buffer,MAX-1);
        if (n < 0) 
            show_error("ERROR reading from socket");
    }

}
