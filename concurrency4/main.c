#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#define NUM_CHAIRS 10

//customer threads
pthread_t threads[NUM_CHAIRS + 1]; //for the barber
//global for waiting time (number of chairs filled)
int waitTime = 0;
//globals for semaphores
sem_t customerSem;
sem_t barberSem;
//mutex for mutual exclusion
pthread_mutex_t barberShopLock = PTHREAD_MUTEX_INITIALIZER;

//helper function barber thread calls when cutting hair
void cut_hair(int ID){
    printf("Barber ID %d: is cutting hair!\n", ID);
    int sleepTime = rand() % 3;
    sleep(sleepTime);
    printf("Barber ID %d: is done!\n", ID);

}

//helper function customer thread calls when getting hair cut
void get_hair_cut(int ID){
    printf("Customer ID %d: is getting hair cut!\n", ID);
    int sleepTime = rand() % 2;
    sleep(sleepTime);
    printf("Customer ID %d: is done, now leaving!\n", ID);
}

//customer thread
void* customer(void* ID){
    //cast Id Number of thread
    int customerID = *((int*)ID);
    while(1){
        pthread_mutex_lock(&barberShopLock);
        if(waitTime < NUM_CHAIRS){
            //there's an open seat so wait then get hair cut
            //increment waiting time
            waitTime++;

            //alert that others ready to be cut
            sem_post(&customerSem);
            pthread_mutex_unlock(&barberShopLock);
            //lock barber
            if(sem_trywait(&barberSem) == 0){
                printf("Customer Id %d: in barber's chair!\n", customerID);

            }else{
                //alert if blocking
                printf("Customer Id %d: blocking!\n", customerID);
                sem_wait(&barberSem);
            }
            //get hair cut
            get_hair_cut(customerID);
        }else {
            //if all chairs are full leave the shop
            pthread_mutex_unlock(&barberShopLock);
            printf("Customer %d: is leaving, All Chairs Full!\n",  customerID);
        }

    }


}

//barber thread
void* barber(void* ID){
    //cast Id Number of thread
    int barberID = *((int*)ID);
    while(1){
        if(sem_trywait(&customerSem) == 0){
            printf("Barber Id %d: is waking time to Cut!\n", barberID);

        }else{
            //alert if blocking
            printf("Barber Id %d: is Sleepy wait for people!\n", barberID);
            sem_wait(&customerSem);
            printf("Barber Id %d: is waking time to Cut!\n", barberID);
        }
        //lock mutex and decrease number of waiting customers
        pthread_mutex_lock(&barberShopLock);
        //decrement waittime if non zero
        if(waitTime > 0){
            waitTime--;
        }else{
            waitTime = 0;
        }
        //alert barbers
        sem_post(&barberSem);
        //unlock mutex
        pthread_mutex_unlock(&barberShopLock);
        //cut hair
        cut_hair(barberID);
    }
}

int main()
{
    //initialize semaphores and locks
    sem_init(&customerSem, 0 , 0);
    sem_init(&barberSem, 0, 0);
    pthread_mutex_init(&barberShopLock,NULL);
    int i = 0;
    for(i = 0; i < NUM_CHAIRS; i++){
        //create customer threads
        pthread_create(&threads[i],NULL, customer, &i);
    }
    //create barber
    pthread_create(&threads[i], NULL, barber, &i);
    //join threads
    for(i = 0; i < NUM_CHAIRS+1; i++){
        //join customer threads
        pthread_join(threads[i], NULL);
    }
    return 0;
}

