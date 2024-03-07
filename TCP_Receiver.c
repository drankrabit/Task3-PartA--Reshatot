#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define MAXLINE 1024

void print_statistics(double *time_taken, double *bandwidth, int num_runs) {
    double avg_time = 0.0, avg_bandwidth = 0.0;
    printf("----------------------------------\n");
    printf("- * Statistics * -\n");
    // Starts to loop to print all of the statistics for each run
    for (int i = 0; i < num_runs; i++) {
        printf("- Run #%d Data: Time=%.6f seconds; Speed=%.2f MB/s\n", i + 1, time_taken[i], bandwidth[i]);
        // Sums all the times and bandwidths for average calculation
        avg_time += time_taken[i];
        avg_bandwidth += bandwidth[i];
    }
    // Calculates the average
    avg_time /= num_runs;
    avg_bandwidth /= num_runs;
    printf("-\n");
    printf("- Average time: %.6f seconds\n", avg_time);
    printf("- Average bandwidth: %.2f MB/s\n", avg_bandwidth);
    printf("----------------------------------\n");
}

int main(int argc, char *argv[]) {
    // Checks if the command that was put in the terminal is in the correct format
    if (argc != 5 || strcmp(argv[1], "-p") != 0 || strcmp(argv[3], "-algo") != 0) {
        fprintf(stderr, "Usage: %s -p <PORT> -algo <ALGO>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Initializes all the things needed for the run
    int listenfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);
    char *algorithm = argv[4];
    char response[MAXLINE];

    printf("Starting Receiver...\n");
    printf("Waiting for TCP connection...\n");
    
    // Create socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR option (overides the Port in the case that it is already in use)
    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Set SO_REUSEADDR failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[2]));

    // Bind socket
    if (bind(listenfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        perror("Socket bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(listenfd, 5) != 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // Accept connection
    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);
    if (connfd < 0) {
        perror("Server accept failed");
       exit(EXIT_FAILURE);
    }
    printf("Sender connected, beginning to receive file...\n");
    // Initializes the arrays for the times, bandwidths and number of runs we have done
    double time_taken[MAXLINE];
    double bandwidth[MAXLINE];
    int num_runs = 0;
    while(1) {
        // Receive data from sender
        char buffer[MAXLINE];
        int bytes_received = recv(connfd, buffer, MAXLINE, 0);
        if (bytes_received < 0) {
            perror("Error receiving data");
            exit(EXIT_FAILURE);
        }
        buffer[bytes_received] = '\0';

        // Checks if the data from the sender points towards a shutdown process
        if (strcmp(buffer, "exit") == 0) {
            printf("Sender requests to exit. Exiting the program.\n");
            close(connfd);
            break;
        }

        // Receive time taken to send data from sender
        double time;
        recv(connfd, &time, sizeof(double), 0);
        time_taken[num_runs] = time;

        // Print "File transfer completed"
        printf("File transfer completed.\n");

        // Print "Waiting for Sender response..."
        printf("Waiting for Sender response...\n");

        // Measure bandwidth
        double bandwidth_run = (double)bytes_received / (time * 1024 * 1024); // in MB/s
        bandwidth[num_runs] = bandwidth_run;
        num_runs++;

        // Send ACK (Acknowledgement) back to sender
        send(connfd, "Received", strlen("Received"), 0);
    }

    // Print statistics
    print_statistics(time_taken, bandwidth, num_runs);
    printf("Receiver end.\n");
    close(listenfd);

    return 0;
}
