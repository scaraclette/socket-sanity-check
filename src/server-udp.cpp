// Server side implementation of UDP client-server model
// Server when sending -> use servaddr
// Server when receiving -> use cliaddr
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;
  
#define PORT  6373
const int MAX_DATA_SIZE = 1460 / sizeof(int);
const int MAX_RECV = 20000;
  
void server_unreliable(const int sockfd, struct sockaddr_in servaddr, struct sockaddr_in fromaddr) {
    int buf[MAX_DATA_SIZE];
    unsigned int fromaddr_len = sizeof(fromaddr);

    for (int i = 0; i < MAX_RECV; i++) {
        int numbytes = recvfrom(sockfd, buf, MAX_DATA_SIZE * sizeof(int), 0, (struct sockaddr *)&fromaddr, &fromaddr_len);    
        if (numbytes == -1) {
            perror("recvfrom");
            return;
        }
        std::cout << buf[0] << std::endl;
        memset(buf, 0, MAX_DATA_SIZE * sizeof(int));
    }
}

void server_sanity_check(const int sockfd, struct sockaddr_in from_addr) {
    // recvfrom: fromaddr
    // sendto: fromaddr
    int buf[MAX_DATA_SIZE];
    int send_ack[sizeof(int)];
    unsigned int from_addr_len = sizeof(from_addr);

    // recvfrom
    int numbytes = recvfrom(sockfd, buf, MAX_DATA_SIZE * sizeof(int), 0, (struct sockaddr *)&from_addr, &from_addr_len);    
    if (numbytes == -1) {
        perror("recvfrom");
        return;
    }

    std::cout << "Received from client: " << buf[0] << std::endl;
    std::cout << "send ack to client..." << std::endl;
    send_ack[0] = buf[0] + 1;
    // sendto
    int send_to = sendto(sockfd, send_ack, sizeof(int), 0, (const struct sockaddr *) &from_addr, from_addr_len);
    if (send_to == -1) {
        perror("sendto");
        return;
    }
}

/**
 * Server Stop and Wait
 * Repeats receiving message[] and sending an acknowledgement at a server side max (=20,000) times using the sock object.
 */
void server_stop_wait(const int sockfd, struct sockaddr_in from_addr) {
    int buf[MAX_DATA_SIZE];
    int send_ack[sizeof(int)];
    unsigned int from_addr_len = sizeof(from_addr);
    int ack = -1;

    while (true) {
        // recvfrom
        int numbytes = recvfrom(sockfd, buf, MAX_DATA_SIZE * sizeof(int), 0, (struct sockaddr *)&from_addr, &from_addr_len);    
        if (numbytes == -1) {
            perror("recvfrom");
            return;
        }

        if (buf[0] > ack) {
            // std::cout << "here" << std::endl;
            ack = buf[0];
        }
        send_ack[0] = buf[0];
        std::cout << send_ack[0] << std::endl;

        int send_to = sendto(sockfd, send_ack, sizeof(int), 0, (const struct sockaddr *) &from_addr, from_addr_len);
        if (send_to == -1) {
            perror("sendto");
            return;
        }

        memset(buf, 0, sizeof(buf));
        memset(send_ack, 0, sizeof(send_ack));
        if (ack == MAX_RECV - 1) {
            break;
        }
    }
}

// Driver code
int main() {
    int sockfd;
    char buffer[sizeof(unsigned int)];
    struct sockaddr_in serv_addr, from_addr;
      
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
      
    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(&from_addr, 0, sizeof(from_addr));
      
    // Filling server information
    serv_addr.sin_family    = AF_INET; // IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
      
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&serv_addr, 
            sizeof(serv_addr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
      
    // server_unreliable(sockfd, servaddr, fromaddr);
    // server_sanity_check(sockfd, from_addr);
    server_stop_wait(sockfd, from_addr);



    // int n;
    // unsigned int len = sizeof(cliaddr);

    // std::cout << "listening for client..." << std::endl;
  
    // while (1) {  
    //     n = recvfrom(sockfd, (char *)buffer, sizeof(unsigned int), 
    //             MSG_WAITALL, ( struct sockaddr *) &cliaddr,
    //             &len);
	// cout << *((unsigned int *) buffer) << endl;
    // }
      
    return 0;
}
