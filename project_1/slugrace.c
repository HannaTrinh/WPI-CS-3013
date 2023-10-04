#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "getSeed.h"

int main()
{
    FILE *fPtr;
    fPtr = fopen("seed.txt", "r");
    char str[256];
    int seed, time, status_code;
    int activeSlug = 0;
    pid_t rc;
    int slugPID[4];
    int slugFinished[] = {0,0,0,0};
    char *numbers[] = {"1", "2", "3", "4"};
    struct timespec start, stop, delta;

    // Function found on stackOverflow. Given two 'TIME' parameters, it subtracts the two times.
    // Used later on to find the final time of the each slug
    enum { NS_PER_SECOND = 1000000000 };
    void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
    {
        td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
        td->tv_sec  = t2.tv_sec - t1.tv_sec;
        if (td->tv_sec > 0 && td->tv_nsec < 0)
        {
            td->tv_nsec += NS_PER_SECOND;
            td->tv_sec--;
        }
        else if (td->tv_sec < 0 && td->tv_nsec > 0)
        {
            td->tv_nsec -= NS_PER_SECOND;
            td->tv_sec++;
        }
    }

    // For loop that spawns 4 children with parameters
    for(int i = 0 ; i < 4; i++){
        rc = fork();
        char * const arguments[] = {"./slug", numbers[i], NULL};
        
        if(rc < 0){
            fprintf(stderr, "fork failed\n");
            exit(1);
        } 
        else if(rc == 0){
            status_code = execvp("./slug", arguments); // Will execute the command "last -l"
            
            if(status_code == -1){
                printf("ERROR\n");
                exit(1);
            }
            exit(1);
        } 
        else{
            printf("[Parent]: I forked off child %d.\n", rc);
            slugPID[i] = rc;
            ++activeSlug;
            printf("Active Children: %d\n",activeSlug);
        } 
    }

    float milisec = (float)(start.tv_nsec/1000000);
    float timeStart = (float)(start.tv_sec) + milisec/1000.0;
    clock_gettime(CLOCK_BOOTTIME, &start);
    int br = 1;
    while(slugFinished[0] == 0 || slugFinished[1] == 0 || slugFinished[2] == 0 || slugFinished[3] == 0)  {
                int status;
                int waitReturn;

                for (int i = 0; i < 4; i++){
                    clock_gettime(CLOCK_BOOTTIME, &stop);
                    waitReturn = waitpid(slugPID[i], &status, WNOHANG); // Waits for any slug and returns -1, 0, or >0 as conditions.
                    
                    if(waitReturn != 0){ // Slug finish race
                        activeSlug--;
                        slugFinished[i] = 1;
                        sub_timespec(start, stop, &delta);
                        //printf("Slug %d has crossed the finish line! It took %f seconds!\n", slugPID[i], timeEnd-timeStart);
                        printf("Slug %d has crossed the finish line! It took %d.%.6ld\n", slugPID[i], (int)delta.tv_sec, delta.tv_nsec);
                        //Timefinish - timestart
                    } 
                    else{
                        printf("\n");
                    }
                }
                //printf here or recap of race
                if(activeSlug >= 0){
                    printf("\nThe race is still ongoing: There are still %d racing\n", activeSlug);
                } else{
                    slugFinished[0] = 1;
                    slugFinished[1] = 1;
                    slugFinished[2] = 1;
                    slugFinished[3] = 1;
                }
                usleep(330000);
            }
            clock_gettime(CLOCK_BOOTTIME, &stop);
            sub_timespec(start, stop, &delta);
            printf("[Parent %d]: The race is over! It took %d.%.6ld seconds\n", getpid(), (int)delta.tv_sec, delta.tv_nsec);
    //printf("Print: %d, %d, %d, %d\n", slugPID[0], slugPID[1], slugPID[2], slugPID[3]);
    //while(slugPID[0] != 0 && )
    //stop time
    fclose(fPtr);
}
