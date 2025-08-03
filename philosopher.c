///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		Adam Moholkar			11/19/2024			philosopher.c
//
//      Purpose: Program is made to run with 5 philosopher processes. If sat around a round table with an
//      equal number of forks as there are philosophers, not all philosophers can eat at once.  This
//      program uses random values for philosophers to think/eat for and allows for each philosopher to
//      eat 3 times before 'leaving the table', and solves the dining philosopher problem without deadlock.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>     // accesses the sleep(int (numSecsSleep)) prototype
#include <stdlib.h>     // srand() and rand() prototypes
#include <stdio.h>      // printf function
#include <time.h>       // used to get the current time when test printing
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#define NUM_PHILS 5     // constant used to keep track of number of philosopher processes

int main(int argc, char *argv[])
{
    int philMutex,      // keeps track of the id of philosopher mutex
        philCtMutex,    // keeps track of the id of the philosopher count mutex
        philNum,        // keeps track of which philosopher process is present
        sleepTime,      // keeps track of how long each process should initially wait (60-90 seconds)
        thinkTime,      // keeps track of how long each process should initially think for
        eatTime,        // keeps track of how long each process will eat for
        numTimesEaten,  // keeps track of how many times each process has eaten for
        leftFork,       // keeps track of if there was an error picking up left fork
        rightFork,      // keeps track of if there was an error picking up right fork
        semRemove,      // keeps track of if there was an error removing semaphores
        semInit,        // keeps track of if there was an error initializing values in semaphore
        forkPickup,     // keeps track of if there was an error 'picking up fork' P()
        forkPutdown,    // keeps track of if there was an error 'putting down a fork' V()
        decPhil,        // keeps track of if there was an error 'decrementing philosopher' P()
        philVal;        // keeps track of the value from philCtMutex semaphore

    long philName = 123,    // arbritrary set long value to be used in philMutex
         philCtName = 234;  // arbritrary set long value to be used in philCtMutex

    time_t currentTime; // used to access the current time - to help set random seed in srand()

    philNum = atoi(argv[1]); // extracted from the second argument in the command
    // sets random seed based on philosopher num and current time
    srand(47 * (time(&currentTime) * philNum) );

    // creates the semaphore consisting of 'forks'
    philMutex = semget(philName, NUM_PHILS, IPC_CREAT | 0600); // returns < 0 on error
    if (philMutex < 0) {
        perror("there was an error creating the semaphore of 5 philosophers");
        exit(-1);
    }
    // creates the semaphore consisting of the current number of philosophers still at table
    philCtMutex = semget(philCtName, 1, IPC_CREAT | 0600);
    if (philMutex < 0) {
        perror("there was an error creating the semaphore of the philosopher count");
        exit(-1);
    }
    // initialize both semaphores when './phil 0 &' is entered
    if (philNum == 0){
        for (int i = 0; i < 5; i++){
            semInit = semctl(philMutex, i, SETVAL, 1); // 'fork' values initially set to 1
            if (semInit < 0){
                perror("error setting values in philMutex");
                exit(-1);
            }
        }
        semInit = semctl(philCtMutex, 0, SETVAL, NUM_PHILS); // number of philosophers is set to 5
        if (semInit < 0){
            perror("error setting value in philCtMutex");
            exit(-1);
        }
    }
    // initializing sembuf variables
    struct sembuf rightForkWait,    // used in philMutex as P() for philosopher's right fork
                  rightForkSignal,  // used in philMutex as V() for philosopher's right fork
                  leftForkWait,     // used in philMutex as P() for philosopher's left fork
                  leftForkSignal,   // used in philMutex as V() for philosopher's left fork
                  countPhil;        // used in philCtMutex as P() to keep track of active philosophers
    rightForkWait.sem_flg = 0;      // sem_flg specifies to block or not for P()
    rightForkWait.sem_op = -1;      // val to be added to the internal sem 'integer counter'
    rightForkSignal.sem_flg = 0;
    rightForkSignal.sem_op = 1;
    if (philNum == 0){
        rightForkWait.sem_num = NUM_PHILS - 1; // sem_num specifies the array position
        rightForkSignal.sem_num = NUM_PHILS - 1;
    } else {
        rightForkWait.sem_num = philNum - 1;
        rightForkSignal.sem_num = philNum - 1;
    }
    leftForkWait.sem_flg = 0;
    leftForkWait.sem_num = philNum;
    leftForkWait.sem_op = -1;
    leftForkSignal.sem_flg = 0;
    leftForkSignal.sem_num = philNum;
    leftForkSignal.sem_op = 1;
    countPhil.sem_flg = 0;
    countPhil.sem_num = 0;
    countPhil.sem_op = -1;

    // Generates a random wait time from 60 to 90 seconds for philosopher to initially wait
    sleepTime = rand() % 31 + 60;
    sleep(sleepTime);
    numTimesEaten = 0;
    while(numTimesEaten < 3) { // philosophers will all eat 3 times each
        // Generates a random think time from 10 to 20 seconds
        thinkTime = rand() % 11 + 10;
        printf("Philosopher %d thinks for %d seconds...\n", philNum, thinkTime);
        fflush(stdout);
        sleep(thinkTime);
        leftFork = 0;   // ensures there was no error picking up leftFork
        rightFork = 0;  // ensures there was no error picking up rightFork

        // tries to pick up left fork
        forkPickup = semop(philMutex, &leftForkWait, 1);
        if (forkPickup < 0) {
            printf("Philosopher %d", philNum);
            perror("error picking up left fork");
            fflush(stdout);
            exit(-1);
        }
        else {
            leftFork = 1;
            printf("Philosopher %d picked up left fork...\n", philNum);
            fflush(stdout);
        }

        //tries to pick up right fork
        forkPickup = semop(philMutex, &rightForkWait, 1);
        if (forkPickup < 0){
            printf("Philosopher %d", philNum);
            perror("error picking up right fork");
            fflush(stdout);
            exit(-1);
        } else {
            rightFork = 1;
            printf("Philosopher %d picked up right fork...\n", philNum);
            fflush(stdout);
        }

        // philosopher eats
        if (leftFork == 1 && rightFork == 1){
            eatTime = rand() % 6 + 5;
            printf("Philosopher %d eats for %d seconds...\n", philNum, eatTime);
            fflush(stdout);
            sleep(eatTime);
            numTimesEaten++;
            // release forks after done eating
            printf("Philosopher %d puts down both forks \n", philNum);
            fflush(stdout);
            forkPutdown = semop(philMutex, &leftForkSignal, 1);
            if (forkPutdown < 0){
                printf("Philosopher %d : ", philNum);
                perror("error releasing left fork semaphore");
                fflush(stdout);
                exit(-1);
            }
            forkPutdown = semop(philMutex, &rightForkSignal, 1);
            if (forkPutdown < 0){
                printf("Philosopher %d : ", philNum);
                perror("error releasing left fork semaphore");
                fflush(stdout);
                exit(-1);
            }
        } // end of eating 'if' statement
    } // end of while loop - after this loop philosopher is done eating
    // decrements the number of philosophers who are still at the 'table'
    decPhil = semop(philCtMutex, &countPhil, 1);
    if (decPhil < 0) {
        perror("there was an error decrementing the philCount semaphore");
        exit(-1);
    }
    // checks to see if this is the last philosopher leaving the 'table'
    philVal = semctl(philCtMutex, 0, GETVAL);
    if (philVal < 0) {  // indicates there was an error getting the value of semaphore
        perror("there was an error getting the value of the philCt semaphore");
        exit(-1);
    } else if (philVal == 0) {  // if there are no philosophers left at table
        printf("Philosopher %d was the last to eat !!!\n", philNum);
        fflush(stdout);
        // remove semaphores
        semRemove = semctl(philMutex, 0, IPC_RMID, NULL);
        if (semRemove < 0){
            perror("Error in removing philMutex semaphore");
            exit(-1);
        }
        semRemove = semctl(philCtMutex, 0, IPC_RMID, NULL);
        if (semRemove < 0){
            perror("Error in removing philCtMutex semaphore");
            exit(-1);
        }
    } else {  // if philosophers are still at the table
        printf("Philosopher %d is done eating !!! \n", philNum);
        fflush(stdout);
    }
    exit(0);
}