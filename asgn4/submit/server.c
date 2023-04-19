#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#define FILE_SIZE 1024
#define PACKET_SIZE 80
#define DATA_SIZE 64
#define DATA_START 8

#define WINDOW 8
#define GO_BACK 5
// prob will be BEP/100000
#define BEP 500

int DATA_END = DATA_SIZE + DATA_START - 1;
int file[FILE_SIZE];

typedef struct
{
    int socketFD;
    int ind;
} thread_data;

struct AcceptedSocket
{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

void checksum(int *data);
struct sockaddr_in *createIPv4Address(char *ip, int port);
int gen_rand();
void gen_file();
void index_to_byte(int *arr, int ind);
int byte_to_index(int *arr);
struct AcceptedSocket *acceptIncomingConnection(int serverSocketFD);
void incomingRequests(int serverSocketFD);
int isError();
int *gen_data(int ind);
void handle_sending(int ind, int socketFD, int *no_of_transmissions);
void *receiveAndCheck(void *arg);
void print_packet(int ind, int *frame);
char *convert_to_char(int *frame, int size);
int *convert_to_int(char *char_frame, int size);
void clean_flags(int *flags);
int get_min_ind_of_ack(int *flags);
void handle_receive(int socketFD, char *ch, int *flags, int tcp_ind);

int main()
{
    srand(time(NULL));
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

void checksum(int *data)
{
    int check[WINDOW];
    memset(check, 0, WINDOW);

    // intial
    int window_no = 0;
    int max_windows = (DATA_END - DATA_START + 1) / WINDOW;
    for (int i = WINDOW - 1; i >= 0; i--)
    {
        check[i] = data[DATA_START + WINDOW * window_no + i];
    }

    window_no++;
    int carry = 0;
    int sum = 0;
    while (window_no < max_windows)
    {
        for (int i = WINDOW - 1; i >= 0; i--)
        {
            sum = (check[i] + carry + data[DATA_START + WINDOW * window_no + i]) % 2;
            carry = (check[i] + carry + data[DATA_START + WINDOW * window_no + i]) / 2;
            check[i] = sum;
        }
        int back = WINDOW - 1;
        while (back >= 0 && carry > 0)
        {
            sum = (check[back] + carry) % 2;
            carry = (check[back] + carry) / 2;
            check[back] = sum;
            back--;
        }
        window_no++;
    }

    for (int i = 0; i < WINDOW; i++)
    {
        data[DATA_END + 1 + i] = 1 ^ check[i];
    }
}

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

int gen_rand()
{
    return rand() % 2;
}

void gen_file()
{
    for (int i = 0; i < FILE_SIZE; i++)
    {
        file[i] = gen_rand();
    }
}

void index_to_byte(int *arr, int ind)
{
    for (int i = 7; i >= 0; i--)
    {
        arr[i] = ind % 2;
        ind /= 2;
    }
}

int byte_to_index(int *arr)
{
    int ind = 0;
    int power = 1;
    for (int i = 7; i >= 0; i--)
    {
        ind += arr[i] * power;
        power *= 2;
    }

    if (ind == 511)
    {
        return -1;
    }
    return ind;
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

void incomingRequests(int serverSocketFD)
{
    while (true)
    {
        struct AcceptedSocket *clientSocket = acceptIncomingConnection(serverSocketFD);
        pthread_t id;

        pthread_create(&id, NULL, (void *)receiveAndCheck, (void *)clientSocket);
    }
}

int isError()
{
    return rand() % 100000 < BEP ? 1 : 0;
}

int *gen_data(int ind)
{
    int *arr;
    arr = malloc(sizeof(int) * PACKET_SIZE);

    index_to_byte(arr, ind);

    for (int i = DATA_START; i < DATA_END; i++)
    {
        arr[i] = file[ind * (DATA_END - DATA_START + 1) + i];
    }

    checksum(arr);

    for (int i = DATA_START; i < DATA_END; i++)
    {
        if (isError())
        {
            arr[i] = arr[i] ^ 1;
        }
    }

    return arr;
}

char *convert_to_char(int *frame, int size)
{
    char *arr = malloc(sizeof(char) * size);
    for (int i = 0; i < size; i++)
    {
        arr[i] = '0' + frame[i];
    }
    return arr;
}

int *convert_to_int(char *char_frame, int size)
{
    int *arr = malloc(sizeof(int) * size);

    for (int i = 0; i < size; i++)
    {
        arr[i] = char_frame[i] - '0';
    }

    return arr;
}

void print_packet(int ind, int *frame)
{
    printf("%d: ", ind);
    for (int i = 0; i < PACKET_SIZE; i++)
    {
        printf("%d", frame[i]);
    }
    printf("\n");
}

void handle_sending(int ind, int socketFD, int *no_of_transmissions)
{
    (*no_of_transmissions)++;
    int *frame = gen_data(ind);
    char *char_frame = convert_to_char(frame, PACKET_SIZE);
    // print_packet(ind, frame);
    send(socketFD, char_frame, PACKET_SIZE, 0);
}

void clean_flags(int *flags)
{
    for (int i = 0; i < GO_BACK; i++)
    {
        flags[i] = 0;
    }
}

int get_min_ind_of_ack(int *flags)
{
    for (int i = 0; i < GO_BACK; i++)
    {
        if (flags[i] == 0)
        {
            return i;
        }
    }
    return GO_BACK;
}

void handle_receive(int socketFD, char *ch, int *flags, int tcp_ind)
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    setsockopt(socketFD, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
    recv(socketFD, ch, 10, 0);
    int *int_received = convert_to_int(ch, 8);
    int ack_of_ind = byte_to_index(int_received);
    if (flags[ack_of_ind - tcp_ind] == 0)
    {
        flags[ack_of_ind - tcp_ind] = 1;
        // printf("ACK received of : %d\n", ack_of_ind);
    }
}

void *receiveAndCheck(void *arg)
{
    struct AcceptedSocket *psocket = (struct AcceptedSocket *)arg;
    int socketFD = psocket->acceptedSocketFD;
    char *inital_message = "1 for varied file size and packet size.\n2 for varied probability.\n";
    send(socketFD, inital_message, strlen(inital_message), 0);
    char response[2];
    recv(socketFD, response, 2, 0);

    gen_file();
    int tcp_ind = 0;
    int max = FILE_SIZE / (DATA_END - DATA_START + 1);
    int flags[GO_BACK];
    int no_of_transmissions = 0;
    int no_of_window_transmissions = 0;
    int min_ind_of_ack;

    while (tcp_ind < max)
    {
        clean_flags(flags);

        // Send frames
        for (int i = 0; tcp_ind + i < max && i < GO_BACK; i++)
        {
            handle_sending(tcp_ind + i, socketFD, &no_of_transmissions);
        }
        no_of_window_transmissions++;
        char buffer[10];
        for (int i = 0; tcp_ind + i < max && i < GO_BACK; i++)
        {
            handle_receive(socketFD, buffer, flags, tcp_ind);
        }

        min_ind_of_ack = get_min_ind_of_ack(flags);
        if (tcp_ind + min_ind_of_ack < max)
        {
            // printf("Packet %d got errors.. So retransmitting from %d again.\n", tcp_ind + min_ind_of_ack, tcp_ind + min_ind_of_ack);
        }
        tcp_ind += min_ind_of_ack;
    }
    double ep = BEP / 100000.0;
    printf("Total packet transmissions required for %d for error probability %lf packets are %d\n", max, ep, no_of_transmissions);
    printf("Total number of windows transmitted are %d\n", no_of_window_transmissions);
}