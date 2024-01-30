#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<regex.h>

#define MAX_BUFFER_SIZE 1024

void send_mail(int smtp_port, const char *server_IP, char *username);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_IP> <smtp_port> <pop3_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *serverIP = argv[1];
    int smtpPort = atoi(argv[2]);
    int pop3Port = atoi(argv[3]);

    char username[100];
    printf("Enter username: ");
    scanf("%s", username);

    int option;
    do {
        printf("\nOptions:\n");
        printf("1. Manage Mail\n");
        printf("2. Send Mail\n");
        printf("3. Quit\n");
        printf("Enter option: ");
        scanf("%d", &option);

        switch (option) {
            case 1:
                // Implement manage mail functionality
                printf("Manage Mail option selected.\n");
                break;
            case 2:
                // Send mail functionality
                send_mail(smtpPort, serverIP, username);
                break;
            case 3:
                printf("Quitting the program.\n");
                break;
            default:
                printf("Invalid option. Please try again.\n");
        }
    } while (option != 3);

    return 0;
}

void send_mail(int smtp_port, const char *server_IP, char *username)
{
    int smtp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (smtp_socket == -1) {
        perror("Error creating SMTP socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(smtp_port);
    server_address.sin_addr.s_addr = inet_addr(server_IP);

    if (connect(smtp_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Error connecting to SMTP server");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_BUFFER_SIZE];
    size_t bytesReceived;

    // Receive initial greeting from server
    bytesReceived = recv(smtp_socket, buffer, sizeof(buffer), 0);
    if (bytesReceived == -1)
    {
        perror("Error receiving from SMTP server");
        exit(EXIT_FAILURE);
    }
    buffer[bytesReceived] = '\0';
    printf("%s", buffer);  // Print server's initial response

    // Sending HELO command
    char *domain = strtok(username, "@");
    domain = strtok(NULL, "@");
    snprintf(buffer, sizeof(buffer), "HELO %s\r\n", domain);
    send(smtp_socket, buffer, strlen(buffer), 0);

    // Receiving response for HELO command
    bytesReceived = recv(smtp_socket, buffer, sizeof(buffer), 0);
    if (bytesReceived == -1) 
    {
        perror("Error receiving from SMTP server");
        exit(EXIT_FAILURE);
    }
    buffer[bytesReceived] = '\0';
    printf("%s", buffer);  // Print server's response to HELO command

    // Get mail from user
    char recipient[100];
    char subject[100];
    printf("Enter mail to be sent in the specified format:\n");

    char *mailContent[50];
    for (int i = 0; i < 50; i++)
    {
        mailContent[i] = (char *)malloc(80*sizeof(char));
    }
    const char *line0Pattern = "From: [^@]+@[^@]+";
    const char *line1Pattern = "To: [^@]+@[^@]+";
    const char *line2Pattern = "Subject: [^@]*";
    int ind = 0;
    while(1)
    {
        scanf(" %[^\n]s", mailContent[ind]);

        if(ind == 0)
        {

        }
        else if(ind == 1)
        {

        }
        else if(ind == 2)
        {

        }
        else
        {

        }
    }
    // printf("From: <%s>\n", username);
    // printf("To: ");
    // scanf("%s", recipient); // Assume only one recipient for simplicity


    // snprintf(buffer, sizeof(buffer), "MAIL FROM: %s\r\n", username);
    // send(smtp_socket, buffer, strlen(buffer), 0);

    // // Receiving response for MAIL command
    // bytesReceived = recv(smtp_socket, buffer, sizeof(buffer), 0);
    // if (bytesReceived == -1) 
    // {
    //     perror("Error receiving from SMTP server");
    //     exit(EXIT_FAILURE);
    // }
    // buffer[bytesReceived] = '\0';
    // printf("%s", buffer);  // Print server's response to MAIL command
    
    // snprintf(buffer, sizeof(buffer), "RCPT TO: %s\r\n", recipient);
    // send(smtp_socket, buffer, strlen(buffer), 0);

    // // Receiving response for RCPT command
    // bytesReceived = recv(smtp_socket, buffer, sizeof(buffer), 0);
    // if (bytesReceived == -1) 
    // {
    //     perror("Error receiving from SMTP server");
    //     exit(EXIT_FAILURE);
    // }
    // buffer[bytesReceived] = '\0';
    // printf("%s", buffer);  // Print server's response to RCPT command


    // printf("Subject: ");
    // scanf(" %[^\n]s", subject); // Read subject with spaces

    // // Construct and send mail data
    // snprintf(buffer, sizeof(buffer), "From: <%s>\r\nTo: <%s>\r\nSubject: %s\r\n", username, recipient, subject);
    // send(smtp_socket, "DATA\r\n", 6, 0);
    // send(smtp_socket, buffer, strlen(buffer), 0);


    // printf("Message body (End with a single dot on a new line):\n");
    // char dataLine[100];
    // while(1)
    // {
    //     scanf(" %[^\n]s", dataLine);
    //     snprintf(buffer, sizeof(buffer), "%s\r\n", dataLine);
    //     send(smtp_socket, buffer, strlen(buffer), 0);

    //     if(strcmp(dataLine, ".") == 0)
    //         break;
    // }

    // Receive response for command DATA
    // bytesReceived = recv(smtp_socket, buffer, sizeof(buffer), 0);
    // if (bytesReceived == -1) {
    //     perror("Error receiving from SMTP server");
    //     exit(EXIT_FAILURE);
    // }
    // buffer[bytesReceived] = '\0';
    // printf("%s", buffer);  // Print server's response to command DATA

    // Close connection with SMTP server
    send(smtp_socket, "QUIT\r\n", 6, 0);
    close(smtp_socket);
}
