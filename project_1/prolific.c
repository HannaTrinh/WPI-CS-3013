#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    FILE *fPtr;
    fPtr = fopen("seed.txt", "r");
    char str[256];
    int seed, i, numChild;

    //variable to store calling function's process id
	pid_t current_id;
	//variable to store parent function's process id
	pid_t p_current_id;

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
    numChild = rand()%6+8;
    int ranArr[numChild];
    printf("\nRandom Child Count: ");
    printf("%d\n", numChild);
    for(i = 0; i < numChild; i++){
        ranArr[i] = rand();
    }
    
    for(int n = 0; n < numChild; n++){
        int rc = fork();
        current_id = getpid();
        pid_t myPID;
       
       if(rc < 0){
            fprintf(stderr, "fork failed\n");
            exit(1);
       }
       else if(rc == 0){ // Child (new process)
            int exitVal = ranArr[n]%50 + 1;
            int waitTime = ranArr[n]%3 + 1;
            myPID = getpid();
            printf("[Child, PID: %d]: I am the child and I will wait %d seconds and exit with code %d \n", myPID, waitTime, exitVal);
            sleep(waitTime);
            printf("[Child, PID: %d]: Now exiting...\n", myPID);
            exit(exitVal);
        } else // Parent 
        {
            int exitCode;
            printf("\n[Parent]: I am waiting for PID: %d to finish\n", rc);
            waitpid(myPID, NULL, WCONTINUED);
            exitCode = exitCode/256;
        } 
        
    }
    

    fclose(fPtr);
}
