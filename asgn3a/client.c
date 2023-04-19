#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX 1000

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

void createClientMessage(char *message, char *client_name, float num1, float num2, int op_type)
{
    int ind = 0;
    int i = 0;
    while (client_name[i] != '\0')
    {
        message[ind] = client_name[i];
        ind++;
        i++;
    }
    message[ind] = ' ';
    ind++;
    // printf("%f %f\n", num1, num2);
    int d1 = digits((int)num1), d2 = digits((int)num2);
    char snum1[20], snum2[20];

    gcvt(num1, 6 + d1, snum1);
    gcvt(num2, 6 + d2, snum2);

    i = 0;
    while (snum1[i] != '\0')
    {
        message[ind] = snum1[i];
        ind++;
        i++;
    }
    message[ind] = ' ';
    ind++;

    i = 0;
    while (snum2[i] != '\0')
    {
        message[ind] = snum2[i];
        ind++;
        i++;
    }
    message[ind++] = ' ';

    message[ind++] = '0' + op_type;
    message[ind] = '\0';
}

int main(void)
{
    int socket_desc;
    struct sockaddr_in server_addr;

    float num1, num2;

    char client_name[100];
    int op_type;

    char server_message[MAX], client_message[MAX];

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0)
    {
        printf("Unable to create socket\n");
        return -1;
    }

    printf("Socket created successfully\n");

    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Send connection request to server:
    if (connect(socket_desc, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");

    // Get input from the user:
    printf("\nEnter your name: ");
    scanf("%s", client_name);

    printf("Enter operators: ");
    scanf("%f %f", &num1, &num2);
    getchar();
    printf("\nEnter operation type (1 for addition, 2 for subtraction, 3 for nultiplication and 4 for division): ");
    scanf("%d", &op_type);

    createClientMessage(client_message, client_name, num1, num2, op_type);

    printf("\n%s\n", client_message);

    // Send the message to server:
    if (send(socket_desc, client_message, strlen(client_message), 0) < 0)
    {
        printf("Unable to send message\n");
        return -1;
    }

    // Receive the server's response:
    if (recv(socket_desc, server_message, sizeof(server_message), 0) < 0)
    {
        printf("Error while receiving server's msg\n");
        return -1;
    }

    printf("Server's response: %s\n", server_message);

    // Close the socket:
    close(socket_desc);

    return 0;
}