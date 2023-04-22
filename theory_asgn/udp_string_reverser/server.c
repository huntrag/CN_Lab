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

#define PORT 5000
#define MAX_BUFFER_SIZE 1024

int main()
{
    int server_socket, len;
    struct sockaddr_in server_addr, client_addr;
    char buffer[MAX_BUFFER_SIZE];

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    // INADDR_ANY is to accept messages from any ip
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind error");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        memset(buffer, 0, MAX_BUFFER_SIZE);

        // Receive message from client
        len = sizeof(client_addr);
        int buf_size = 0;
        if ((buf_size = recvfrom(server_socket, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &len)) < 0)
        {
            perror("Receive error");
            exit(EXIT_FAILURE);
        }

        printf("Received message: %s & %d\n", buffer, buf_size);

        if (strcmp(buffer, "bye") == 0)
        {
            break;
        }
        // Send the same message back to client
        char temp;
        for (int i = 0; i <= (buf_size - 2) / 2; i++)
        {
            temp = buffer[i];
            buffer[i] = buffer[buf_size - 2 - i];
            buffer[buf_size - 2 - i] = temp;
        }

        if (sendto(server_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr, len) < 0)
        {
            perror("Send error");
            exit(EXIT_FAILURE);
        }

        printf("Message sent back to client\n");
    }

    close(server_socket);
    return 0;
}
