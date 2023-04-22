/*
    Name: Raghav Gade
    Roll Number: 20CS02003
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 13

// Port No 13 is reserved specifically for tcp/ip connecctions and specifically for day time

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    if (read(sockfd, buffer, sizeof(buffer)) < 0)
    {
        perror("read failed");
        exit(EXIT_FAILURE);
    }

    printf("Time: %s", buffer);

    close(sockfd);

    return 0;
}
