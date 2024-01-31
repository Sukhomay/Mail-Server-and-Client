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
#define DATETIMESIZE 15
#define PATHSIZE 1024


int main(int argc, char *argv[]) 
{
    if (argc<2) 
    {
        fprintf(stderr, "[SMTPMAIL_SERVER]: Error! %s received no port. Exiting...\n", argv[0]);
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
    int i, continueFlag;
    char username[50] = "powerSMTP";


    time_t t;                                                        
    struct tm dateTime;  
    char dateTimeData[DATETIMESIZE];

    char receiverMailBoxPath[PATHSIZE];
    FILE *receiverMailBoxFilePtr;


    my_port = atoi(argv[1]);

    // Socket creation
    serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFD<0) 
    {
        perror("[SMTPMAIL_SERVER]: Error! Socket creation failed. Exiting...\n");
        exit(EXIT_FAILURE);
    }
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(my_port);

    // Bind the server
    if (bind(serverSocketFD, (struct sockaddr *) &serverAddress, sizeof(serverAddress))<0) 
    {
        perror("[SMTPMAIL_SERVER]: Error! Binding of server address failed. Exiting...\n");
        exit(EXIT_FAILURE);
    }
    printf("[SMTPMAIL_SERVER]: SMTP-server running...\n");

    // Server ready to accept connections ans allow process pileup up to 5
    if (listen(serverSocketFD, 5) == -1) 
    {
        perror("[SMTPMAIL_SERVER]: Error! Server unable to listen. Exiting...\n");
        exit(EXIT_FAILURE);
    }

    while (1) 
    {
        lenClientAddress = sizeof(clientAddress);
        // Accept the client connection
        clientSocketFD = accept(serverSocketFD, (struct sockaddr *) &clientAddress, &lenClientAddress);
        if (clientSocketFD<0) 
        {
            perror("[SMTPMAIL_SERVER]: Error! Unable to accept client address.\n");
            exit(0);
        }
        // Fork to serve the client
        pid = fork();
        if(pid<0)
        {
            /* ERROR */
            perror("[SMTPMAIL_SERVER]: Error! Fork failed.\n");
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

            // Continuously receive the data packcets
            continueFlag=1;
            for(i=0; i<DATASIZE; i++)
            {
                dataReceived[i] = '\0';
            }
            
            snprintf(buffer, sizeof(buffer), "220 %s Service ready\r\n", username);
            send(clientSocketFD, buffer, strlen(buffer), 0);

            // Receive initial response from client
            bytesReceived = recv(clientSocketFD, buffer, sizeof(buffer), 0);
            if (bytesReceived == -1)
            {
                perror("Error receiving from client server");
                exit(EXIT_FAILURE);
            }
            buffer[bytesReceived-2] = '\0';
            printf("%s\n", buffer);  // Print client's initial response

            char sender_domain[50];
            strcpy(sender_domain, buffer + 5);
            snprintf(buffer, sizeof(buffer), "250 OK Hello %s\r\n", sender_domain);
            send(clientSocketFD, buffer, strlen(buffer), 0);

            // Receive MAIL from client
            bytesReceived = recv(clientSocketFD, buffer, sizeof(buffer), 0);
            if (bytesReceived == -1)
            {
                perror("Error receiving from client server");
                exit(EXIT_FAILURE);
            }
            buffer[bytesReceived-2] = '\0';
            printf("%s\n", buffer); 

            // answer for MAIL
            char sender[50];
            strcpy(sender, buffer + 11);
            snprintf(buffer, sizeof(buffer), "250 %s... Sender ok\r\n", sender);
            send(clientSocketFD, buffer, strlen(buffer), 0);

            // Receive RCPT from client
            bytesReceived = recv(clientSocketFD, buffer, sizeof(buffer), 0);
            if (bytesReceived == -1)
            {
                perror("Error receiving from client server");
                exit(EXIT_FAILURE);
            }
            buffer[bytesReceived-2] = '\0';
            printf("%s\n", buffer); 

            // answer for RCPT
            char recipient[50];
            strcpy(recipient, buffer + 9);
            snprintf(buffer, sizeof(buffer), "250 root... Recipient ok\r\n");
            send(clientSocketFD, buffer, strlen(buffer), 0);

            // Receive DATA command from client
            bytesReceived = recv(clientSocketFD, buffer, sizeof(buffer), 0);
            if (bytesReceived == -1)
            {
                perror("Error receiving from client server");
                exit(EXIT_FAILURE);
            }
            buffer[bytesReceived-2] = '\0';
            printf("%s\n", buffer); 

            // answer for DATA
            snprintf(buffer, sizeof(buffer), "354 Enter mail, end with \".\" on a line by itself\r\n");
            send(clientSocketFD, buffer, strlen(buffer), 0);

            // Receive whole message from client
            int fd = open("try.txt", O_WRONLY | O_APPEND);
            char c1 = '\0', c2 = '\0', c3 = '\0';
            int flag = 1;
            while (flag)
            {
                bytesReceived = recv(clientSocketFD, buffer, sizeof(buffer), 0);
                // buffer[bytesReceived] = '\0';
                // printf("%s", buffer);
                for (int i = 0; i < bytesReceived; i++)
                {
                    // write(fd, )
                    printf("%c", buffer[i]);
                    
                    c1 = c2;
                    c2 = c3;
                    c3 = buffer[i];
                    if(c1 == '\n' && c2 == '.' && c3 == '\r')
                    {
                        // printf("bui\n");
                        printf("%c", '\n');
                        flag = 0;
                        break;
                    }
                }
            }
            printf("succeed\n");
            exit(0);
        }
    }

    close(serverSocketFD);

    return 0;
}


