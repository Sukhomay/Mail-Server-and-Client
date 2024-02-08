#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<regex.h>

#define MAX_BUFFER_SIZE 1024

// Function to receive one line commmands or response
void dataReceive(int _socket_, char *dataReceived);
void multiLineDataReceive(int _socket_, char *dataReceived);

// Function to send mail and handle corresponding responses and errors
void send_mail(int smtp_port, const char *server_IP, const char *username);
void format(char *s);
int checkFormat_1(char *str);
int checkFormat_2(const char *str, const char *domain);
int checkFormat_3(const char *str, const char *email);
int checkFormat_4(const char *str);
int checkFormat_5(const char *str);
int checkFormat_6(const char *str);
int checkFormat_7(const char *str);
int checkFormat_8(const char *str);

// Function to access mailbox and handle corresponding responses and errors
void manage_mail(int pop3_port, const char *server_IP, const char *username, const char *password);
void getMailList(int pop3_socket);
void getCompleteMail(int pop3_socket, int user_select);
void sendSTATToPOP3(int pop3_socket, int *numMailPtr, int *numTotalCharPtr);
void sendPasswordToPOP3(const int pop3_socket, const char* password);
void sendUsernameToPOP3(const int pop3_socket, const char* username);
void sendQUITToPOP3(int pop3_socket);
void deleteMail(int pop3_socket, int user_select);

