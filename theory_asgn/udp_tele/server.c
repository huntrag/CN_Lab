/*
    Name: Raghav Gade
    20cs02003
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 5000
#define MAX_BUFFER_SIZE 1024

struct dir
{
    char *telephone;
    char *name;
    struct dir *next;
};

typedef struct dir dir;

dir *insert_into(dir *cur, char *name, char *tele)
{
    if (cur == NULL)
    {
        cur = (dir *)malloc(sizeof(dir));
        cur->name = (char *)malloc(sizeof(char) * strlen(name));
        cur->telephone = (char *)malloc(sizeof(char) * strlen(tele));
        strcpy(cur->name, name);
        strcpy(cur->telephone, tele);

        cur->next = NULL;
    }
    else
    {
        dir *new_cur;
        new_cur = (dir *)malloc(sizeof(dir));

        new_cur->name = (char *)malloc(sizeof(char) * strlen(name));
        new_cur->telephone = (char *)malloc(sizeof(char) * strlen(tele));
        strcpy(new_cur->name, name);
        strcpy(new_cur->telephone, tele);

        new_cur->next = NULL;

        cur->next = new_cur;
        cur = new_cur;
    }

    return cur;
}

dir *create_db(dir *root)
{
    dir *temp;
    char name[100], tele[100];

    strcpy(name, "Raghav");
    strcpy(tele, "1234567890");
    root = insert_into(root, name, tele);
    temp = root;

    strcpy(name, "Prakash");
    strcpy(tele, "0987654321");
    temp = insert_into(temp, name, tele);

    strcpy(name, "Dinesh");
    strcpy(tele, "1111111111");
    temp = insert_into(temp, name, tele);

    return root;
}

int search_db(dir *root, char *tele, char *response)
{
    dir *cur = root;
    int found = 0;
    while (cur != NULL)
    {
        if (strcmp(cur->telephone, tele) == 0)
        {
            found = 1;
            strcpy(response, cur->name);
            return strlen(cur->name);
        }
        cur = cur->next;
    }

    strcpy(response, "Not Found");
    return strlen(response);
}

int main()
{
    // Creating database
    dir *root = NULL;
    root = create_db(root);

    int server_socket, len;
    struct sockaddr_in server_addr, client_addr;
    char buffer[MAX_BUFFER_SIZE];
    char response[100];

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
        buffer[buf_size] = 0;
        printf("Received telephone: %s\n", buffer);

        int term = search_db(root, buffer, response);
        response[term] = 0;
        printf("Telephone: %s & Name: %s\n", buffer, response);

        if (sendto(server_socket, response, term, 0, (struct sockaddr *)&client_addr, len) < 0)
        {
            perror("Send error");
            exit(EXIT_FAILURE);
        }
    }

    close(server_socket);
    return 0;
}