/*
// Store the entire message in dataReceived for further processing
            while(continueFlag)
            {
                bytesReceived = recv(clientSocketFD, buffer, BUFFERSIZE, 0);
                if (bytesReceived<0) 
                {
                    perror("[SMTPMAIL_SERVER]: Error! Unable to receive packets from client. Exiting...\n");
                    exit(EXIT_FAILURE);
                }
                buffer[bytesReceived] = '\0';
                // Check for end of message
                for(i=bytesReceived-1; i>=0; --i)
                {
                    if(buffer[i]==EOF)
                    {
                        continueFlag=0;
                        buffer[i]='\0';
                        break;
                    }
                }
                strcat(dataReceived, buffer);
            }
            
            // Extract all the fields
            char *fromData = strtok(dataReceived, "\n");
            char *toData = strtok(NULL, "\n");
            char *subjectData = strtok(NULL, "\n");
            char *bodyData = strtok(NULL, "\0");
            // Get the current date and time
            t = time(NULL);                                                        
            dateTime = *localtime(&t);  
            snprintf(dateTimeData, DATETIMESIZE, "%d:%d:%d:%d:%d", dateTime.tm_mday, dateTime.tm_mon+1, dateTime.tm_year+1900, dateTime.tm_hour, dateTime.tm_min);

            //Put the formated message in the receivers mailbox
            snprintf(receiverMailBoxPath, PATHSIZE, "%s/mymailbox", toData);
            receiverMailBoxFilePtr = fopen(receiverMailBoxPath, "a");
            if(receiverMailBoxFilePtr==NULL)
            {
                perror("[SMTPMAIL_SERVER]: Error! Unable to access receiver's mailbox. Exiting...\n");
                exit(EXIT_FAILURE);
            }
            fprintf(receiverMailBoxFilePtr, "From: %s\nTo: %s\nSubject: %s\nReceived: %s\n%s\n.\n", fromData, toData, subjectData, dateTimeData, bodyData);
            fclose(receiverMailBoxFilePtr);

            // Close the client socket at the end of service complete
            close(clientSocketFD);
            exit(EXIT_SUCCESS);
*/