int main(int argc, char *argv[]) 
{
    // Error in case of incorrect set of arguments
    if (argc != 4) 
    {
        fprintf(stderr, "Usage: %s <server_IP> <smtp_port> <pop3_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Get the server_IP that will indicate the IP address of the SMTP and POP3 server to connect to, 
    // smtp_port that identifies the port no. the SMTP server is running at, and 
    //pop3_port that identifies the port no. the POP3 server is running at.
    const char *serverIP = argv[1];
    int smtpPort = atoi(argv[2]);
    int pop3Port = atoi(argv[3]);

    // Get the username and password, and store it locally
    char username[100], password[100];
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);
    printf("\n---------------------------------------------------------------------\n");
    // User choice 
    int option;
    do {
        printf("\nPlease select from an option below:\n");
        printf("1. Manage Mail\n");
        printf("2. Send Mail\n");
        printf("3. Quit\n");
        scanf("%d", &option);

        switch (option) {
            case 1:
                // Implement manage mail functionality
                printf("Option Manage Mail selected.\n");
                manage_mail(pop3Port, serverIP, username, password);
                break;
            case 2:
                // Send mail functionality
                printf("Option Send Mail selected.\n");
                send_mail(smtpPort, serverIP, username);
                break;
            case 3:
                // User aks to quit
                printf("Option Quit selected.\n");
                printf("Quitting the program...\n");
                break;
            default:
                printf("Invalid option. Please try again.\n");
        }
    } while (option != 3);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void dataReceive(int _socket_, char *dataReceived)
{
    int endOfTextFlag=0;
    char buffer[MAX_BUFFER_SIZE];
    int i;
    for(int i=0; i<MAX_BUFFER_SIZE; i++)
    {
        dataReceived[i]='\0';
    }
    char c1 = '\0', c2 = '\0';
    while(1)
    {
        int bytesReceived = recv(_socket_, buffer, sizeof(buffer), 0);
        if (bytesReceived == -1)
        {
            perror("Error receiving from client.\n Exiting...\n");
            exit(EXIT_FAILURE);
        }
        buffer[bytesReceived] = '\0';
        for(i=0; i<bytesReceived; i++)
        {
            c1 = c2;
            c2 = buffer[i];
            if(c1=='\r' && c2=='\n')
            {
                endOfTextFlag=1;
                break;
            }
        }
        strcat(dataReceived, buffer);
        if(endOfTextFlag)
            break;
    }
    int l = strlen(dataReceived);
    dataReceived[l-2] = '\0';
    return;
}

void multiLineDataReceive(int _socket_, char *dataReceived)
{
    char c1 = '\0', c2 = '\0', c3 = '\0';
    int flag = 1, bytesReceived;
    char buffer[MAX_BUFFER_SIZE];
    int idx_dataReceived=0;
    while (flag)
    {
        bytesReceived = recv(_socket_, buffer, sizeof(buffer), 0);
        for (int i = 0; i < bytesReceived; i++)
        {
            if(buffer[i]!='\r')
            {
                dataReceived[idx_dataReceived++] = buffer[i];
            }
            c1 = c2;
            c2 = c3;
            c3 = buffer[i];
            if(c1 == '\n' && c2 == '.' && c3 == '\r')
            {
                flag = 0;
                break;
            }
        }
    }
    dataReceived[idx_dataReceived++]='\0';
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void send_mail(int smtp_port, const char *server_IP, const char *username)
{
    // Get mail from user at first
    char sender[100];
    char recipient[100];
    char subject[100];
    printf("\nEnter mail to be sent in the specified format:\n");

    char *mailContent[50];
    for (int i = 0; i < 50; i++)
    {
        mailContent[i] = (char *)malloc(80*sizeof(char));
    }
    // Regular expressions to check on the entered data following the specified format
    const char *line0Pattern = "From:[ ]+[^@]+@[^@]+";
    const char *line1Pattern = "To:[ ]+[^@]+@[^@]+";
    const char *line2Pattern = "Subject:[ ]+[^\n]*";
    regex_t regex0, regex1, regex2;
    regcomp(&regex0, line0Pattern, REG_EXTENDED);
    regcomp(&regex1, line1Pattern, REG_EXTENDED);
    regcomp(&regex2, line2Pattern, REG_EXTENDED);

    int ind = 0;
    char clear;
    scanf("%c", &clear);
    while(1)
    {
        // Take each line at a time
        fgets(mailContent[ind], 80, stdin);
        format(mailContent[ind]);

        if(ind == 0)
        {
            // FROM line
            if (regexec(&regex0, mailContent[ind], 0, NULL, 0) != 0) 
            {
                printf("Incorrect format!\n");
                return;
            }
            int i = 6;
            while(mailContent[ind][i] == ' ')
                i++;
            snprintf(sender, sizeof(sender), "%s", mailContent[ind] + i);
        }
        else if(ind == 1)
        {
            // TO line
            if (regexec(&regex1, mailContent[ind], 0, NULL, 0) != 0) 
            {
                printf("Incorrect format!\n");
                return;
            }
            int i = 4;
            while(mailContent[ind][i] == ' ')
                i++;
            snprintf(recipient, sizeof(recipient), "%s", mailContent[ind] + i);
        }
        else if(ind == 2)
        {
            // SUBJECT line
            if (regexec(&regex2, mailContent[ind], 0, NULL, 0) != 0) 
            {
                printf("Incorrect format!\n");
                return;
            }
        }
        if(strcmp(mailContent[ind], ".") == 0)
        {
            // END of message
            break;
        }
        ind++;
    }

    // Socket creation
    int smtp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (smtp_socket == -1) 
    {
        perror("SMTP socket creation failed!\n Exiting...\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(smtp_port);
    server_address.sin_addr.s_addr = inet_addr(server_IP);

    // Connect to the server port
    if (connect(smtp_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Error connecting to SMTP server,\n Exiting...\n");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_BUFFER_SIZE];
    char dataReceived[MAX_BUFFER_SIZE];
    size_t bytesReceived;
    int endOfTextFlag=0;
    int i;
    char* serverDomainName;

    // Receive acceptance message from server
    dataReceive(smtp_socket, dataReceived);
    if(checkFormat_1(dataReceived))
    {
        // printf("%s\n", dataReceived);
    }
    else
    {
        fprintf(stderr, "Unknown message received from server.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }


    // Sending HELO command
    char username_[100];
    strcpy(username_, username);
    char *domain = strtok(username_, "@");
    domain = strtok(NULL, "@");
    snprintf(buffer, sizeof(buffer), "HELO %s\r\n", domain);
    send(smtp_socket, buffer, strlen(buffer), 0);

    // Receiving response for HELO command
    dataReceive(smtp_socket, dataReceived);
    if(checkFormat_2(dataReceived, domain))
    {
        // printf("%s\n", dataReceived);
    }
    else
    {
        fprintf(stderr, "Unknown message received from server.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }


    // Sending MAIL command
    snprintf(buffer, sizeof(buffer), "MAIL FROM: <%s>\r\n", sender);
    send(smtp_socket, buffer, strlen(buffer), 0);

    // Receiving response for MAIL command
    dataReceive(smtp_socket, dataReceived);
    if(checkFormat_3(dataReceived, sender))
    {
        // printf("%s\n", dataReceived);
    }
    else
    {
        fprintf(stderr, "Unknown message received from server.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }


    // Sending RCPT command
    snprintf(buffer, sizeof(buffer), "RCPT TO: <%s>\r\n", recipient);
    send(smtp_socket, buffer, strlen(buffer), 0);

    // Receiving response for RCPT command
    dataReceive(smtp_socket, dataReceived);
    if(checkFormat_4(dataReceived))
    {
        // printf("%s\n", dataReceived);
    }
    else if(checkFormat_5(dataReceived))
    {
        printf("Invalid address in [TO] field. Please try again!\n");
        close(smtp_socket);
        return;
    }
    else
    {
        fprintf(stderr, "Unknown message received from server.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }


    // sending DATA command
    snprintf(buffer, sizeof(buffer), "DATA\r\n");
    send(smtp_socket, buffer, strlen(buffer), 0);

    // Receiving response for DATA command
    dataReceive(smtp_socket, dataReceived);
    if(checkFormat_6(dataReceived))
    {
        // printf("%s\n", dataReceived);
    }
    else
    {
        fprintf(stderr, "Unknown message received from server.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }


    // Sending whole message
    for (int i = 0; i < ind+1; i++)
    {
        snprintf(buffer, sizeof(buffer), "%s\r\n", mailContent[i]);
        send(smtp_socket, buffer, strlen(buffer), 0);
    }


    // Receiving response for whole message
    dataReceive(smtp_socket, dataReceived);
    if(checkFormat_7(dataReceived))
    {
        // printf("%s\n", dataReceived);
    }
    else
    {
        fprintf(stderr, "Unknown message received from server.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }


    // sending QUIT command
    snprintf(buffer, sizeof(buffer), "QUIT\r\n");
    send(smtp_socket, buffer, strlen(buffer), 0);

    // Receiving response for QUIT command
    dataReceive(smtp_socket, dataReceived);
    if(checkFormat_8(dataReceived))
    {
        // printf("%s\n", dataReceived);
    }
    else
    {
        fprintf(stderr, "Unknown message received from server.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }

    printf("\n--------------- Mail sent successfully! ----------------\n");
    close(smtp_socket);
}

// Function to format a string for proper printing
void format(char *s)
{
    int i = 0;
    while(s[i] != '\n')
        i++;
    s[i] = '\0';
}

// Function to check the intial response of server 
int checkFormat_1(char *str) 
{
    const char *pattern1 = "220 <[^\n]+> Service ready";
    regex_t regex;
    if (regcomp(&regex, pattern1, REG_EXTENDED) < 0)
    {
        fprintf(stderr, "Regex failed!\n");
        return 0;
    }
    if ( regexec(&regex, str, 0, NULL, 0) == 0 )
    {
        return 1;
    }
    return 0;
}

// Function to check the response of server after sending HELO command
int checkFormat_2(const char *str, const char *domain) 
{
    if (strncmp(str, "250 OK ", 7) != 0) return 0;
    if (strlen(str) <= 7) return 0;

    const char *domain_ptr = strstr(str, domain);
    if (domain_ptr == NULL) return 0;
    if (domain_ptr[strlen(domain)] != '\0') return 0;
    return 1;
}

// Function to check the response of server after sending MAIL command
int checkFormat_3(const char *str, const char *email) 
{
    if (strncmp(str, "250 <", 5) != 0) return 0;
    const char *start_bracket = strstr(str, "<");
    const char *end_bracket = strstr(str, ">");
    if (!start_bracket || !end_bracket) return 0;
    start_bracket++;
    int email_length = end_bracket - start_bracket;
    char extracted_email[email_length + 1];
    strncpy(extracted_email, start_bracket, email_length);
    extracted_email[email_length] = '\0';
    if (strcmp(extracted_email, email) != 0) return 0;
    const char *sender_ok = strstr(str, "... Sender ok");
    if (!sender_ok) return 0;
    return 1;
}

// Function to check the response of server after sending RCPT command
int checkFormat_4(const char *str)
{
    if (strncmp(str, "250 root... Recipient ok", 24) != 0) return 0;
    return 1;
}

// Function to check the response of server after sending RCPT command and if it had sent wrong address 
int checkFormat_5(const char *str)
{
    if (strncmp(str, "550 No such user", 16) != 0) return 0;
    return 1;
}

// Function to check the response of server after sending DATA command
int checkFormat_6(const char *str)
{
    if (strncmp(str, "354 Enter mail, end with \".\" on a line by itself", 47) != 0) return 0;
    return 1;
}

// Function to check the response of server at the end of message delivery
int checkFormat_7(const char *str)
{
    if (strncmp(str, "250 OK Message accepted for delivery", 32) != 0) return 0;
    return 1;
}

// Function to check the response of server after sending QUIT command
int checkFormat_8(const char *str)
{
    const char *pattern8 = "221 [^\n]+";
    regex_t regex;
    if (regcomp(&regex, pattern8, REG_EXTENDED) < 0)
    {
        fprintf(stderr, "Regex failed!\n");
        return 0;
    }
    if ( regexec(&regex, str, 0, NULL, 0) == 0 )
    {
        return 1;
    }
    return 0;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void manage_mail(int pop3_port, const char *server_IP, const char *username, const char *password)
{
    // Socket creation
    int pop3_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (pop3_socket == -1) 
    {
        perror("POP3 socket creation failed!\n Exiting...\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(pop3_port);
    server_address.sin_addr.s_addr = inet_addr(server_IP);

    // Connect to the server port
    if (connect(pop3_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Error connecting to POP3 server,\n Exiting...\n");
        exit(EXIT_FAILURE);
    }

    //------------ INITIALIZATION --------------
    char dataReceived[MAX_BUFFER_SIZE];
    dataReceive(pop3_socket, dataReceived);
    if (strncmp(dataReceived, "+OK", 3) == 0) 
    {
        // do nothing continue
        // POP3 server ready
    } 
    else
    {
        fprintf(stderr, "Server not available. Exiting...\n");
        exit(EXIT_FAILURE);
    }


    //---------- State: AUTHORIZATION ------------
    // Send username
    sendUsernameToPOP3(pop3_socket, username);

    // Send password
    sendPasswordToPOP3(pop3_socket, password);

    //---------- State: UPDATE------------

    //---------- State: TRANSITION------------
    // Send STAT
    int numMail=0;
    int numTotalChar=0;
    sendSTATToPOP3(pop3_socket, &numMail, &numTotalChar);

    while(1)
    {
        // Get and print the mail list
        getMailList(pop3_socket);

        // Prompt-input for user
        int user_select=0;
        while(1)
        {
            printf("Enter mail no. to see: ");
            scanf("%d", &user_select);
            if(user_select == -1)
            {
                sendQUITToPOP3(pop3_socket);
                return;
            }
            else if(user_select>=1 && user_select<=numMail)
            {
                break;
            }
            else
            {
                printf("Mail no. out of range, give again.\n");
            }
        }

        // Get complete mail from the server and print it
        getCompleteMail(pop3_socket, user_select);

        // Promt-input for user
        char user_del='*';
        printf("Press 'd' to delete the above mail, otherwise press any other character: ");
        // fflush(stdin);
        char buf;
        scanf("%c", &buf);
        user_del = getchar();
        if(user_del=='d')
        {
            deleteMail(pop3_socket, user_select);
        }
        else
        {
            // do nothing and continue
        }
    }

}

// Function to send username to the POP3 server
void sendUsernameToPOP3(const int pop3_socket, const char* username)
{
    char buffer[MAX_BUFFER_SIZE];
    for(int i=0; i<MAX_BUFFER_SIZE; i++)
    {
        buffer[i]='\0';
    }
    snprintf(buffer, sizeof(buffer), "USER %s\r\n", username);
    send(pop3_socket, buffer, strlen(buffer), 0);
    
    char dataReceived[MAX_BUFFER_SIZE];
    dataReceive(pop3_socket, dataReceived);
    if (strncmp(dataReceived, "+OK", 3) == 0) 
    {
        // do nothing continue
    } 
    else if (strncmp(dataReceived, "-ERR", 4) == 0) 
    {
        fprintf(stderr, "Username or Password not found. Exiting...\n");
        exit(EXIT_FAILURE);
    } 
    else 
    {
        fprintf(stderr, "Unknown message received from POP3 server. Exiting...\n");
        exit(EXIT_FAILURE);
    }

}

// Function to send password to the POP3 server
void sendPasswordToPOP3(const int pop3_socket, const char* password)
{
    char buffer[MAX_BUFFER_SIZE];
    for(int i=0; i<MAX_BUFFER_SIZE; i++)
    {
        buffer[i]='\0';
    }
    snprintf(buffer, sizeof(buffer), "PASS %s\r\n", password);
    send(pop3_socket, buffer, strlen(buffer), 0);

    char dataReceived[MAX_BUFFER_SIZE];
    dataReceive(pop3_socket, dataReceived);
    // printf("%s \n", dataReceived);
    if (strncmp(dataReceived, "+OK", 3) == 0) 
    {
        // do nothing continue
    } 
    else if (strncmp(dataReceived, "-ERR", 4) == 0) 
    {
        fprintf(stderr, "Username or Password not found. Exiting...\n");
        exit(EXIT_FAILURE);
    } 
    else 
    {
        fprintf(stderr, "Unknown message received from POP3 server. Exiting...\n");
        exit(EXIT_FAILURE);
    }
}

// Function to send STAT command to the POP3 server
void sendSTATToPOP3(int pop3_socket, int *numMailPtr, int *numTotalCharPtr)
{
    char buffer[MAX_BUFFER_SIZE];
    for(int i=0; i<MAX_BUFFER_SIZE; i++)
    {
        buffer[i]='\0';
    }
    snprintf(buffer, sizeof(buffer), "STAT\r\n");
    send(pop3_socket, buffer, strlen(buffer), 0);

    char dataReceived[MAX_BUFFER_SIZE];
    dataReceive(pop3_socket, dataReceived);
    if (strncmp(dataReceived, "+OK", 3) == 0) 
    {
        sscanf(dataReceived, "+OK %d %d\r\n", numMailPtr, numTotalCharPtr);
    } 
    else if (strncmp(dataReceived, "-ERR", 4) == 0) 
    {
        fprintf(stderr, "Error: %s\n", dataReceived+5);
        exit(EXIT_FAILURE);
    } 
    else 
    {
        fprintf(stderr, "Unknown message received from POP3 server. Exiting...\n");
        exit(EXIT_FAILURE);
    }
}

// Function to print the entire maillist of Mailbox to the POP3 server
void getMailList(int pop3_socket)
{
    int i;
    char buffer[MAX_BUFFER_SIZE];
    for(int i=0; i<MAX_BUFFER_SIZE; i++)
    {
        buffer[i]='\0';
    }
    snprintf(buffer, sizeof(buffer), "LIST\r\n");
    send(pop3_socket, buffer, strlen(buffer), 0);

    char dataReceived[MAX_BUFFER_SIZE];
    multiLineDataReceive(pop3_socket, dataReceived);

    // Extract the number of lines from the first line
    int num_lines;
    sscanf(dataReceived, "+OK %d", &num_lines);
    int *mail_no_list = (int *)malloc(num_lines * sizeof(int));
    if (mail_no_list == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    // Extract mail_no_list from each subsequent line and store them in the array
    char *token = strtok(dataReceived, "\n");
    int index = 0;
    while ((token = strtok(NULL, "\n")) != NULL && index < num_lines) {
        int num;
        sscanf(token, "%d", &num);
        mail_no_list[index] = num;
        index++;
    }

    printf("\n------------------------------ Mailbox -----------------------------\n");
    for(i=0; i<index; i++)
    {
        // Ask for i-th mail info
        char buffer[MAX_BUFFER_SIZE];
        for(int j=0; j<MAX_BUFFER_SIZE; j++)
        {
            buffer[j]='\0';
        }
        snprintf(buffer, sizeof(buffer), "RETR %d\r\n", mail_no_list[i]);
        send(pop3_socket, buffer, strlen(buffer), 0);

        char dataReceived[MAX_BUFFER_SIZE];
        int bytesReceived = recv(pop3_socket, buffer, 1, 0);
        if (buffer[0]=='+') 
        {
            multiLineDataReceive(pop3_socket, dataReceived);
            char *from = strstr(dataReceived, "From:");
            char *subject = strstr(dataReceived, "Subject:");
            char *received = strstr(dataReceived, "Received:");
            if (from && subject && received) {
                from += strlen("From:");
                subject += strlen("Subject:");
                received += strlen("Received:");
                while (*from == ' ')
                    from++;
                while (*subject == ' ')
                    subject++;
                while (*received == ' ')
                    received++;
                char *fromEnd = strchr(from, '\n');
                char *subjectEnd = strchr(subject, '\n');
                char *receivedEnd = strchr(received, '\n');

                // Print the extracted data
                printf("%d: ", mail_no_list[i]);
                printf("<%.*s> ", (int)(fromEnd - from), from);
                printf("<%.*s> ", (int)(subjectEnd - subject), subject);
                printf("<%.*s> ", (int)(receivedEnd - received), received);
                printf("\n");

            } else {
                fprintf(stderr, "Required fields not found.\n");
            }

        } 
        else if (buffer[0]=='-') 
        {
            dataReceive(pop3_socket, dataReceived);
            fprintf(stderr, "Error: %s\n", dataReceived+4);
            return;
        } 
        else 
        {
            fprintf(stderr, "Unknown message received from POP3 server. Exiting...\n");
            exit(EXIT_FAILURE);
        }
    }
    printf("---------------------------------------------------------------------\n");
    free(mail_no_list);
}

// Function to print a complete mail to the POP3 server
void getCompleteMail(int pop3_socket, int user_select)
{
    char buffer[MAX_BUFFER_SIZE];
    for(int i=0; i<MAX_BUFFER_SIZE; i++)
    {
        buffer[i]='\0';
    }
    snprintf(buffer, sizeof(buffer), "RETR %d\r\n", user_select);
    send(pop3_socket, buffer, strlen(buffer), 0);

    char dataReceived[MAX_BUFFER_SIZE];
    int bytesReceived = recv(pop3_socket, buffer, 1, 0);

    if (buffer[0]=='+') 
    {
        multiLineDataReceive(pop3_socket, dataReceived);
        char *first_newline_pos = strchr(dataReceived, '\n');
        size_t remaining_length = strlen(first_newline_pos+1);
        memmove(dataReceived, first_newline_pos, remaining_length + 1);
        dataReceived[remaining_length] = '\0';
        printf("\n----------------------------- Mail %d -----------------------------\n", user_select);
        printf("%s\n", dataReceived);
    } 
    else if (buffer[0]=='-') 
    {
        dataReceive(pop3_socket, dataReceived);
        fprintf(stderr, "Error: %s\n", dataReceived+4);
    } 
    else 
    {
        fprintf(stderr, "Unknown message received from POP3 server. Exiting...\n");
        exit(EXIT_FAILURE);
    }
}

// Function to send delete a mail to the POP3 server
void deleteMail(int pop3_socket, int user_select)
{
    char buffer[MAX_BUFFER_SIZE];
    for(int i=0; i<MAX_BUFFER_SIZE; i++)
    {
        buffer[i]='\0';
    }
    snprintf(buffer, sizeof(buffer), "DELE %d\r\n", user_select);
    send(pop3_socket, buffer, strlen(buffer), 0);

    char dataReceived[MAX_BUFFER_SIZE];
    dataReceive(pop3_socket, dataReceived);
    if (strncmp(dataReceived, "+OK", 3) == 0) 
    {
        printf("Mail %d successfully deleted.\n", user_select);
    } 
    else if (strncmp(dataReceived, "-ERR", 4) == 0) 
    {
        fprintf(stderr, "Error: %s\n", dataReceived+5);
    } 
    else 
    {
        fprintf(stderr, "Unknown message received from POP3 server. Exiting...\n");
        exit(EXIT_FAILURE);
    }
}

// Function to send QUIT command to the POP3 server
void sendQUITToPOP3(int pop3_socket)
{
    char buffer[MAX_BUFFER_SIZE];
    for(int i=0; i<MAX_BUFFER_SIZE; i++)
    {
        buffer[i]='\0';
    }
    snprintf(buffer, sizeof(buffer), "QUIT\r\n");
    send(pop3_socket, buffer, strlen(buffer), 0);

    char dataReceived[MAX_BUFFER_SIZE];
    dataReceive(pop3_socket, dataReceived);
    if (strncmp(dataReceived, "+OK", 3) == 0) 
    {
        printf("%s\n", dataReceived+4);
    } 
    else if (strncmp(dataReceived, "-ERR", 4) == 0) 
    {
        fprintf(stderr, "Error: %s\n", dataReceived+5);
    } 
    else 
    {
        fprintf(stderr, "Unknown message received from POP3 server. Exiting...\n");
        exit(EXIT_FAILURE);
    }
}




