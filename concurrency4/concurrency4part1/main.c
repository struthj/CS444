#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_PROCESSES 15

//semaphores for blocking and mutex
sem_t processBlock;
sem_t processLock;
//process threads
pthread_t threads[NUM_PROCESSES];
//boolean for full threads
int threadsFull = 0;
int waitingThreads = 0;
int currentThreads = 0;
//process function
void* process(void* ID){
    //cast Id Number of thread
    int processID = *((int*)ID);

    while(1){
        //enter the 3 process mux
        sem_wait(&processLock);
        if(threadsFull == 1){
            //if we have 3 threads signal lock, and wait for others to end
            waitingThreads++;
            sem_post(&processLock);
            sem_wait(&processBlock);
            printf("Process %d: is blocking!\n", processID);
        }else {
            //we have less than 3 threads so increment the current threads
            currentThreads++;
            if(currentThreads == 3){ //if we have 3 threads, its full and unlock
                threadsFull = 1;

            }
            sem_post(&processLock);
        }
        //process "processes"
        printf("Process %d: is working!\n", processID);
        sleep(rand() % 2);
        printf("Process %d: is done!\n", processID);
        sem_wait(&processLock); //get lock
        //decrement current threads
        currentThreads--;
        //if we reach 3 all must leave
        if(currentThreads == 0){ //check if we were the last thread
            int numBlocking = 0;
            if(waitingThreads < 3){
                //threads we must "kill"
                numBlocking = waitingThreads;
            } else{
                //if not less than 3 then delete 3
                numBlocking = 3;
            }
            //"deleted" threads are no longer waiting
            waitingThreads = waitingThreads - numBlocking;
            //threads we want to get rid of
            currentThreads = numBlocking;
            while(numBlocking > 0){
                //get rid of them
                sem_post(&processBlock);
                numBlocking--;
            }
            if(currentThreads == 3){ //if we have 3 threads, its full and unlock
                threadsFull = 1;
            }else {
                threadsFull = 0;
            }
        }
        sem_post(&processLock);
    }
}



int main()
{
     //initialize semaphores and locks
    sem_init(&processBlock, 0 , 0);
    sem_init(&processLock, 0, 1);
    int i = 0;
    for(i = 0; i < NUM_PROCESSES; i++){
        //create customer threads
        pthread_create(&threads[i],NULL, process, &i);
    }

    //join threads
    for(i = 0; i < NUM_PROCESSES; i++){
        //join customer threads
        pthread_join(threads[i], NULL);
    }

    return 0;
}
