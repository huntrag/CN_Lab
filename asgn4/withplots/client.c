#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>

#define DATA_START 8
#define CHECK_SUM 8

int DATA_SIZE = 64;
int PACKET_SIZE;

struct sockaddr_in *createIPv4Address(char *ip, int port);
int checksum(int *data);
void index_to_byte(int *arr, int ind);
int byte_to_index(int *arr);
void listenAndPrint(int socketFD);
char *convert_to_char(int *frame, int size);
int *convert_to_int(char *char_frame, int size);

int main()
{
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in *address = createIPv4Address("127.0.0.1", 2000);

    int result = connect(socketFD, address, sizeof(*address));

    if (result == 0)
        printf("Connection was successful\n");

    listenAndPrint(socketFD);

    return 0;
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

int checksum(int *data)
{
    PACKET_SIZE = DATA_SIZE + DATA_START + CHECK_SUM;
    int error = 0;
    int check[CHECK_SUM];
    memset(check, 0, CHECK_SUM);

    // intial
    int window_no = 0;
    int max_windows = (PACKET_SIZE - DATA_START) / CHECK_SUM;
    for (int i = CHECK_SUM - 1; i >= 0; i--)
    {
        check[i] = data[DATA_START + CHECK_SUM * window_no + i];
    }

    window_no++;
    int carry = 0;
    int sum = 0;
    while (window_no < max_windows)
    {
        for (int i = CHECK_SUM - 1; i >= 0; i--)
        {
            sum = (check[i] + carry + data[DATA_START + CHECK_SUM * window_no + i]) % 2;
            carry = (check[i] + carry + data[DATA_START + CHECK_SUM * window_no + i]) / 2;
            check[i] = sum;
        }
        int back = CHECK_SUM - 1;
        while (back >= 0 && carry > 0)
        {
            sum = (check[back] + carry) % 2;
            carry = (check[back] + carry) / 2;
            check[back] = sum;
            back--;
        }
        window_no++;
    }

    for (int i = 0; i < CHECK_SUM; i++)
    {
        if (check[i] == 0)
        {
            error = 1;
            break;
        }
    }

    return error;
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

void listenAndPrint(int socketFD)
{
    PACKET_SIZE = DATA_START + DATA_SIZE + CHECK_SUM;
    char initial_message[100];
    size_t len = recv(socketFD, initial_message, 100, 0);
    initial_message[len] = 0;
    printf("%s", initial_message);

    while (true)
    {
        char *char_frame;
        char_frame = malloc(sizeof(char) * PACKET_SIZE);
        size_t size = recv(socketFD, char_frame, PACKET_SIZE, 0);

        int *frame = convert_to_int(char_frame, PACKET_SIZE);
        if (size > 0)
        {
            for (int i = 0; i < PACKET_SIZE; i++)
            {
                printf("%d", frame[i]);
            }
            int ind = byte_to_index(frame);
            int error = checksum(frame);

            printf("\n%d %d\n", ind, error);
            int int_send[8];
            index_to_byte(int_send, ind);

            char *char_send = malloc(sizeof(char) * 10);

            for (int i = 0; i < 8; i++)
            {
                char_send[i] = '0' + int_send[i];
            }
            char_send[8] = '0' + error;
            char_send[9] = 0;
            if (!error)
            {
                send(socketFD, char_send, 10, 0);
            }
        }
        if (size == 0)
        {
            break;
        }
    }
}