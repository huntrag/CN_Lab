/*
Raghav Gade
20CS02003
CN Lab Assignment 2
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#define packet_size 78
#define pay_str 1
#define pay_end 64
#define crc_str 65
#define crc_end 68
#define n 64
#define k 4

int flag = 1;
int iter = 0;
double errProb;
int *payload;

double final_prob[100];

struct data
{
    sem_t sent;
    sem_t received;
    int pack_no;
    int packet[packet_size];
};

typedef struct data data;

void genErrorBit(int *ebit1, int *ebit2)
{
    *ebit1 = rand() % n + pay_str;
    *ebit2 = rand() % n + pay_str;

    while (*ebit1 == *ebit2)
    {
        *ebit2 = rand() % n + pay_str;
    }
}

int isError() // 1 means error and 0 means no error
{
    int errprob = (int)(100 * errProb);

    int check = rand() % 100;

    if (check <= errprob)
    {
        return 1;
    }
    return 0;
}

int checkError(int *packet, int *crc) // if 1 there is error
{
    for (int i = 0; i < k; i++)
    {
        if (packet[crc_str + i] != crc[i])
        {
            return 1;
        }
    }
    return 0;
}

void checksum(int *packet)
{
    for (int i = 0; i < k; i++)
    {
        int ans = 0;
        for (int j = 0; j < n / k; j++)
        {
            ans = ans ^ (packet[pay_str + i + j * k]);
        }
        packet[crc_str + i] = ans;
    }
}

void *sender(void *arg)
{

    data *pack = (data *)arg;
    // ------------Uncomment below
    // printf("Sender packet %d\n", pack->pack_no);
    // printf("Payload: ");
    for (int i = 0; i < n; i++)
    {
        pack->packet[pay_str + i] = payload[i];
        // printf("%d", payload[i]);
    }
    checksum(pack->packet);
    // ------------Uncomment below
    // printf("\nCrc: ");
    // for (int i = 0; i < k; i++)
    // {
    //     printf("%d", pack->packet[crc_str + i]);
    // }
    // printf("\n");
    if (isError() == 1)
    {
        int ebit1, ebit2;
        genErrorBit(&ebit1, &ebit2);
        // printf("Flipped are %d %d\n", ebit1, ebit2);
        pack->packet[ebit1] = pack->packet[ebit1] ^ 1;
        pack->packet[ebit2] = pack->packet[ebit2] ^ 1;
    }
    sem_post(&pack->sent);
}

void *receiver(void *arg)
{
    data *pack = (data *)arg;
    sem_wait(&pack->sent);
    // ------------Uncomment below
    // printf("Receiver packet %d\n", pack->pack_no);
    // printf("Payload: ");
    // for (int i = 0; i < n; i++)
    // {
    //     printf("%d", pack->packet[pay_str + i]);
    // }
    int crc[k];
    for (int i = 0; i < k; i++)
    {
        crc[i] = pack->packet[crc_str + i];
    }
    checksum(pack->packet);
    // ------------Uncomment below
    // printf("\nCrc: ");
    // for (int i = 0; i < k; i++)
    // {
    //     printf("%d", pack->packet[crc_str + i]);
    // }
    // printf("\n");
    if (checkError(pack->packet, crc) == 0)
    {
        flag = 0;
    }
    sem_post(&pack->received);
}
// --------------------------------------------------------------------------------

void generator()
{

    for (int prob = 1; prob <= 100; prob++)
    {
        for (int iteration = 0; iteration < 100; iteration++)
        {
            errProb = (double)prob / 100;
            flag = 1;
            int iter;
            pthread_t thread[2];
            for (int i = 0; i < 100; i++)
            {
                data *packet;
                packet = (data *)malloc(sizeof(data));
                packet->pack_no = i;
                sem_init(&packet->sent, 0, 0);
                sem_init(&packet->received, 0, 0);
                pthread_create(&thread[0], NULL, sender, (void *)packet);
                pthread_create(&thread[1], NULL, receiver, (void *)packet);
                pthread_join(thread[0], NULL);
                pthread_join(thread[1], NULL);
                sem_wait(&packet->received);
                if (flag == 0)
                {
                    iter = i + 1;
                    break;
                }
                sem_destroy(&packet->sent);
                sem_destroy(&packet->received);
            }
            final_prob[prob] += (double)iter;
        }
        final_prob[prob] = (double)final_prob[prob] / 100;
    }
}
// --------------------------------------------------------------------------------
// Note that if you are generating the results for probabilty 0.1 to 1 then please
// uncomment the print statements in sender and reciever functions

// In order to test the code it is advised to uncomment the print statements
// to get the feel of the errors taking place

// --------------------------------------------------------------------------------

int main()
{
    printf("Enter error probability\n");
    scanf("%lf", &errProb);
    srand(time(0));
    payload = (int *)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++)
    {
        payload[i] = rand() % 2;
    }
    flag = 1;
    pthread_t thread[2];
    for (int i = 0; i < 100; i++)
    {
        data *packet;
        packet = (data *)malloc(sizeof(data));
        packet->pack_no = i;
        sem_init(&packet->sent, 0, 0);
        sem_init(&packet->received, 0, 0);
        pthread_create(&thread[0], NULL, sender, (void *)packet);
        pthread_create(&thread[1], NULL, receiver, (void *)packet);
        pthread_join(thread[0], NULL);
        pthread_join(thread[1], NULL);
        sem_wait(&packet->received);
        if (flag == 0)
        {
            iter = i;
            break;
        }
        sem_destroy(&packet->sent);
        sem_destroy(&packet->received);
    }
    printf("Retransmissions took to receive packet successfully %d\n", iter);

    // For generating probabiliies from 0.01 to 1.00
    // 2 decimal precision
    // ----------------------------Uncomment the below code to generate the same
    // generator();
    // for (int i = 1; i <= 100; i++)
    // {
    //     // printf("%lf,", (double)i / 100);
    //     // printf("%lf,", final_prob[i - 1] == 0 ? 1 : final_prob[i - 1]);
    //     printf("%lf %lf\n", (double)i / 100, final_prob[i - 1] == 0 ? 1 : final_prob[i - 1]);
    // }
    return 0;
}
