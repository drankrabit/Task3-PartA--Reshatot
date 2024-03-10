#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define MAXLINE 1024

char *util_generate_random_data(unsigned int size) {
    char *buffer = NULL;
    if (size == 0)
        return NULL;
    buffer = (char *)calloc(size, sizeof(char));
    if (buffer == NULL)
        return NULL;
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++)
        *(buffer + i) = ((unsigned int)rand() % 256);
    return buffer;
}

int main(int argc, char *argv[]) {
    // Checks if the command that was put in the terminal is in the correct format
    if (argc != 7 || strcmp(argv[1], "-ip") != 0 || strcmp(argv[3], "-p") != 0 || strcmp(argv[5], "-algo") != 0) {
        fprintf(stderr, "Usage: %s -ip <IP> -p <PORT> -algo <ALGO>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // Initializes all the things needed for the run
    int sockfd;
    struct sockaddr_in servaddr;
    char *algorithm = argv[6];

    printf("Starting Sender...\n");
    printf("Waiting for TCP connection...\n");

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[2]);
    servaddr.sin_port = htons(atoi(argv[4]));

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    else {
        printf("Receiver connected, beginning to send file...\n");
    }

    char user_input[MAXLINE];

    // Initialize congestion window
    int cwnd = MAXLINE;

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &algorithm, strlen(algorithm)) < 0) {
            perror("Error setting congestion control algorithm");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

    while (1) {
        // Generate random data
        char *data = util_generate_random_data(cwnd);

        // Send data to server
        clock_t start_time = clock();
        int bytes_sent = send(sockfd, data, cwnd, 0);
        if (bytes_sent < 0) {
            perror("Error sending data");
            free(data);
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // Record the time taken to send data
        clock_t end_time = clock();
        double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

        // Send time taken to the receiver
        send(sockfd, &time_taken, sizeof(double), 0);

        // Wait for response from the receiver
        printf("Data sent. Waiting for Receiver response...\n");
        int bytes_received = recv(sockfd, user_input, MAXLINE, 0);
        if (bytes_received < 0) {
            perror("Error receiving response");
            free(data);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        
        printf("Receiver sent Acknowledgement message...\n");

        // Ask user if they want to send another file
        printf("Do you want to send another file? (Y/N): ");
        fgets(user_input, MAXLINE, stdin);
        strtok(user_input, "\n"); // Remove newline character

        // Check user input
        if (strcmp(user_input, "N") == 0 || strcmp(user_input, "n") == 0) {
            printf("Sender end.\n");
            // Send exit message to receiver
            if (send(sockfd, "exit", strlen("exit"), 0) < 0) {
                perror("Error sending exit message");
                exit(EXIT_FAILURE);
            }
            close(sockfd);
            free(data);
            break;
        }
        
    }

    return 0;
}
