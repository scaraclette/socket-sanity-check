// Client side implementation of UDP client-server model
// Client when sending -> use cliaddr
// Client when receiving -> use servaddr
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <iostream>
#include <sys/time.h>
#include <vector>
#include <chrono>

  
#define PORT 6373
const int MAX_MESSAGE_SIZE = 1460 / sizeof(int);
const int TIMEOUT = 1500;
const int MAX_SEND = 20000;
  
/**
 * Unreliable Test
 * Implements natural behavior of UDP packet transmissions
 */
void client_unreliable(const int sockfd, int message[], struct sockaddr_in serv_addr) {
    unsigned int servaddr_len = sizeof(serv_addr);
    for (int i = 0; i < MAX_SEND; i++) {
        message[0] = i;
        int send_to = sendto(sockfd, message, MAX_MESSAGE_SIZE * sizeof(int), 0, (struct sockaddr *)&serv_addr, servaddr_len);
        if (send_to == -1) {
            perror("sendto");
            break;
        }
    }
}

/**
 * Stop-and-Wait test
 * Implements the stop and wait algorithm (can be done as a sliding window with a size = 1).
 */
int client_stop_wait(const int sockfd, int message[], struct sockaddr_in serv_addr, struct sockaddr_in from_addr) {

    // addr lengths
    unsigned int from_addr_len = sizeof(from_addr);
    unsigned int serv_addr_len = sizeof(serv_addr);

    // int array to receive ack
    int ack_buf[sizeof(int)];
    ack_buf[0] = -1;

    // Count retransmit
    int retransmit_count = 0;

    for (int i = 0; i < MAX_SEND; i++) {
        message[0] = i;

        // Send to server blocked
        int send_to = sendto(sockfd, message, MAX_MESSAGE_SIZE * sizeof(int), 0, (struct sockaddr *)&serv_addr, serv_addr_len);
        if (send_to == -1) {
            perror("sendto");
            return -1;
        }

        // Non-block recvfrom to receive ack
        int recv_from = recvfrom(sockfd, ack_buf, sizeof(int), MSG_DONTWAIT, (struct sockaddr *) &from_addr, &from_addr_len);
        if (recv_from == -1) {
            perror("recvfrom");
            
            // Start timer;
            struct timeval start_time;
            struct timeval stop_time;

            // Initialize ack to -1 until we get correct value
            int ack = -1;

            // Initialize retransmit flag
            bool retransmit = true;

            // Start timer and re-transmit until we get correct ack
            while (ack != i) {
                // loop until elapsed time usec == 1500
                gettimeofday(&start_time, NULL);
                unsigned long long elapsed_time_usec = 0;

                std::cout << "restart timer" << std::endl;
                while (elapsed_time_usec < TIMEOUT) {
                    // update elapsed time
                    gettimeofday(&stop_time, NULL);
                    elapsed_time_usec = (((stop_time.tv_sec - start_time.tv_sec) * 1000000) + (stop_time.tv_usec - start_time.tv_usec));

                    recv_from = recvfrom(sockfd, ack_buf, sizeof(int), MSG_DONTWAIT, (struct sockaddr *) &from_addr, &from_addr_len);
                    ack_buf[recv_from] = '\0';
                    ack = ack_buf[0];
                    // std::cout << "received ack: " << ack << std::endl; 

                    if (ack == i) {
                        retransmit = false;
                        break;
                    }
                }
                // If ack is still incorrect, retransmit message
                if (retransmit) {
                    std::cout << "TIMEOUT!" << std::endl;
                    retransmit_count++;
                    std::cout << "retransmit count " << retransmit_count << std::endl;
                    int send_to = sendto(sockfd, message, MAX_MESSAGE_SIZE * sizeof(int), 0, (struct sockaddr *)&serv_addr, serv_addr_len);
                    if (send_to == -1) {
                        perror("sendto");
                        return -1;
                    }
                }
            }

            // Ack is now correct
            std::cout << "current ack: " << ack << ", expected ack: " << i << std::endl << std::endl;  
        } else {
            int ack = ack_buf[0];
            std::cout << "client received ack right away: " << ack << " at i: " << i << std::endl << std::endl;
        }
        memset(ack_buf, 0, sizeof(ack_buf));
    }

    std::cout << "retransmit count: " << retransmit_count << std::endl;
    return retransmit_count;
}

/**
 * Sliding Window test
 * Implements sliding window algorithm with Go-back-n protocol
 */
