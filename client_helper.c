#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX 1001

extern void show_error(char *);
extern void init_zero(char buff[], int size);

void client_police(int sockfd)
{
    char buffer[MAX],user_id[MAX],operation[MAX];
    int n;
    char flag;
    printf("*************************************\n");
    printf("Do u want to continue (y/n): ");
    scanf("%c",&flag);
    getchar();
    
    while(flag=='y')
    {
        init_zero(buffer,MAX);
        buffer[0] = flag;
        buffer[1] = '\0';
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) 
            show_error("ERROR writing to socket");

        /* input for services */
        printf("Enter User ID of Customer: ");
        init_zero(user_id,MAX);
        fgets (user_id, MAX, stdin);

        printf("Operation types: ");
        printf("\n\t\u2022 balance");
        printf("\n\t\u2022 mini_statement");
        printf("\nChoose type: ");
        init_zero(operation,MAX);
        fgets (operation, MAX, stdin);
        
        init_zero(buffer,MAX);
        strcat(buffer,operation);
        strcat(buffer,"#$#");
        strcat(buffer,user_id);

        // sending command
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) 
            show_error("ERROR writing to socket");

        operation[strlen(operation)-1] = '\0';

        // true or false
        init_zero(buffer,MAX);
        n = read(sockfd,buffer,MAX-1);
        if (n < 0) 
            show_error("ERROR reading from socket");
        
        if(!strcmp(buffer,"0"))
        {   
            printf("////////////////////////////////\n");
            printf("******* Invalid Username *******\n");
            printf("////////////////////////////////\n");
        }
        else if(!strcmp(buffer,"1"))
        {   
            printf("////////////////////////////////\n");
            printf("**** Not a valid customer ****\n");
            printf("////////////////////////////////\n");
        } 
        else if(!strcmp(buffer,"2"))
        {   
            printf("////////////////////////////////\n");
            printf("******* Invalid operation *******\n");
            printf("////////////////////////////////\n");
        }    
        else if(!strcmp(buffer,"true"))
        {
            if(!strcmp(operation,"balance"))
            {
            	// delimeter string
                init_zero(buffer,MAX);
                strcpy(buffer,"content");
                n = write(sockfd,buffer,strlen(buffer));
                if (n < 0) 
                    show_error("ERROR writing to socket");
                // balance
                init_zero(buffer,MAX);
                n = read(sockfd, buffer, MAX-1);
                if (n < 0) 
                    show_error("ERROR reading from socket");
                printf("BALANCE: %s\n\n", buffer);   
            }
            else if(!strcmp(operation,"mini_statement"))
            {
            	// delimeter string
                init_zero(buffer,MAX);
                strcpy(buffer,"size");
                n = write(sockfd,buffer,strlen(buffer));
                if (n < 0) 
                    show_error("ERROR writing to socket");
                init_zero(buffer,MAX);
                
                // file size
                n = read(sockfd, buffer, MAX-1);
                if (n < 0) 
                    show_error("ERROR reading from socket");
                
                int file_size = atoi(buffer);
                int remain_data = file_size;
                
                // delimeter string
                init_zero(buffer,MAX);
                strcpy(buffer,"content");
                n = write(sockfd,buffer,strlen(buffer));
                if (n < 0) 
                    show_error("ERROR writing to socket");

                // mini statement
                printf("\nMINI STATEMENT: \n");
                printf("----------------------------------\n");
                printf("DATE \tTYPE \tAMOUNT \n");
                printf("----------------------------------\n");
                init_zero(buffer,MAX);
                while ((remain_data > 0) && ((n = read(sockfd, buffer, MAX)) > 0))
                {
                    printf("%s", buffer);
                    remain_data -= n;
                    init_zero(buffer,MAX);
                }
                printf("\n\n");
            }
        }
        printf("*************************************\n");
        printf("Do u want to continue (y/n): ");
        scanf("%c",&flag);
        getchar();
    }
    // sending flag
    init_zero(buffer,MAX);
    buffer[0] = flag;
    buffer[1] = '\0';
    n = write(sockfd,buffer,strlen(buffer));  
    printf("THANK YOU!! Exiting..\n");
}

