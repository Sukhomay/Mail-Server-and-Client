#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFERSIZE 101
#define DATASIZE 1024
#define DATETIMESIZE 25
#define PATHSIZE 1024
#define COMMANDSIZE 10
#define MAXEMAILADDRLEN 80

// File path for storing the list of registered mailboxes
const char userListFile[] = "user.txt";

// Forward function declarations
void respondToHELO(int clientSocketFD, char dataReceived[]);
void respondToMAIL(int clientSocketFD, char dataReceived[]);
void respondToRCPT(int clientSocketFD, char dataReceived[], char receiverMailAddr[]);
void respondToDATA(int clientSocketFD, char dataReceived[], char receiverMailAddr[]);
void respondToQUIT(int clientSocketFD, char dataReceived[], char username[]);


int main(int argc, char *argv[]) 
{
    if (argc<2) 
    {
        fprintf(stderr, "Error! %s received no port. Exiting...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Local variables shared across generations
    int my_port;
    int serverSocketFD, clientSocketFD;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t lenClientAddress;
    pid_t pid;
    char buffer[BUFFERSIZE], dataReceived[DATASIZE]; 
    size_t bytesReceived, bytesSent;
    int i, endOfTextFlag;
    char username[50] = "powerSMTP";

    char receiverMailBoxPath[PATHSIZE];
    FILE *receiverMailBoxFilePtr;


    my_port = atoi(argv[1]);

    // Socket creation
    serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFD<0) 
    {
        perror("Error! Socket creation failed. Exiting...\n");
        exit(EXIT_FAILURE);
    }
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(my_port);

    // Bind the server
    if (bind(serverSocketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress))<0) 
    {
        perror("Error! Binding of server address failed. Exiting...\n");
        exit(EXIT_FAILURE);
    }
    printf("SMTP-server running...\n");

    // Server ready to accept connections ans allow process pileup up to 5
    if (listen(serverSocketFD, 5) == -1) 
    {
        perror("Error! Server unable to listen. Exiting...\n");
        exit(EXIT_FAILURE);
    }

    while (1) 
    {
        lenClientAddress = sizeof(clientAddress);
        // Accept the client connection
        clientSocketFD = accept(serverSocketFD, (struct sockaddr *) &clientAddress, &lenClientAddress);
        if (clientSocketFD<0) 
        {
            perror("Error! Unable to accept client address.\n");
            exit(0);
        }


        // Fork to serve the client
        pid = fork();
        if(pid<0)
        {
            /* ERROR */
            perror("Error! Fork failed.\n");
            close(clientSocketFD);
        }
        else if (pid) 
        {
            /* PARENT */
            // Close the client socket
            close(clientSocketFD);
        } 
        else
        {
            /* CHILD */
            // Close the parent socket
            close(serverSocketFD);

            // Acknowledment of scuccessful acceptance to the client
            snprintf(buffer, sizeof(buffer), "220 <%s> Service ready\r\n", username);
            send(clientSocketFD, buffer, strlen(buffer), 0);

            // Continuously receive command response from client
            char *commandFromClient;
            char receiverMailAddr[MAXEMAILADDRLEN];
            while(1)
            {
                endOfTextFlag=0;
                for(i=0; i<DATASIZE; i++)
                {
                    dataReceived[i] = '\0';
                }
                while(1)
                {
                    bytesReceived = recv(clientSocketFD, buffer, sizeof(buffer), 0);
                    if (bytesReceived == -1)
                    {
                        perror("Error receiving from client server");
                        exit(EXIT_FAILURE);
                    }
                    buffer[bytesReceived]='\0';
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
                printf("%s\n", dataReceived);
                // Process each command as required 
                char _dataReceived_[DATASIZE];
                strcpy(_dataReceived_, dataReceived);
                commandFromClient = strtok(_dataReceived_, " \t\n");
                if(strcmp(commandFromClient, "HELO")==0)
                {
                    respondToHELO(clientSocketFD, dataReceived);
                }
                else if(strcmp(commandFromClient, "MAIL")==0)
                {
                    respondToMAIL(clientSocketFD, dataReceived);
                }
                else if(strcmp(commandFromClient, "RCPT")==0)
                {
                    respondToRCPT(clientSocketFD, dataReceived, receiverMailAddr);
                }
                else if(strcmp(commandFromClient, "DATA")==0)
                {
                    respondToDATA(clientSocketFD, dataReceived, receiverMailAddr);
                }
                else if(strcmp(commandFromClient, "QUIT")==0)
                {
                    respondToQUIT(clientSocketFD, dataReceived, username);
                }
                else
                {
                    fprintf(stderr, "invalid command received from client.\n Exiting... \n");
                    exit(EXIT_FAILURE);
                }

            }

            printf("\n\n");
            exit(0);
        }
    }

    close(serverSocketFD);

    return 0;
}


// Funtion to respond to HELO command
void respondToHELO(int clientSocketFD, char dataReceived[])
{
    if (strncmp(dataReceived, "HELO ", 5) != 0 || strlen(dataReceived) <= 5) 
    {
        fprintf(stderr, "Invalid command received from client.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }

    /* Check for the valid domain name */

    char buffer[BUFFERSIZE];
    char sender_domain[50];
    strcpy(sender_domain, dataReceived + 5);
    snprintf(buffer, sizeof(buffer), "250 OK Hello %s\r\n", sender_domain);
    send(clientSocketFD, buffer, strlen(buffer), 0);
}

// Funtion to respond to MAIL command
void respondToMAIL(int clientSocketFD, char dataReceived[])
{
    if (strncmp(dataReceived, "MAIL FROM: <", 12) != 0 || strlen(dataReceived) <= 12) 
    {
        fprintf(stderr, "Invalid command received from client.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }
    const char *end_bracket = strchr(dataReceived + 12, '>');
    if (end_bracket == NULL) 
    {
        fprintf(stderr, "Invalid command received from client.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }
    const char *at_symbol = strchr(dataReceived + 12, '@');
    if (at_symbol == NULL || at_symbol > end_bracket) 
    {
        fprintf(stderr, "Invalid command received from client.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }

    char senderMailAddr[MAXEMAILADDRLEN];
    const char *start_bracket = strstr(dataReceived, "<");
    end_bracket = strstr(dataReceived, ">");
    start_bracket++;
    int lengthAddr = end_bracket - start_bracket;
    strncpy(senderMailAddr, start_bracket, lengthAddr);
    senderMailAddr[lengthAddr] = '\0';
    
    char buffer[BUFFERSIZE];
    snprintf(buffer, sizeof(buffer), "250 <%s>... Sender ok\r\n", senderMailAddr);
    send(clientSocketFD, buffer, strlen(buffer), 0);
}

// Funtion to respond to RCPT command
void respondToRCPT(int clientSocketFD, char dataReceived[], char receiverMailAddr[])
{
    if (strncmp(dataReceived, "RCPT TO: <", 10) != 0 || strlen(dataReceived) <= 10) 
    {
        fprintf(stderr, "Invalid command received from client.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }
    const char *end_bracket = strchr(dataReceived + 10, '>');
    if (end_bracket == NULL) 
    {
        fprintf(stderr, "Invalid command received from client.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }
    const char *at_symbol = strchr(dataReceived + 10, '@');
    if (at_symbol == NULL || at_symbol > end_bracket) 
    {
        fprintf(stderr, "Invalid command received from client.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }

    const char *start_bracket = strstr(dataReceived, "<");
    end_bracket = strstr(dataReceived, ">");
    start_bracket++;
    int lengthAddr = end_bracket - start_bracket;
    strncpy(receiverMailAddr, start_bracket, lengthAddr);
    receiverMailAddr[lengthAddr] = '\0';


    FILE *userFilePtr = fopen(userListFile, "r");
    if (userFilePtr == NULL) 
    {
        fprintf(stderr, "Error opening %s.\n Exiting... \n", userListFile);
        exit(EXIT_FAILURE);
    }

    char line[DATASIZE];
    int isValidMailAddr=0;
    while (fgets(line, sizeof(line), userFilePtr) != NULL) 
    {
        char *token = strtok(line, " \t\n");
        if (token != NULL && strcmp(token, receiverMailAddr) == 0) {
            isValidMailAddr=1;
            break;
        }
    }
    fclose(userFilePtr);
    
    char buffer[BUFFERSIZE];
    if(!isValidMailAddr)
    {
        snprintf(buffer, sizeof(buffer), "550 No such user\r\n");
        send(clientSocketFD, buffer, strlen(buffer), 0);
        printf("Invalid address in [TO] field. Closing the connection...\n\n");
        close(clientSocketFD);
        exit(0);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "250 root... Recipient ok\r\n");
        send(clientSocketFD, buffer, strlen(buffer), 0);
    }
    
}

// Funtion to respond to DATA command
void respondToDATA(int clientSocketFD, char dataReceived[], char receiverMailAddr[])
{
    if (strncmp(dataReceived, "DATA", 4) != 0) 
    {
        fprintf(stderr, "Invalid command received from client.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFERSIZE];
    char sender_domain[50];
    strcpy(sender_domain, dataReceived + 5);
    snprintf(buffer, sizeof(buffer), "354 Enter mail, end with \".\" on a line by itself\r\n");
    send(clientSocketFD, buffer, strlen(buffer), 0);

    // Access the mailbox of the recipient
    char mailboxPath[PATHSIZE];
    snprintf(mailboxPath, sizeof(mailboxPath), "%s/mymailbox", receiverMailAddr);
    FILE *receiverMailbox = fopen(mailboxPath, "a");
    if (!receiverMailbox) 
    {
        perror("Error opening receiver mailbox.\n Exiting...\n");
        exit(EXIT_FAILURE);
    }

    // Get the current date and time
    time_t t;                                                        
    struct tm dateTime;  
    char dateTimeData[DATETIMESIZE];
    t = time(NULL);                                                        
    dateTime = *localtime(&t);  
    snprintf(dateTimeData, DATETIMESIZE, "Received: %d.%d.%d:%d:%d", dateTime.tm_mday, dateTime.tm_mon+1, dateTime.tm_year+1900, dateTime.tm_hour, dateTime.tm_min);

    // Receive whole message from client
    char c1 = '\0', c2 = '\0', c3 = '\0';
    int newl = 0, dtflag = 1;
    int flag = 1, bytesReceived;
    while (flag)
    {
        bytesReceived = recv(clientSocketFD, buffer, sizeof(buffer), 0);
        for (int i = 0; i < bytesReceived; i++)
        {
            if(buffer[i]!='\r') 
                fprintf(receiverMailbox, "%c", buffer[i]);

            if(buffer[i] == '\n')
                newl++;
            if(newl == 3 && dtflag)
            {
                fprintf(receiverMailbox, "%s\n", dateTimeData);
                dtflag = 0;
            }

            c1 = c2;
            c2 = c3;
            c3 = buffer[i];
            if(c1 == '\n' && c2 == '.' && c3 == '\r')
            {
                fprintf(receiverMailbox, "%c", '\n');
                flag = 0;
                break;
            }
        }
    }

    fclose(receiverMailbox);
    
    // Answer for whole message
    snprintf(buffer, sizeof(buffer), "250 OK Message accepted for delivery\r\n");
    send(clientSocketFD, buffer, strlen(buffer), 0);
}

// Funtion to respond to QUIT command
void respondToQUIT(int clientSocketFD, char dataReceived[], char username[])
{
    if (strncmp(dataReceived, "QUIT", 4) != 0) 
    {
        fprintf(stderr, "Invalid command received from client.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFERSIZE];
    snprintf(buffer, sizeof(buffer), "221 %s closing connection\r\n", username);
    send(clientSocketFD, buffer, strlen(buffer), 0);
    close(clientSocketFD);
    printf("Closing the connection...\n\n");
    exit(0);
}
