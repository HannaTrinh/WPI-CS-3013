#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "getSeed.h"
void endProcess(int seed);

int main(int argc, char *argv[]){
    int seed = atoi(argv[1]);

    if( seed == 1 ) {
      //printf("\nThe argument supplied is %d\n", seed);
      srand(getSeed("1"));
      endProcess(getSeed("1"));
    }
    else if( seed == 2 ) {
      //printf("\nThe argument supplied is %d\n", seed);
      srand(getSeed("2"));
      endProcess(getSeed("2"));
    }
    else if( seed == 3) {
      //printf("\nThe argument supplied is %d\n", seed);
      srand(getSeed("3"));
      endProcess(getSeed("3"));
    }
    else if( seed == 4) {
      //printf("\nThe argument supplied is %d\n", seed);
      srand(getSeed("4"));
      endProcess(getSeed("4"));
    } 
    else{
        printf("\nNot a valid Number\n");
    }
}

void endProcess(int seed){
    srand(seed);
    int status_code;
    int ranTime = rand()%4 + 2; // Random time between 2-6 in seconds
    int coinFlip = rand()%2; // Generates a coinflip with 0 & 1
    printf("Delay time is %d seconds. Coin flip: %d\n", ranTime, coinFlip);
    printf("I'll get the job done. Eventually...\n");
    sleep(ranTime);

    char* argument_list[] = {"last", " -i -x", NULL}; // NULL terminated array of char* strings
    char* argument_list2[] = {"id", "--group", NULL}; // NULL terminated array of char* strings
    if(coinFlip == 0){
        printf("break time is over! I am running the 'last -i -x' command.\n");
        status_code = execvp("last", argument_list); // Will execute the command "last -i -x"
        if(status_code == -1){
            printf("ERROR\n");
            exit(1);
        }
    } else{
        printf("break time is over! I am running the 'id --group' command.\n");
        status_code = execvp("id", argument_list2); // Will execute the command "id --group"
        if(status_code == -1){
            printf("ERROR\n");
            exit(1);
        }
    }
}