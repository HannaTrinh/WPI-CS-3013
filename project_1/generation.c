#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int decrLifespan(int lifespan);

int main()
{
    FILE *fPtr;
    fPtr = fopen("seed.txt", "r");
    char str[256];
    int seed, i, lifespan;

    // Checks if file exists
    if(fPtr == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    // Reads the seed.txt file and converts content to integer
    while(fgets(str, 256, fPtr)){
        printf("Read seed value: ");
        printf("%s\n",str);

        seed = atoi(str);
        printf("\nRead seed value (converted to Integer): ");
        printf("%d\n",seed);
    }
    
    srand(seed);
    lifespan = rand()%7+5;
    printf("Random Descendant Count: %d\n", lifespan);
    decrLifespan(lifespan);
    
    fclose(fPtr);
}

// Recursive function that takes a random number and decrements it by 1 each time a child is fork()
int decrLifespan(int lifespan){
    int rc = fork();
    pid_t childPID;
    pid_t parentPID;

    if(lifespan == 0){
        printf("%d",rc);
        exit(0);
    } else if(rc == 0){
        lifespan--;
        wait(NULL);
        printf("[Child, PID: %d]: I was called with descendant count=%d. I'll have %d descendant(s).\n", getpid(), lifespan+1, lifespan);
        decrLifespan(lifespan);
    } else {
        //parentPID = getpid();
        printf("\n[Parent, PID: %d]: I am waiting for PID %d to finish\n", getpid(), rc);
    }
    return 0;
}
