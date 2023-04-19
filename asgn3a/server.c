/*
    Raghav Gade
    20CS02003
*/
// Run the server.c and then put your inputs using client.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

double calculate(double num1, double num2, int op, int flag)
{
    double ans;
    switch (op)
    {

    case 1:
        ans = num1 + num2;
        break;

    case 2:
        ans = num1 - num2;
        break;

    case 3:
        ans = num1 * num2;
        break;

    case 4:
        if (num2 == 0)
        {
            flag = 1;
            break;
        }
        ans = num1 / num2;
        break;

    default:
        flag = 1;
        ans = 0;
    }

    return ans;
}

int digits(int num)
{
    int ans = 0;
    while (num > 0)
    {
        num = num / 10;
        ans++;
    }
    // printf("no :%d\n", ans);
    return ans;
}

int main(void)
{
    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;
    char server_message[2000], client_message[2000];

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0)
    {
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bind to the set port and IP:
    if (bind(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");

    // Listen for clients:
    if (listen(socket_desc, 1) < 0)
    {
        printf("Error while listening\n");
        return -1;
    }
    while (1)
    {
        printf("\nListening for incoming connections.....\n");

        // Accept an incoming connection:
        client_size = sizeof(client_addr);
        client_sock = accept(socket_desc, (struct sockaddr *)&client_addr, &client_size);

        if (client_sock < 0)
        {
            printf("Can't accept\n");
            return -1;
        }
        printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Receive client's message:
        if (recv(client_sock, client_message, sizeof(client_message), 0) < 0)
        {
            printf("Couldn't receive\n");
            return -1;
        }
        //      Thread implementation should be here

        // Create a thread and then call a function then it will keep engaging the given client while server looks for other client

        printf("Msg from client: %s\n", client_message);

        char client_name[100];

        char snum1[20], snum2[20];
        int i = 0;
        int ind = 0;
        while (client_message[ind] != ' ')
        {
            client_name[i] = client_message[ind];
            i++;
            ind++;
        }
        ind++;
        i = 0;
        while (client_message[ind] != ' ')
        {
            snum1[i] = client_message[ind];
            i++;
            ind++;
        }
        snum2[i] = '\0';
        ind++;
        i = 0;
        while (client_message[ind] != ' ')
        {
            snum2[i] = client_message[ind];
            i++;
            ind++;
        }
        snum2[i] = '\0';
        ind++;
        char op = client_message[ind];

        // --------------------------------Printing
        printf("Client name: %s\n", client_name);
        printf("Operator %c\n", op);
        // -------------------------------------------

        double num1 = atof(snum1);
        double num2 = atof(snum2);
        int flag = 0;
        double ans = calculate(num1, num2, op - '0', flag);
        int d1 = digits((int)ans);
        //      Create the client message
        if (flag)
        {
            strcpy(server_message, "Error.");
        }
        else
        {
            gcvt(ans, 6 + d1, server_message);
        }

        if (send(client_sock, server_message, strlen(server_message), 0) < 0)
        {
            printf("Can't send\n");
            return -1;
        }
        // Closing the socket:
        close(client_sock);

        // Join the threads here
    }

    close(socket_desc);
    return 0;
}