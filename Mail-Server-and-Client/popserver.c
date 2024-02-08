#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFERSIZE 101
#define MAX_BUFFER_SIZE 101
#define DATASIZE 1024
#define DATETIMESIZE 25
#define PATHSIZE 1024
#define COMMANDSIZE 10
#define MAXEMAILADDRLEN 80
#define MAXDETAILS 100
#define MAXMAIL 250

// File path for storing the list of registered mailboxes
const char userListFile[] = "user.txt";

// Forward function declarations
int loadInfo(char *username, int *stratLine, int *mailSize);
void dataReceive(int cli_socket, char *dataReceived);
void sendError(int clientSocketFD, char *message, int ext);
void respondToUSER(int clientSocketFD, char *dataReceived, char *);
void respondToPASS(int clientSocketFD, char *dataReceived, char *);
void respondToSTAT(int clientSocketFD, int *isDeleted, char *dataReceived, int total, int *MailSize);
void respondToLIST(int clientSocketFD, int *isDeleted, char *dataReceived, int total_mail, int *stratLine, int *mailSize);
void respondToRETR(int clientSocketFD, int *isDeleted, char *dataReceived, int total_mail, int *stratLine, int *mailSize, char *username);
void respondToDELE(int clientSocketFD, int *isDeleted, int toatl_mail, char *dataReceived);
void respondToRSET(int clientSocketFD, int *isDeleted, char *dataReceived, int total_mail, int *stratLine, int *mailSize);
void respondToQUIT(int clientSocketFD, int *isDeleted, char *dataReceived, int total_mail, int *stratLine, int *mailSize, char *username);

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
            int stratLine[MAXMAIL], mailSize[MAXMAIL], total_mail, isDeleted[MAXMAIL];
            
            // Acknowledment of scuccessful acceptance to the client
            snprintf(buffer, sizeof(buffer), "+OK %s server ready\r\n", username);
            int x = send(clientSocketFD, buffer, strlen(buffer), 0);
            // Continuously receive command response from client
            char *commandFromClient;
            int auth = 0, user = 0, passw = 0;
            while(1)
            {
                dataReceive(clientSocketFD, dataReceived);

                // Process each command as required 
                char _dataReceived_[DATASIZE];
                strcpy(_dataReceived_, dataReceived);
                commandFromClient = strtok(_dataReceived_, " \t\n");
                
                // Calling different functions for different commands
                if(strcmp(commandFromClient, "USER") == 0)
                {
                    if(auth == 0)
                    {
                        respondToUSER(clientSocketFD, dataReceived, client_user);
                        user = 1;
                    }
                    else
                    {
                        sendError(clientSocketFD, "-ERR Authentication already done", 0);
                    }
                }
                else if(strcmp(commandFromClient, "PASS") == 0)
                {
                    if(auth == 0)
                    {
                        respondToPASS(clientSocketFD, dataReceived, password);
                        passw = 1;
                    }
                    else
                    {
                        sendError(clientSocketFD, "-ERR Authentication already done", 0);
                    }
                }
                else if(strcmp(commandFromClient, "STAT") == 0)
                {
                    if(auth == 1)
                    {
                        respondToSTAT(clientSocketFD, isDeleted, dataReceived, total_mail, mailSize);
                    }
                    else
                    {
                        sendError(clientSocketFD, "-ERR Authentication not done", 0);
                    }
                }
                else if(strcmp(commandFromClient, "LIST") == 0)
                {
                    if(auth == 1)
                    {
                        respondToLIST(clientSocketFD, isDeleted, dataReceived, total_mail, stratLine, mailSize);
                    }
                    else
                    {
                        sendError(clientSocketFD, "-ERR Authentication not done", 0);
                    }
                }
                else if(strcmp(commandFromClient, "RETR") == 0)
                {
                    if(auth == 1)
                    {
                        respondToRETR(clientSocketFD, isDeleted, dataReceived, total_mail, stratLine, mailSize, client_user);
                    }
                    else
                    {
                        sendError(clientSocketFD, "-ERR Authentication not done", 0);
                    }
                }
                else if(strcmp(commandFromClient, "DELE") == 0)
                {
                    if(auth == 1)
                    {
                        respondToDELE(clientSocketFD, isDeleted, total_mail, dataReceived);
                    }    
                    else
                    {
                        sendError(clientSocketFD, "-ERR Authentication not done", 0);
                    }
                }
                else if(strcmp(commandFromClient, "RSET") == 0)
                {
                    if(auth == 1)
                    {
                        respondToRSET(clientSocketFD, isDeleted, dataReceived, total_mail, stratLine, mailSize);
                    }    
                    else
                    {
                        sendError(clientSocketFD, "-ERR Authentication not done", 0);
                    }
                }
                else if(strcmp(commandFromClient, "QUIT") == 0)
                {
                    if(auth == 1)
                    {
                        respondToQUIT(clientSocketFD, isDeleted, dataReceived, total_mail, stratLine, mailSize, client_user);
                    }    
                    else
                    {
                        sendError(clientSocketFD, "-ERR Authentication not done", 0);
                    }
                }
                else
                {
                    fprintf(stderr, "invalid command received from client.\n Exiting... \n");
                    exit(EXIT_FAILURE);
                }

                // when user and pass is got, doing authentication 
                if(auth == 0 && user == 1 && passw == 1)
                {
                    char info[2 * MAXDETAILS];
                    info[0] = '\0';
                    strcat(info, client_user);
                    strcat(info, " ");
                    strcat(info, password);
                    strcat(info, "\n");

                    // printf("info : %s\n", info);


                    FILE *userFilePtr = fopen(userListFile, "r");
                    if (userFilePtr == NULL) 
                    {
                        fprintf(stderr, "Error opening %s.\n Exiting... \n", userListFile);
                        exit(EXIT_FAILURE);
                    }

                    char line[DATASIZE];
                    int isvalid=0;
                    while (fgets(line, sizeof(line), userFilePtr) != NULL) 
                    {
                        if (strcmp(line, info) == 0) {
                            isvalid = 1;
                            break;
                        }
                    }
                    fclose(userFilePtr);

                    if(isvalid)
                    {
                        auth = 1;
                        // printf("Auth done\n");
                        total_mail = loadInfo(client_user, stratLine, mailSize);
                        for (int i = 0; i < total_mail; i++)
                        {
                            isDeleted[i] = 0;
                        }
                    }
                    else
                    {
                        sendError(clientSocketFD, "-ERR Authentication failed", 0);
                    }
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

// Function to load information of all mail like id, size, starting line
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

    // for (int i = 0; i < total; i++)
    // {
    //     printf("%d  %d\n", stratLine[i], mailSize[i]);
    // }

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
    char c1 = '\0', c2 = '\0';
    while(1)
    {
        int bytesReceived = recv(cli_socket, buffer, sizeof(buffer), 0);
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

// Function to send error message to client
void sendError(int clientSocketFD, char *message, int ext)
{
    char buffer[BUFFERSIZE];
    snprintf(buffer, sizeof(buffer), "%s\r\n", message);
    send(clientSocketFD, buffer, strlen(buffer), 0);
    if(ext)
        exit(0);
}

// server's response to USER command
void respondToUSER(int clientSocketFD, char *dataReceived, char *clientuser)
{
    printf("%s\n", dataReceived);

    // Process each command as required 
    char _dataReceived_[DATASIZE];
    strcpy(_dataReceived_, dataReceived);
    char *cl_user = strtok(_dataReceived_, " \t\n");
    cl_user = strtok(NULL, " \t\n");

    strcpy(clientuser, cl_user);
    cl_user = strtok(NULL, " \t\n");
    if(cl_user != NULL)
    {
        printf("Invalid command received from client.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "+OK Please enter a password\r\n");
    send(clientSocketFD, buffer, strlen(buffer), 0);

}

// server's response to PASS command
void respondToPASS(int clientSocketFD, char *dataReceived, char *password)
{
    printf("%s\n", dataReceived);

    // Process each command as required 
    char _dataReceived_[DATASIZE];
    strcpy(_dataReceived_, dataReceived);
    char *cl_pw = strtok(_dataReceived_, " \t\n");
    cl_pw = strtok(NULL, " \t\n");

    strcpy(password, cl_pw);
    cl_pw = strtok(NULL, " \t\n");
    if(cl_pw != NULL)
    {
        printf("Invalid command received from client.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "+OK valid logon\r\n");
    send(clientSocketFD, buffer, strlen(buffer), 0);
}

// server's response to STAT command
// Sending total number of mails and total size
void respondToSTAT(int clientSocketFD, int *isDeleted, char *dataReceived, int total, int *MailSize)
{
    printf("%s\n", dataReceived);

    int t_size = 0;
    int tot = 0;
    for (int i = 0; i < total; i++)
    {
        if(isDeleted[i] == 0)
        {
            t_size += MailSize[i];
            tot ++;
        }
    }
    
    char buffer[MAX_BUFFER_SIZE];
    // printf("STAT : %d, %d\n", tot, t_size);
    snprintf(buffer, sizeof(buffer), "+OK %d %d\r\n", tot, t_size);
    send(clientSocketFD, buffer, strlen(buffer), 0);
}

// server's response to LIST command
// Listing all mail id and size 
// If index given then only this
void respondToLIST(int clientSocketFD, int *isDeleted, char *dataReceived, int total_mail, int *stratLine, int *mailSize)
{
    printf("%s\n", dataReceived);

    // Process each command as required 
    char _dataReceived_[DATASIZE];
    strcpy(_dataReceived_, dataReceived);
    char *arg2 = strtok(_dataReceived_, " \t\n");
    arg2 = strtok(NULL, " \t\n");

    if(arg2 == NULL)
    {
        char buffer[MAX_BUFFER_SIZE];

        int t_size = 0;
        int tot = 0;
        for (int i = 0; i < total_mail; i++)
        {
            if(isDeleted[i] == 0)
            {
                t_size += mailSize[i];
                tot ++;
            }
        }
        
        snprintf(buffer, sizeof(buffer), "+OK %d messages (%d octets)\r\n", tot, t_size);
        send(clientSocketFD, buffer, strlen(buffer), 0);

        for (int i = 0; i < total_mail; i++)
        {
            if(isDeleted[i] == 0)
            {
                snprintf(buffer, sizeof(buffer), "%d %d\r\n", i+1, mailSize[i]);
                send(clientSocketFD, buffer, strlen(buffer), 0);
            }
        }

        snprintf(buffer, sizeof(buffer), ".\r\n");
        send(clientSocketFD, buffer, strlen(buffer), 0);
    }
    else
    {
        int ind = atoi(arg2);
        ind--;

        if(ind >= total_mail)
        {
            sendError(clientSocketFD, "-ERR no such message", 0);
            return;
        }

        if(isDeleted[ind] == 1)
        {
            sendError(clientSocketFD, "-ERR No such message", 0);
            return;
        }

        char buffer[MAX_BUFFER_SIZE];

        snprintf(buffer, sizeof(buffer), "+OK %d %d\r\n", ind+1, mailSize[ind]);
        send(clientSocketFD, buffer, strlen(buffer), 0);
    }
}

// server's response to RETR command
// Receiving a whole mail with id
void respondToRETR(int clientSocketFD, int *isDeleted, char *dataReceived, int total_mail, int *stratLine, int *mailSize, char *username)
{
    // Process each command as required 
    char _dataReceived_[DATASIZE];
    strcpy(_dataReceived_, dataReceived);
    char *arg2 = strtok(_dataReceived_, " \t\n");
    arg2 = strtok(NULL, " \t\n");
    if(arg2 == NULL)
    {
        printf("Invalid command received from client.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }
    int ind = atoi(arg2);
    ind--;

    if(ind >= total_mail)
    {
        sendError(clientSocketFD, "-ERR no such message", 0);
        return;
    }

    printf("%s\n", dataReceived);

    if(isDeleted[ind] == 1)
    {
        sendError(clientSocketFD, "-ERR No such message", 0);
        return;
    }

    char buffer[MAX_BUFFER_SIZE];

    snprintf(buffer, sizeof(buffer), "+OK %d octets follow\r\n", mailSize[ind]);
    send(clientSocketFD, buffer, strlen(buffer), 0);

    // file
    char mailPath[PATHSIZE];
    snprintf(mailPath, sizeof(mailPath), "./%s/mymailbox", username);

    FILE *file = fopen(mailPath, "r");

    // Check if file opened successfully
    if (file == NULL) 
    {
        printf("Unable to open the file.\n");
        exit(0);
    }

    char line[80]; 
    int line_no = 0;
    int active = 0;
    // Read lines from the file using fgets
    while (fgets(line, sizeof(line), file) != NULL)
    {
        if(line_no == stratLine[ind])
        {
            active = 1;
        }

        if(active == 1)
        {
            line[strlen(line)-1] = '\0';
            snprintf(buffer, sizeof(buffer), "%s\r\n", line);
            send(clientSocketFD, buffer, strlen(buffer), 0);

            if(strcmp(buffer, ".\r\n") == 0)
            {
                break;
            }
        }    

        line_no++;
    }

    fclose(file);
}

// server's response to DELE command
// Deleting a mail
void respondToDELE(int clientSocketFD, int *isDeleted, int total_mail, char *dataReceived)
{
    printf("%s\n", dataReceived);

    // Process each command as required 
    char _dataReceived_[DATASIZE];
    strcpy(_dataReceived_, dataReceived);
    char *arg2 = strtok(_dataReceived_, " \t\n");
    arg2 = strtok(NULL, " \t\n");
    if(arg2 == NULL)
    {
        printf("Invalid command received from client.\n Exiting... \n");
        exit(EXIT_FAILURE);
    }
    int ind = atoi(arg2);
    ind--;

    if(ind >= total_mail)
    {
        sendError(clientSocketFD, "-ERR no such message", 0);
        return;
    }

    if(isDeleted[ind] == 1)
    {
        sendError(clientSocketFD, "-ERR message already deleted", 0);
        return;
    }

    isDeleted[ind] = 1;

    char buffer[BUFFERSIZE];
    snprintf(buffer, sizeof(buffer), "+OK message %d deleted\r\n", ind + 1);
    send(clientSocketFD, buffer, strlen(buffer), 0);
}

// server's response to RSET command
// Reseting all changes done
void respondToRSET(int clientSocketFD, int *isDeleted, char *dataReceived, int total_mail, int *stratLine, int *mailSize)
{
    printf("%s\n", dataReceived);

    int t_size = 0;
    for (int i = 0; i < total_mail; i++)
    {
        t_size += mailSize[i];
        isDeleted[i] = 0;
    }

    char buffer[BUFFERSIZE];
    snprintf(buffer, sizeof(buffer), "+OK maildrop has %d messages (%d octets)\r\n", total_mail, t_size);
    send(clientSocketFD, buffer, strlen(buffer), 0);
    return;
}

// server's response to QUIT command
// Here copying the non-deleted messages to a new file, delete old and renaming new file to 
// mymailbox
void respondToQUIT(int clientSocketFD, int *isDeleted, char *dataReceived, int total_mail, int *stratLine, int *mailSize, char *username)
{
    printf("%s\n", dataReceived);
    // file
    char mailPath[PATHSIZE], mailPath_new[PATHSIZE];
    snprintf(mailPath, sizeof(mailPath), "./%s/mymailbox", username);
    snprintf(mailPath_new, sizeof(mailPath_new), "./%s/mymailbox_", username);

    FILE *file = fopen(mailPath, "r");
    FILE *f2 = fopen(mailPath_new, "w");

    // Check if file opened successfully
    if (file == NULL || f2 == NULL) 
    {
        printf("Unable to open the file.\n");
        exit(0);
    }

    char line[100]; 
    int line_no = 0;
    int active = 0;
    int ind = 0;
    // Read lines from the file using fgets
    int end = 0;
    while (1)
    {
        if(line_no == stratLine[ind] && isDeleted[ind] == 0)
        {
            active = 1;
        }

        while(1)
        {
            if(fgets(line, sizeof(line), file) == NULL)
            {
                end = 1;
                break;
            }
            if(active == 1)
                fputs(line, f2);
            line_no++;
            if(strcmp(line, ".\n") == 0)
            {
                ind++;
                active = 0;
                break;
            }
        }   
        if(end)
        {
            break;
        }
    }

    fclose(file);
    fclose(f2);

    if (remove(mailPath) != 0)
    {
        perror("Error deleting file");
        return;
    }

    if (rename(mailPath_new, mailPath) != 0)
    {
        perror("Error renaming file");
        return;
    }

    char buffer[BUFFERSIZE];
    snprintf(buffer, sizeof(buffer), "+OK Bye-bye\r\n");
    send(clientSocketFD, buffer, strlen(buffer), 0);

    close(clientSocketFD);
    printf("Closing the connection...\n\n");
    exit(0);
}