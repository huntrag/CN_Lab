/*
    Name: Raghav Gade
    Roll Number: 20CS02003
*/

/*
    Start by registering yourself with->
    $REG your_name
    and then server will allocate you thread
    If you waste servers time, you will be removed

    Then you have $LIST function which will display the registered clients on server and their status whether busy or free

    You can connect to other clients forcefully by using ->
    $CONN other_client_ind_no

    You can disconnect from client by "goodbye" or "exit"

    You can exit server by "exit"
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

int connected = 0;
int reg = 0;

void listenAndPrint(int socketFD);
struct sockaddr_in *createIPv4Address(char *ip, int port);
void readConsoleEntriesAndSendToServer(int socketFD);
void startListeningAndPrintMessagesOnNewThread(int socketFD);

// -----------------------------------------------------------------------------------------------

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

int main()
{

    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in *address = createIPv4Address("127.0.0.1", 2000);

    int result = connect(socketFD, address, sizeof(*address));

    if (result == 0)
        printf("Connection was successful\n");

    startListeningAndPrintMessagesOnNewThread(socketFD);

    readConsoleEntriesAndSendToServer(socketFD);

    close(socketFD);

    return 0;
}

void readConsoleEntriesAndSendToServer(int socketFD)
{
    char name[20];
    printf("$REG name ->Register yourself.\n$LIST -> List the clients connected.\n$CONN ind -> Connect with the client.\n");

    char *line = NULL;
    size_t lineSize = 0;

    int cnt = 0;
    while (true)
    {
        char buffer[1024];
        ssize_t charCount = getline(&line, &lineSize, stdin);
        line[charCount - 1] = 0;

        if (charCount > 0)
        {
            if (strcmp(line, "exit") == 0 || cnt >= 2)
            {
                ssize_t amountWasSent = send(socketFD,
                                             "$EXIT",
                                             strlen("$EXIT"), 0);
                break;
            }
            if (line[0] == '$')
            {
                int it = 0;
                while (line[it] != '\0' && line[it] != ' ')
                {
                    it++;
                }
                line[it] = 0;
                char *command = line;
                if (strcmp(command, "$LIST") == 0)
                {
                    ssize_t amountWasSent = send(socketFD,
                                                 command,
                                                 strlen(command), 0);
                    continue;
                }
                if (reg == 0)
                {
                    if (strcmp(command, "$REG") == 0)
                    {
                        sprintf(name, "%s", &(line[it + 1]));
                        sprintf(buffer, "%s:%s", command, name);
                        ssize_t amountWasSent = send(socketFD,
                                                     buffer,
                                                     strlen(buffer), 0);
                        sleep(1);
                        continue;
                    }
                    else
                    {
                        cnt++;
                        printf("Not Registered\n");
                        continue;
                    }
                }
                if (connected == 0)
                {
                    if (strcmp(command, "$CONN") == 0)
                    {
                        sprintf(name, "%s", &(line[it + 1]));
                        sprintf(buffer, "%s:%s", command, name);
                        ssize_t amountWasSent = send(socketFD,
                                                     buffer,
                                                     strlen(buffer), 0);
                        sleep(1);
                    }
                    else
                    {
                        printf("Stop talking to yourself. Connect with somebody\n");
                        continue;
                    }
                }
            }
            else
            {
                if (reg == 0)
                {
                    cnt++;
                    printf("Not Registered\n");
                    continue;
                }
                if (connected == 0)
                {
                    printf("Stop talking to yourself. Connect with somebody\n");
                    continue;
                }
                else if (connected == 1)
                {
                    if (strcmp(line, "goodbye") == 0)
                    {
                        ssize_t amountWasSent = send(socketFD,
                                                     "$DISCONN",
                                                     strlen("$DISCONN"), 0);
                    }
                    else
                    {
                        sprintf(buffer, "%s", line);
                        ssize_t amountWasSent = send(socketFD,
                                                     buffer,
                                                     strlen(buffer), 0);
                    }
                }
            }
        }
    }
}

void startListeningAndPrintMessagesOnNewThread(int socketFD)
{

    pthread_t id;
    pthread_create(&id, NULL, listenAndPrint, socketFD);
}

void listenAndPrint(int socketFD)
{
    while (true)
    {
        char buffer[1024];

        ssize_t amountReceived = recv(socketFD, buffer, 1024, 0);
        buffer[amountReceived] = 0;
        if (amountReceived > 0)
        {
            if (buffer[0] == '$')
            {
                int it = -1;
                while (buffer[++it] != ':')
                    ;
                buffer[it] = 0;
                char *command = buffer;
                char *line = &(buffer[it + 1]);
                if (strcmp(command, "$REG") == 0)
                {
                    printf("%s registered successfully\n", line);
                    reg = 1;
                }
                else if (strcmp(command, "$LIST") == 0)
                {
                    printf("%s", line);
                }
                else if (strcmp(command, "$ERR") == 0)
                {
                    break;
                }
                else if (strcmp(command, "$CONN_ERR") == 0)
                {
                    printf("User already busy or doesnt exist. Try again\n");
                }
                else if (strcmp(command, "$CONN_SUCC") == 0)
                {
                    printf("Connected with user %s\n", line);
                    connected = 1;
                }
                else if (strcmp(command, "$DIS") == 0)
                {
                    printf("Disconnected with user %s\n", line);
                    connected = 0;
                }
                else if (strcmp(command, "$EXIT") == 0)
                {
                    break;
                }
                else
                {
                    printf("%s\n", line);
                }
            }
            else
            {
                buffer[amountReceived] = 0;
                if (connected)
                {
                    printf("%s\n", buffer);
                }
            }
        }
        if (amountReceived == 0)
            break;
    }

    close(socketFD);
}