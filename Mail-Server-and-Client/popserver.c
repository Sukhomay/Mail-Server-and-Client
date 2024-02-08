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
#define MAXDETAILS 100
#define MAXMAIL 100

// File path for storing the list of registered mailboxes
const char userListFile[] = "user.txt";

// Forward function declarations
int loadInfo(char *username, int *stratLine, int *mailSize);
void dataReceive(int cli_socket, char *dataReceived);
void sendError(int clientSocketFD, char *message);
void respondToUSER(int clientSocketFD, char *dataReceived);

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
    char username[50] = "powerPOP3";

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
    printf("POP3-server running...\n");

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
            
            char client_user[MAXDETAILS], password[MAXDETAILS];
            int stratLine[MAXMAIL], mailSize[MAXMAIL];
            
            // Acknowledment of scuccessful acceptance to the client
            snprintf(buffer, sizeof(buffer), "+OK %s Service ready\r\n", username);
            send(clientSocketFD, buffer, strlen(buffer), 0);

            // Continuously receive command response from client
            char *commandFromClient;

            // First command must be user
            dataReceive(clientSocketFD, dataReceived);

            // Process command as required 
            char _dataReceived_[DATASIZE];
            strcpy(_dataReceived_, dataReceived);
            commandFromClient = strtok(_dataReceived_, " \t\n");

            if(strcmp(commandFromClient, "USER")==0)
            {
                respondToUSER(clientSocketFD, dataReceived);
            }
            else
            {
                sendError(clientSocketFD, "-ERR");
            }

            while(1)
            {
                dataReceive(clientSocketFD, dataReceived);

                // Process each command as required 
                char _dataReceived_[DATASIZE];
                strcpy(_dataReceived_, dataReceived);
                commandFromClient = strtok(_dataReceived_, " \t\n");
                
                if(8)
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

// Functions 
int loadInfo(char *username, int *stratLine, int *mailSize)
{
    char mailPath[PATHSIZE];
    snprintf(mailPath, sizeof(mailPath), "./%s/mymailbox", username);

    FILE *file = fopen(mailPath, "r");

    // Check if file opened successfully
    if (file == NULL) 
    {
        printf("Unable to open the file.\n");
        exit(0);
    }

    char line[100]; 
    // Read lines from the file using fgets
    int ind = 0;
    int line_no = 0;
    int size = 0;
    stratLine[ind] = line_no;
    while (fgets(line, sizeof(line), file) != NULL)
    {
        size += strlen(line);
        line_no++;
        if(strcmp(line, ".\n") == 0)
        {
            mailSize[ind] = size;
            size = 0;
            ind++;
            stratLine[ind] = line_no;
        }
        // printf("%s****\n", line);
    }
    int total = ind;
    // Close the file
    fclose(file);

    for (int i = 0; i < total; i++)
    {
        printf("%d  %d\n", stratLine[i], mailSize[i]);
    }

    return total;
}

// Function to receive data from the server
void dataReceive(int cli_socket, char *dataReceived)
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
        int bytesReceived = recv(cli_socket, buffer, sizeof(buffer), 0);
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


void respondToUSER(int clientSocketFD, char *dataReceived)
{

}