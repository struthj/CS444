#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "simclist.h"
#include <pthread.h>
#include <semaphore.h>
#define NUM_THREADS 9

//global for linked list using opensource (BSD license) simclist
//reference: http://mij.oltrelinux.com/devel/simclist/
//additional references: https://blog.ksub.org/bytes/2016/04/10/the-search-insert-delete-problem/
list_t l;
//global for locks
pthread_mutex_t deleteLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t insertLock = PTHREAD_MUTEX_INITIALIZER;
//global for threads
pthread_t threads[NUM_THREADS];
//gloabls for semaphores
sem_t deleteInsertSem;
sem_t deleteSearchSem;

//Searches and displays elements in the linked list
void* searcher(void* n){
    //cast Id Number of thread
    int IdNum = *((int*)n);
    //time to wait after execution
    int waitTime = 0;
    //loop continuously
    while(1){
    //wait for delete
    if(sem_trywait(&deleteSearchSem) == 0){
        printf("Searcher Id %d: searching elements!\n", IdNum);
    }else{
        //alert if blocking
        printf("Searcher Id %d: blocking!\n", IdNum);
        sem_wait(&deleteInsertSem);
    }
    //random wait time
    waitTime = rand() % 20;
    //display linked list
    //if list empty don't iterate
    if(list_empty(&l) == 0){
        //list iterator
        list_iterator_start(&l);
        //print nodes until reached null
        printf("Searcher Id %d: Elements are: ", IdNum);
        while (list_iterator_hasnext(&l)) {
            //print value
            printf("%d ", *(int *)list_iterator_next(&l));
        }
        printf("\n");
        list_iterator_stop(&l);
        //finished searching
        printf("Searcher Id %d: done searching!\n", IdNum);
    }else{
        printf("Searcher Id %d: list empty!\n", IdNum);

    }
    //random waittime (helps with visualizing program, otherwise runs too fast)
    sleep(waitTime);
    }
}

//inserts an element into the end of the linked list
void* inserter(void* n){
    //cast Id Number of thread
    int IdNum = *((int*)n);
    //time to wait after execution
    int waitTime = 0;
    //loop continuously
    while(1){
    //generate random wait time
    waitTime = rand() % 20;
    //wait for delete
    if(sem_trywait(&deleteInsertSem) == 0){
        printf("Inserter Id %d: starting!\n", IdNum);
    }else{
        //alert if blocking
        printf("Inserter Id %d: blocking!\n", IdNum);
        sem_wait(&deleteInsertSem);
    }
    //lock insert
    pthread_mutex_lock(&insertLock);
    //generate random element
    int newElement = rand() % 100;
    //insert element at end of list
    printf("Inserter Id %d: inserting element: %d\n", IdNum,newElement );
    //add new element to the end of the list
    list_append(&l, &newElement);
    //unlock insert
    pthread_mutex_unlock(&insertLock);
    printf("Inserter Id %d: done inserting!\n", IdNum);
    //random waittime (helps with visualizing program, otherwise runs too fast)
    sleep(waitTime);
    }
}

//Deletes a random element in-between position 0 and list_size
void* deleter(void* n){
    //cast Id Number of thread
    int IdNum = *((int*)n);
    //time to wait after execution
    int waitTime = 0;
    //loop continuously
    while(1){
    //lock search
    pthread_mutex_lock(&insertLock);
    pthread_mutex_lock(&deleteLock);
    printf("Deleter Id %d: starting! (acquired lock, others blocking) \n", IdNum);
    //generate random wait time
    waitTime = rand() % 50;
    //check if list is not empty
    if(list_empty(&l) == 0){
        int listSize = list_size(&l);
        //delete random element
        int deleteChoice = rand() % listSize;
        //delete element at random position between (0 to list_size)
        void *deletionElement = list_get_at(&l, deleteChoice);
        if(list_delete_at(&l, deleteChoice) == 0){
            //display Id, element and data
            printf("Deleter Id %d: deleting element at: %d, element Data: %d\n", IdNum, deleteChoice, *((int*)deletionElement));
        }else{
            //alert if delete failed
            printf("Deleter Id %d: deleting element failed! %d\n", IdNum, deleteChoice);
        }
    }else{
        printf("Deleter Id %d: list empty!\n", IdNum);

    }
    printf("Deleter Id %d: done deleting!\n", IdNum);
    //unlock search
    pthread_mutex_unlock(&deleteLock);
    pthread_mutex_unlock(&insertLock);
    //Wake up both search and insert threads
    sem_post(&deleteInsertSem);
    sem_post(&deleteSearchSem);
    //random waittime (helps with visualizing program, otherwise runs too fast)
    sleep(waitTime);
    }
}


int main(int argc, char **argv) {

    //Linked list initialization using SimCList
    list_init(&l);
    list_attributes_comparator(&l, list_comparator_int32_t);
    //index vars also used for keeping track of thread Id
    int i = 0;
    int j = 0;
    int k = 0;
    //initialize locks
    pthread_mutex_init(&deleteLock, NULL);
    pthread_mutex_init(&insertLock, NULL);
    //initialize semaphores
    sem_init(&deleteInsertSem, 0, 1);
    sem_init(&deleteSearchSem, 0, 1);

    //create searcher inserter and deleters
    for(i = 0;i < 3;i++)
    {
        //create searchers
        pthread_create(&threads[i], NULL, searcher, &i);
    }
    for(j = 3;j < 6;j++)
    {
        //create inserters
        pthread_create(&threads[j], NULL, inserter, &j);
    }
    for(k = 6;k < 9;k++)
    {
        //create deleters
        pthread_create(&threads[k], NULL, deleter, &k);
    }

    //join  processes
    for(i = 0;i < 3;i++)
    {
        //join searchers
        pthread_join(threads[i], NULL);
    }
    for(j = 3;j < 6;j++)
    {
        //join inserters
        pthread_join(threads[j], NULL);
    }
    for(k = 6;k < 9;k++)
    {
        //join deleters
        pthread_join(threads[k], NULL);
    }

    //list_destroy(&l);
  return(0);
}
