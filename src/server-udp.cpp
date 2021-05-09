// Server side implementation of UDP client-server model
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
  
void server_unreliable(const int sockfd, struct sockaddr_in servaddr, struct sockaddr_in cliaddr) {
    int buf[MAX_DATA_SIZE];
    unsigned int cliaddr_len = sizeof(cliaddr);

    for (int i = 0; i < MAX_RECV; i++) {
        int numbytes = recvfrom(sockfd, buf, MAX_DATA_SIZE * sizeof(int), 0, (struct sockaddr *)&cliaddr, &cliaddr_len);    
        if (numbytes == -1) {
            perror("recvfrom");
            return;
        }
        std::cout << buf[0] << std::endl;
        memset(buf, 0, MAX_DATA_SIZE * sizeof(int));
    }
}

// Driver code
int main() {
    int sockfd;
    char buffer[sizeof(unsigned int)];
    struct sockaddr_in servaddr, cliaddr;
      
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
      
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
      
    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
      
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
      
    server_unreliable(sockfd, servaddr, cliaddr);
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
