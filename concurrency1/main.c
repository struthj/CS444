#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "mt19937ar.c"
//32 total items
#define BUFFERSIZE 32

//shared buffer for data
struct buffer itemBuffer;
//index for itemBuffer
int consumerNum = 0;
int producerNum = 0;

//Conditions to signal that an item is ready to consume
//and that an item has been consumed and needs another
pthread_cond_t consumerCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t producerCond = PTHREAD_COND_INITIALIZER;


struct item
{
    int number;
    int wait;
};

struct buffer
{
    struct item items[BUFFERSIZE];
    pthread_mutex_t shareLock;
};

int genRandomNumber(int floor, int ceiling)
{
    int num = 0;
    num =  (int)genrand_int32();
    num = num % (ceiling - floor);
    if(num < floor)
    {
        return floor;
    }
    return num;
}

void printItem(struct item *myItem)
{
    printf("Item number: %d\n", myItem->number);
    printf("Item wait: %d\n", myItem->wait);
}

void *consumer(void *foo)
{
    while(1)
    {
        //item to be consumed
        struct item consumeItem;
        //lock shared buffer
        pthread_mutex_lock(&itemBuffer.shareLock);
        //if a consumer thread arrives while the buffer is empty
        //it blocks until a producer adds a new item.
        while(producerNum == 0)
        {
            pthread_cond_wait(&consumerCond, &itemBuffer.shareLock);
        }
        //get item to consume from buffer
        consumeItem = itemBuffer.items[consumerNum];
        //increase the count of consumed items
        consumerNum++;
        //random waiting period
        sleep(consumeItem.wait);
        //consume item from buffer
        printf("Consuming Item data: %d\n", consumeItem.number);
        //resize if at max buffer size
        if(consumerNum >= BUFFERSIZE)
        {
            consumerNum = 0;
        }
        //ready to consume again
        pthread_cond_signal(&producerCond);
        //unlock shared buffer
        pthread_mutex_unlock(&itemBuffer.shareLock);
        //exit condition for testing/debug
//        if(consumerNum == BUFFERSIZE)
//        {
//            printf("MAX ITEMS!\n");
//            break;
//        }
    }
}

void *producer(void *foo)
{
    while(1)
    {
        //item to be produced
        struct item newItem;
        //data value and wait time using Mersenne Twister
        int value = genRandomNumber(1, 100);
        printf("Value is %d\n", value);
        int wait = genRandomNumber(2,9);
        newItem.number = value;
        newItem.wait = wait;
        printItem(&newItem);
        //lock shared buffer
        pthread_mutex_lock(&itemBuffer.shareLock);
        //block until consumer removes an item
        while(producerNum == 31)
        {
            pthread_cond_wait(&producerCond, &itemBuffer.shareLock);
        }
        //add item to buffer
        itemBuffer.items[producerNum] = newItem;
        producerNum++;
        //resize if at max buffer size
        if(producerNum >= BUFFERSIZE)
        {
            producerNum = 0;
        }
        //ready to consume
        pthread_cond_signal(&consumerCond);
        //shared buffer unlock
        pthread_mutex_unlock(&itemBuffer.shareLock);
    }

}

int main()
{
    //producer and consumer threads
    pthread_t threads[2];
    //index var
    int i = 0;
    //create threads
    pthread_create(&threads[0], NULL, consumer, NULL);
    pthread_create(&threads[1], NULL, producer, NULL);
    //join threads
    for(i = 0; i < 2; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
