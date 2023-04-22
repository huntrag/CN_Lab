/*
    Name: Raghav Gade
    20cs02003
*/

// Send a message and it will reverse it
// In order to end connection type bye

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_ADDRESS "127.0.0.1"
#define PORT 5000
#define MAX_BUFFER_SIZE 1024

int main()
{
    int client_socket, len;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE];

    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // This is something for UDP thats not for tcp
    // This connects client with server
    if (inet_pton(AF_INET, SERVER_ADDRESS, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        memset(buffer, 0, MAX_BUFFER_SIZE);

        printf("Enter message: ");
        fgets(buffer, MAX_BUFFER_SIZE, stdin);

        // Send message to server
        if (sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            perror("Send error");
            exit(EXIT_FAILURE);
        }

        printf("Message sent to server\n");

        // Receive message from server
        len = sizeof(server_addr);
        if (recvfrom(client_socket, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &len) < 0)
        {
            perror("Receive error");
            exit(EXIT_FAILURE);
        }

        printf("Message received from server: %s\n", buffer);
    }

    close(client_socket);
    return 0;
}
