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

using namespace std;
  
#define PORT 6373
const int MAX_MESSAGE_SIZE = 1460 / sizeof(int);
const int MAX_SEND = 20000;
  
void client_unreliable(const int sockfd, int message[], struct sockaddr_in servaddr) {
    unsigned int servaddr_len = sizeof(servaddr);
    for (int i = 0; i < MAX_SEND; i++) {
        message[0] = i;
        int send_to = sendto(sockfd, message, MAX_MESSAGE_SIZE * sizeof(int), 0, (struct sockaddr *)&servaddr, servaddr_len);
        if (send_to == -1) {
            perror("sendto");
            break;
        }
    }
}

void client_sanity_check(const int sockfd, int message[], struct sockaddr_in serv_addr, struct sockaddr_in from_addr) {
    // sendto: serveraddr
    // recvfrom: fromaddr

    // len
    unsigned int from_addr_len = sizeof(from_addr);
    unsigned int serv_addr_len = sizeof(serv_addr);

    // int array to receive ack
    int ack_buf[sizeof(int)];
    ack_buf[0] = -1;
    message[0] = 33;

    // send_to
    int send_to = sendto(sockfd, message, MAX_MESSAGE_SIZE * sizeof(int), 0, (struct sockaddr *)&serv_addr, serv_addr_len);
    if (send_to == -1) {
        perror("sendto");
        return;
    }

    // recv_from
    int recv_from = recvfrom(sockfd, ack_buf, sizeof(int), 0, (struct sockaddr *) &from_addr, &from_addr_len);
    if (recv_from == -1) {
        perror("recvfrom");
        return;
    }

    std::cout << "From server: " << ack_buf[0] << std::endl;
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
    he = gethostbyname("127.0.0.1");
    // Filling server information
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*he->h_addr_list));
      
    // client_unreliable(sockfd, message, servaddr);
    client_sanity_check(sockfd, message, serv_addr, from_addr);


    // int n, len;
    // for (unsigned int i = 0; i < 10000; i++) {     
    // sendto(sockfd, (const unsigned int *)&i, sizeof(unsigned int),
    //     0, (const struct sockaddr *) &servaddr, 
    //         sizeof(servaddr));
    // }
    // std::cout << "done sending!" << std::endl;
  
    close(sockfd);
    return 0;
}