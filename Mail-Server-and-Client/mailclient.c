#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<regex.h>

#define MAX_BUFFER_SIZE 1024

// Forwoard fucntion declaration
void send_mail(int smtp_port, const char *server_IP, const char *username);
void format(char *s);
void dataReceive(int smtp_socket, char *dataReceived);
int checkFormat_1(char *str);
int checkFormat_2(const char *str, const char *domain);
int checkFormat_3(const char *str, const char *email);
int checkFormat_4(const char *str);
int checkFormat_5(const char *str);
int checkFormat_6(const char *str);
int checkFormat_7(const char *str);
int checkFormat_8(const char *str);

void manage_mail(int pop3_port, const char *server_IP, const char *username, const char *password);

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
                printf("Option Quit selected.\n");
                printf("Quitting the program...\n");
                break;
            default:
                printf("Invalid option. Please try again.\n");
        }
    } while (option != 3);

    return 0;
}


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
        printf("%s\n", dataReceived);
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
        printf("%s\n", dataReceived);
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
        printf("%s\n", dataReceived);
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
        printf("%s\n", dataReceived);
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
        printf("%s\n", dataReceived);
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
        printf("%s\n", dataReceived);
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
        printf("%s\n", dataReceived);
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

// Function to receive data from the server
void dataReceive(int smtp_socket, char *dataReceived)
{
    int endOfTextFlag=0;
    char buffer[MAX_BUFFER_SIZE];
    int i;
    for(int i=0; i<MAX_BUFFER_SIZE; i++)
    {
        dataReceived[i]='\0';
    }
    while(1)
    {
        int bytesReceived = recv(smtp_socket, buffer, sizeof(buffer), 0);
        if (bytesReceived == -1)
        {
            perror("Error receiving from SMTP server.\n Exiting...\n");
            exit(EXIT_FAILURE);
        }
        buffer[bytesReceived] = '\0';
        for(i=0; i<bytesReceived-1; i++)
        {
            if(buffer[i]=='\r' && buffer[i+1]=='\n')
            {
                buffer[i] = '\0';
                endOfTextFlag=1;
                break;
            }
        }
        strcat(dataReceived, buffer);
        if(endOfTextFlag)
            break;
    }
    return;
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

/////////////////////////////////////////////////////////



void manage_mail(int pop3_port, const char *server_IP, const char *username, const char *password)
{
    // Socket creation
    int smtp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (smtp_socket == -1) 
    {
        perror("SMTP socket creation failed!\n Exiting...\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(pop3_port);
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

    recv(smtp_socket, buffer, sizeof(buffer), 0);

}