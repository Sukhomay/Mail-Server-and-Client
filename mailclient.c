#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024

void send_mail(int smtp_port, const char *server_IP, const char *username);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_IP> <smtp_port> <pop3_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_IP = argv[1];
    int smtpPort = atoi(argv[2]);
    int pop3_port = atoi(argv[3]);

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
                send_mail(smtp_port, server_IP, username);
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

void send_mail(int smtp_port, const char *server_IP, const char *username) {
    int smtp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (smtp_socket == -1) {
        perror("Error creating SMTP socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(smtp_port);
    server_address.sin_addr.s_addr = inet_addr(server_IP);

    if (connect(smtp_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Error connecting to SMTP server");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received;

    // Receive initial greeting from server
    bytes_received = recv(smtp_socket, buffer, sizeof(buffer), 0);
    if (bytes_received == -1) {
        perror("Error receiving from SMTP server");
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0';
    printf("%s", buffer);  // Print server's initial response

    // Sending HELO command
    snprintf(buffer, sizeof(buffer), "HELO %s\r\n", username);
    send(smtp_socket, buffer, strlen(buffer), 0);

    // Receiving response for HELO command
    bytes_received = recv(smtp_socket, buffer, sizeof(buffer), 0);
    if (bytes_received == -1) {
        perror("Error receiving from SMTP server");
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0';
    printf("%s", buffer);  // Print server's response to HELO command

    // Get mail from user
    printf("Enter mail to be sent in the specified format:\n");
    printf("From: <%s>\n", username);
    printf("To: ");
    scanf("%s", buffer); // Assume only one recipient for simplicity
    printf("Subject: ");
    scanf(" %[^\n]s", buffer); // Read subject with spaces
    printf("Message body (End with a single dot on a new line):\n");

    // Construct and send mail data
    snprintf(buffer, sizeof(buffer), "From: <%s>\r\nTo: <%s>\r\nSubject: %s\r\n", username, buffer, buffer);
    send(smtp_socket, "DATA\r\n", 6, 0);
    send(smtp_socket, buffer, strlen(buffer), 0);
    send(smtp_socket, "\r\n.\r\n", 5, 0);

    // Receive response for mail data
    bytes_received = recv(smtp_socket, buffer, sizeof(buffer), 0);
    if (bytes_received == -1) {
        perror("Error receiving from SMTP server");
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0';
    printf("%s", buffer);  // Print server's response to mail data

    // Close connection with SMTP server
    send(smtp_socket, "QUIT\r\n", 6, 0);
    close(smtp_socket);
}