int client_sliding_window(const int sockfd, int message[], struct sockaddr_in serv_addr, struct sockaddr_in from_addr, int window_size = 4) {
    std::cout << "Sliding Window with size: " << window_size << std::endl;
    // addr lengths
    unsigned int from_addr_len = sizeof(from_addr);
    unsigned int serv_addr_len = sizeof(serv_addr);
    // int array to receive ack
    int ack_buf[sizeof(int)];

    // Count retransmit
    int retransmit_count = 0;

    // Sliding window variables
    int base = 0;
    int next_seq = 0;

    struct timeval start_time;
    struct timeval stop_time;
    unsigned long long elapsed_time_usec = 0;
    // long long elapsed_time_usec = 0;
    bool timer_start = false;
    bool starting_point = false;
    bool timeout = true;

    while (true) {
        if (next_seq < base + window_size) {
            message[0] = next_seq;
            // Send to server blocked
            int send_to = sendto(sockfd, message, MAX_MESSAGE_SIZE * sizeof(int), 0, (struct sockaddr *)&serv_addr, serv_addr_len);
            next_seq++;
        }

        int recv_from = recvfrom(sockfd, ack_buf, sizeof(int), MSG_DONTWAIT, (struct sockaddr *)&from_addr, &from_addr_len);
        if (recv_from != 1) {
            int ack = ack_buf[0];
            if (ack >= base) {
                base = ack_buf[0] + 1;
            }
            
            if (base == next_seq) {
                // stop the timer
                start_time.tv_sec = 0;
                start_time.tv_usec = 0;

                stop_time.tv_usec = 0;
                stop_time.tv_sec = 0;

                elapsed_time_usec = 0;

                // Set timer to be inactive
                timer_start = false;
            } else {
                if (!timer_start) {
                    gettimeofday(&start_time, NULL);
                    elapsed_time_usec = 0;
                }

                // Run new timer
                timer_start = true;
                if (timeout) {
                    starting_point = true;
                }
            }
        }
       
        if (timer_start && !starting_point) {
            // Calculate time only if the timer didn't start in this iteration
            gettimeofday(&stop_time, NULL);
            elapsed_time_usec = (((stop_time.tv_sec - start_time.tv_sec) * 1000000) + (stop_time.tv_usec - start_time.tv_usec));        
        } else {
            // Timer just started for this loop, set starting_point and timout to false
            starting_point = false;
            timeout = false;
        }

        if (elapsed_time_usec > TIMEOUT) {
            timeout = true;
            timer_start = true;
            gettimeofday(&start_time, NULL);
            elapsed_time_usec = 0;
            for (int i = base; i < next_seq; i++) {
                retransmit_count++;
                message[0] = i;
                int send_to = sendto(sockfd, message, MAX_MESSAGE_SIZE * sizeof(int), 0, (struct sockaddr *)&serv_addr, serv_addr_len);
                if (send_to == -1) {
                    perror("sendto");
                    return -1;
                }
            }
        }

        memset(ack_buf, 0, sizeof(ack_buf));
        // Break when all messages are sent
        if (base == MAX_SEND) {
            std::cout << "All messages sent!" << std::endl;
            break;
        }
    }

    return retransmit_count;
}

/**
 * Function helper to test sliding window
 */
void error_test_sliding_window(int sockfd, int message[], struct sockaddr_in serv_addr, struct sockaddr_in from_addr) {
    std::vector<int> retransmits_vector;
    std::vector<unsigned long long> elapsed_time_msec_vector;

    // Start timer;
    struct timeval start_time;
    struct timeval stop_time;

    int i = 1;
    std::cout << "Choose window size: ";
    std::cin >> i;

    std::cout << "current: " << i << std::endl;
    gettimeofday(&start_time, NULL);

    int sliding_window_retransmits = client_sliding_window(sockfd, message, serv_addr, from_addr, i);
    retransmits_vector.push_back(sliding_window_retransmits);

    gettimeofday(&stop_time, NULL);

    unsigned long long elapsed_time_msec = (((stop_time.tv_sec - start_time.tv_sec) * 1000000) + (stop_time.tv_usec - start_time.tv_usec)) / 1000;

    elapsed_time_msec_vector.push_back(elapsed_time_msec);
    std::cout << "retransmit: " << sliding_window_retransmits << std::endl;
    std::cout << "elapsed time ms: " << elapsed_time_msec << std::endl << std::endl;

    std::cout << "RETRANSMITS: " << std::endl;
    for (int i = 0; i < retransmits_vector.size(); i++) {
        std::cout << retransmits_vector[i] << "," << std::endl;
    }

    std::cout << "ELAPSED TIME: " << std::endl;
    for (int i = 0; i < elapsed_time_msec_vector.size(); i++) {
        std::cout << elapsed_time_msec_vector[i] << "," << std::endl;
    }
}

/**
 * Helper function to get user input
 */
int test_user_input() {
    int test_case;
    std::cout << "Choose a testcase" << 
        "\n\t1: unreliable test" <<
        "\n\t2: stop-and-wait test" << 
        "\n\t3: sliding window" <<
        "\n\t4: sliding window with errors" <<
        "\n--> ";
    std::cin >> test_case;

    return test_case;
}

// Driver code
int main() {
    int sockfd;
    struct sockaddr_in serv_addr, from_addr;
    int message[MAX_MESSAGE_SIZE];
  
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
  
    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(&from_addr, 0, sizeof(from_addr));
    struct hostent        *he;      
    // he = gethostbyname("127.0.0.1");

    // Linux lab: 2
    he = gethostbyname("10.155.176.23");

    // Filling server information
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*he->h_addr_list));

    // Get user input
    int test_case = test_user_input();
    int stop_wait_retransmits = 0;
    int sliding_window_retransmits = 0;
      
    switch (test_case)
    {
    case 1:
        std::cout << "Unreliable test chosen!" << std::endl;
        client_unreliable(sockfd, message, serv_addr);
        break;
    case 2:
        std::cout << "Stop-and-Wait test chosen!" << std::endl;
        stop_wait_retransmits = client_stop_wait(sockfd, message, serv_addr, from_addr);
        std::cout << "Packet retransmits: " << stop_wait_retransmits << std::endl;
        break;
    case 3:
        std::cout << "Sliding Window (Go-Back-N) test chosen!" << std::endl;
        // Default transmit window size is 4
        sliding_window_retransmits = client_sliding_window(sockfd, message, serv_addr, from_addr);
        std::cout << "Packet retransmits: " << stop_wait_retransmits << std::endl;
        break;
    case 4:
        std::cout << "Sliding Window (Go-Back-N) with errors test chosen!" << std::endl;
        error_test_sliding_window(sockfd, message, serv_addr, from_addr);
        break;
    default:
        std::cout << "No valid testcase chosen!" << std::endl;
        break;
    }
  
    close(sockfd);
    return 0;
}