#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#define NUM_SMOKERS 3

//semaphores for ingredients fr 3 smokers, and lock for smoking
//note that greens means tobacco
sem_t paperAndGreens;
sem_t paperAndLighter;
sem_t lighterAndGreens;
sem_t agent;
sem_t smokingLock;
//threads for smokers
pthread_t smokers[NUM_SMOKERS + 1]; //one more for the agent!

void* agentDealer(void* ID){
    //cast Id Number of thread
    int agentID = *((int*)ID);
    //wait till done smoking
    while(1){
        if(sem_trywait(&smokingLock) == 0){
            printf("Agent Id %d: is waking time to deal!\n", agentID);

        }else{
            //alert if blocking
            printf("Agent Id %d: is Sleepy!\n", agentID);
            sem_wait(&smokingLock);
            printf("Agent Id %d: is waking time to deal!\n", agentID);
        }
        //pick dealing choice
        int dealersChoice = rand() % 3;
        switch(dealersChoice){
            //signal each smoker based on ingredients
            case 0:
                printf("Agent Id %d: placing paper And Greens!\n", agentID);
                sem_post(&lighterAndGreens);
                break;
            case 1:
                printf("Agent Id %d: placing paper And Lighter\n", agentID);
                sem_post(&paperAndGreens);
                break;
            case 2:
                printf("Agent Id %d: placing lighter And Greens\n", agentID);
                sem_post(&paperAndLighter);
                break;
        }
        //done dealing they can smoker
        printf("Agent Id %d: is Sleepy!\n", agentID);
        sem_post(&smokingLock);
        sem_wait(&agent);

    }
}


void smoke(int n){
    printf("Smoker Id %d: is smokin!\n", n);
    sleep(rand() % 2);
    printf("Smoker Id %d: is done!\n", n);
}

void* smoker1(void* ID){
    //cast Id Number of thread
    int smokerID = *((int*)ID);
    while(1){
       if(sem_trywait(&lighterAndGreens) == 0){
            printf("Smoker Id %d: is waking time to deal!\n", smokerID);

        }else{
            //alert if blocking
            printf("Smoker Id %d: is waiting!\n", smokerID);
            sem_wait(&lighterAndGreens);

        }
        sem_wait(&smokingLock);
        printf("SmokerId %d: is ready time to smoke!\n", smokerID);
        //done release lock
        sem_post(&agent);
        sem_post(&smokingLock);
        //smoke!
        smoke(smokerID);

    }

}

void* smoker2(void* ID){
    //cast Id Number of thread
    int smokerID = *((int*)ID);
    while(1){
       if(sem_trywait(&paperAndGreens) == 0){
            printf("Smoker Id %d: is waking time to deal!\n", smokerID);

        }else{
            //alert if blocking
            printf("Smoker Id %d: is waiting!\n", smokerID);
            sem_wait(&paperAndGreens);

        }
        sem_wait(&smokingLock);
        printf("SmokerId %d: is ready time to smoke!\n", smokerID);
        //done release lock
        sem_post(&agent);
        sem_post(&smokingLock);
        //smoke!
        smoke(smokerID);

    }

}

void* smoker3(void* ID){
    //cast Id Number of thread
    int smokerID = *((int*)ID);
    while(1){
       if(sem_trywait(&paperAndLighter) == 0){
            printf("Smoker Id %d: is waking time to deal!\n", smokerID);

        }else{
            //alert if blocking
            printf("Smoker Id %d: is waiting!\n", smokerID);
            sem_wait(&paperAndLighter);

        }
        sem_wait(&smokingLock);
        printf("SmokerId %d: is ready time to smoke!\n", smokerID);
        //done release lock
        sem_post(&agent);
        sem_post(&smokingLock);
        //smoke!
        smoke(smokerID);

    }

}


void main()
{
    //initialize semaphores and locks
    sem_init(&lighterAndGreens, 0 , 0);
    sem_init(&paperAndGreens, 0, 0);
    sem_init(&paperAndLighter, 0, 0);
    sem_init(&smokingLock, 0, 1);

    int i = 0;
    //create smoker threads
    pthread_create(&smokers[i],NULL, smoker1, &i);
    i++;
    pthread_create(&smokers[i],NULL, smoker2, &i);
    i++;
    pthread_create(&smokers[i],NULL, smoker3, &i);
    i++;
    pthread_create(&smokers[i],NULL, agentDealer, &i);
    //this is messy but its late
    //join threads
    for(i = 0; i < NUM_SMOKERS+1; i++){
        //join customer threads
        pthread_join(smokers[i], NULL);
    }


}
