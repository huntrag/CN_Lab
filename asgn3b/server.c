/*
    Name: Raghav Gade
    Roll Number: 20CS02003
*/

/*
    Server will always replace the exiting clients so that new ones can take place
*/

#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>

#define MAX 3

struct AcceptedSocket
{
    int ind;
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

char names[MAX + 1][20];
struct AcceptedSocket *acceptedSockets[MAX + 1];
int freeind[MAX + 1];
int acceptedSocketsCount = 0;

struct sockaddr_in *createIPv4Address(char *ip, int port);
struct AcceptedSocket *acceptIncomingConnection(int serverSocketFD);
int freeConnections();
void incomingRequests(int serverSocketFD);
void putOnThread(struct AcceptedSocket *pSocket);
int getMessage(char *buffer);
void receiveAndCheck(void *arg);
void sendReceivedMessageToTheOtherClients(char *buffer, int socketFD);
// ----------------------------------------------------------------------------------
struct sockaddr_in *createIPv4Address(char *ip, int port)
{

    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    if (strlen(ip) == 0)
        address->sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);

    return address;
}

struct AcceptedSocket *acceptIncomingConnection(int serverSocketFD)
{
    struct sockaddr_in clientAddress;
    int clientAddressSize = sizeof(struct sockaddr_in);
    int clientSocketFD = accept(serverSocketFD, &clientAddress, &clientAddressSize);

    struct AcceptedSocket *acceptedSocket = malloc(sizeof(struct AcceptedSocket));
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD > 0;

    if (!acceptedSocket->acceptedSuccessfully)
        acceptedSocket->error = clientSocketFD;

    return acceptedSocket;
}

int freeConnections()
{
    for (int i = 1; i <= MAX; i++)
    {
        if (freeind[i] == 0)
        {
            return i;
        }
    }
    return -1;
}

void incomingRequests(int serverSocketFD)
{
    while (true)
    {
        struct AcceptedSocket *clientSocket = acceptIncomingConnection(serverSocketFD);
        int ind = freeConnections();
        if (ind == -1)
        {
            send(clientSocket->acceptedSocketFD, "$ERR", strlen("$ERR"), 0);
            close(clientSocket->acceptedSocketFD);
            continue;
        }
        acceptedSocketsCount++;
        acceptedSockets[ind] = clientSocket;
        freeind[ind] = -2;
        clientSocket->ind = ind;
        putOnThread(clientSocket);
    }
}

void putOnThread(struct AcceptedSocket *pSocket)
{

    pthread_t id;
    pthread_create(&id, NULL, receiveAndCheck, (void *)pSocket);
}

int getMessage(char *buffer)
{
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        if (buffer[i] == ':')
        {
            buffer[i] = 0;
            return i;
        }
    }
    return -1;
}

void receiveAndCheck(void *arg)
{
    struct AcceptedSocket *psocket = (struct AcceptedSocket *)arg;
    int socketFD = psocket->acceptedSocketFD;
    while (true)
    {
        char buffer[1024];
        char letter[1024];
        ssize_t amountReceived = recv(socketFD, buffer, 1024, 0);
        if (amountReceived > 0)
        {
            buffer[amountReceived] = 0;
            if (buffer[0] == '$') // Commands to server
            {
                int mes_ind = getMessage(buffer);
                char *name = buffer;
                char *message = &(buffer[mes_ind + 1]);
                if (strcmp(name, "$REG") == 0)
                {
                    freeind[psocket->ind] = -1;
                    sprintf(names[psocket->ind], "%s", message);
                    sprintf(buffer, "%s:%s", name, message);
                    send(socketFD, buffer, strlen(buffer), 0);
                }
                else if (strcmp(name, "$LIST") == 0)
                {
                    strcat(buffer, "$LIST:");
                    for (int i = 1; i <= MAX; i++)
                    {
                        // printf("%d - %d\n", i, freeind[i]);
                        char sentence[30];
                        if (freeind[i] == -1)
                        {
                            sprintf(sentence, "%d- %s -> FREE\n", i, names[i]);
                            strcat(buffer, sentence);
                        }
                        else if (freeind[i] > 0)
                        {
                            sprintf(sentence, "%d- %s -> BUSY\n", i, names[i]);
                            strcat(buffer, sentence);
                        }
                    }
                    send(socketFD, buffer, strlen(buffer), 0);
                }
                else if (strcmp(name, "$CONN") == 0)
                {
                    int ind = message[0] - '0';
                    if (freeind[ind] != -1)
                    {
                        send(socketFD, "$CONN_ERR", strlen("$CONN_ERR"), 0);
                    }
                    else
                    {
                        freeind[psocket->ind] = ind;
                        freeind[ind] = psocket->ind;
                        sprintf(buffer, "$CONN_SUCC:%s\n", names[ind]);
                        send(socketFD, buffer, strlen(buffer), 0);
                        sprintf(buffer, "$CONN_SUCC:%s\n", names[psocket->ind]);
                        send(acceptedSockets[ind]->acceptedSocketFD, buffer, strlen(buffer), 0);
                    }
                }
                else if (strcmp(name, "$DISCONN") == 0)
                {
                    int other_ind = freeind[psocket->ind];
                    freeind[other_ind] = -1;
                    freeind[psocket->ind] = -1;
                    sprintf(buffer, "$DIS:%s\n", names[other_ind]);
                    send(socketFD, buffer, strlen(buffer), 0);
                    sprintf(buffer, "$DIS:%s\n", names[psocket->ind]);
                    send(acceptedSockets[other_ind]->acceptedSocketFD, buffer, strlen(buffer), 0);
                }
                else if (strcmp(name, "$EXIT") == 0)
                {
                    int other_ind = freeind[psocket->ind];
                    if (other_ind > 0)
                    {
                        freeind[other_ind] = -1;
                        sprintf(buffer, "$DIS:%s\n", names[psocket->ind]);
                        send(acceptedSockets[other_ind]->acceptedSocketFD, buffer, strlen(buffer), 0);
                    }
                    send(socketFD, "$EXIT", strlen("$EXIT"), 0);
                }
            }
            else if (freeind[psocket->ind] > 0)
            {
                sprintf(letter, "%s : %s", names[psocket->ind], buffer);
                send(acceptedSockets[freeind[psocket->ind]]->acceptedSocketFD, letter, strlen(letter), 0);
            }
        }

        if (amountReceived == 0)
        {
            int ret = 100;
            freeind[psocket->ind] = 0;
            acceptedSocketsCount--;
            pthread_exit(&ret);
        }
    }

    close(socketFD);
}

void sendReceivedMessageToTheOtherClients(char *buffer, int socketFD)
{

    for (int i = 0; i < acceptedSocketsCount; i++)
        if (acceptedSockets[i]->acceptedSocketFD != socketFD)
        {
            send(acceptedSockets[i]->acceptedSocketFD, buffer, strlen(buffer), 0);
        }
}

int main()
{
    for (int i = 1; i <= MAX; i++)
    {
        freeind[i] = 0;
    }
    int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in *serverAddress = createIPv4Address("127.0.0.1", 2000);

    int result = bind(serverSocketFD, (const struct sockaddr_in *)serverAddress, sizeof(*serverAddress));
    if (result == 0)
        printf("Socket was bound successfully\n");

    int listenResult = listen(serverSocketFD, 10);

    incomingRequests(serverSocketFD);

    shutdown(serverSocketFD, SHUT_RDWR);

    return 0;
}