void client_customer(int sockfd)
{
    char buffer[MAX];
    char operation[MAX];
    int n;
    char flag;
    printf("*************************************\n");
    printf("Do u want to continue (y/n): ");
    scanf("%c",&flag);
    getchar();
    
    while(flag=='y')
    {
        init_zero(buffer,MAX);
        buffer[0] = flag;
        buffer[1] = '\0';
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) 
            show_error("ERROR writing to socket");
        printf("\nOPERATION COICES: ");
        printf("\n\t\u2022 balance");
        printf("\n\t\u2022 mini_statement");
        printf("\nENTER OPERATION: ");
        init_zero(operation,MAX);
        fgets (operation, MAX, stdin);
        
        // sending command
        n = write(sockfd,operation,strlen(operation));
        if (n < 0) 
            show_error("ERROR writing to socket");
        
        operation[strlen(operation)-1] = '\0';

        // true or false
        init_zero(buffer,MAX);
        n = read(sockfd,buffer,MAX-1);
        if (n < 0) 
            show_error("ERROR reading from socket");
        
        if(!strcmp(buffer,"false"))
        {   
            printf("////////////////////////////////\n");
            printf("******* Invalid Operation ******\n");
            printf("////////////////////////////////\n");
        }    
        else if(!strcmp(buffer,"true"))
        {
            if(!strcmp(operation,"balance"))
            {
                // delimiter string
                init_zero(buffer,MAX);
                strcpy(buffer,"content");
                n = write(sockfd,buffer,strlen(buffer));
                if (n < 0) 
                    show_error("ERROR writing to socket");
                // balance
                init_zero(buffer,MAX);
                n = read(sockfd, buffer, MAX-1);
                if (n < 0) 
                    show_error("ERROR reading from socket");
                printf("BALANCE: %s\n\n", buffer);   
            }
            else if(!strcmp(operation,"mini_statement"))
            {
                // delimeter string
                init_zero(buffer,MAX);
                strcpy(buffer,"size");
                n = write(sockfd,buffer,strlen(buffer));
                if (n < 0) 
                    show_error("ERROR writing to socket");
                
                // file size 
                init_zero(buffer,MAX);
                n = read(sockfd, buffer, MAX-1);
                if (n < 0) 
                    show_error("ERROR reading from socket");
                
                int file_size = atoi(buffer);
                int remain_data = file_size;
                
                // delimeter string
                init_zero(buffer,MAX);
                strcpy(buffer,"content");
                n = write(sockfd,buffer,strlen(buffer));
                if (n < 0) 
                    show_error("ERROR writing to socket");

                // mini statement
                printf("\nMINI STATEMENT: \n");
                printf("----------------------------------\n");
                printf("DATE \tTYPE \tAMOUNT \n");
                printf("----------------------------------\n");
                init_zero(buffer,MAX);
                while ((remain_data > 0) && ((n = read(sockfd, buffer, MAX)) > 0))
                {
                    printf("%s", buffer);
                    remain_data -= n;
                    init_zero(buffer,MAX);
                }
                printf("\n");
            }
        }
        printf("*************************************");
        printf("\nDo u want to continue (y/n): ");
        scanf("%c",&flag);
        getchar();
    }
    // sending flag
    init_zero(buffer,MAX);
    buffer[0] = flag;
    buffer[1] = '\0';
    n = write(sockfd,buffer,strlen(buffer));  
    printf("THANK YOU!! Exiting..\n");
}

void client_admin(int sockfd)
{
	char buffer[MAX];
    char id[MAX],trans[MAX],amount[MAX];
    int n;
    char flag;
    printf("*************************************\n");
    printf("Do u want to continue (y/n): ");
    scanf("%c",&flag);
    getchar();
    
    while(flag=='y')
    {
        init_zero(buffer,MAX);
        buffer[0] = flag;
        buffer[1] = '\0';
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) 
            show_error("ERROR writing to socket");

        printf("User ID of Customer: ");
        init_zero(id,MAX);
        fgets (id, MAX, stdin);
        
        printf("Transaction Types: ");
        printf("\n\t\u2022 credit");
        printf("\n\t\u2022 debit");
        printf("\nChoose type: ");
        init_zero(trans,MAX);
        fgets (trans, MAX, stdin);

        printf("Enter amount: ");
        init_zero(amount,MAX);
        fgets (amount, MAX, stdin);

        init_zero(buffer,MAX);
        strcat(buffer,id);
        strcat(buffer,"#$#");
		strcat(buffer,trans);
        strcat(buffer,"#$#");
        strcat(buffer,amount);
        strcat(buffer,"#$#");

        // sending command
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) 
            show_error("ERROR writing to socket");
        
        // true or false
        init_zero(buffer,MAX);
        n = read(sockfd,buffer,MAX-1);
        if (n < 0) 
            show_error("ERROR reading from socket");
        
        if(!strcmp(buffer,"0"))
        {   
            printf("////////////////////////////////\n");
            printf("******* Invalid user ID *******\n");
            printf("////////////////////////////////\n");
        }
        else if(!strcmp(buffer,"1"))
        {   
            printf("////////////////////////////////\n");
            printf("***** Not a valid customer *****\n");
            printf("////////////////////////////////\n");
        }
        else if(!strcmp(buffer,"2"))
        {   
            printf("////////////////////////////////\n");
            printf("*** Invalid transaction type ***\n");
            printf("////////////////////////////////\n");
        }
        else if(!strcmp(buffer,"3"))
        {   
            printf("////////////////////////////////\n");
            printf("******** Invalid amount ********\n");
            printf("////////////////////////////////\n");
        }
        else if(!strcmp(buffer,"true"))
        {   
            printf("////////////////////////////////\n");
            printf("**** Transaction successful ****\n");
            printf("////////////////////////////////\n");
        }
        else if(!strcmp(buffer,"deficit"))
        {   
            printf("////////////////////////////////\n");
        	printf("***** Insufficient Amount *****\n");
            printf("////////////////////////////////\n");
        }
        printf("*************************************\n");
        printf("Do u want to continue (y/n): ");
        scanf("%c",&flag);
        getchar();
    }
    // sending flag
    init_zero(buffer,MAX);
    buffer[0] = flag;
    buffer[1] = '\0';
    n = write(sockfd,buffer,strlen(buffer));  
    printf("THANK YOU!! Exiting..\n");
}