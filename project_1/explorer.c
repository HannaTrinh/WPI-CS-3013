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
    char str[256], cwd[256];
    char* selectedPath;
    int seed, ranNum;

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
    
    char *paths[] = {"/home", "/proc", "/proc/sys", "/usr", "/usr/bin", "/bin"};
    srand(seed);
    int ranArr[5];
    pid_t cpid;
    int rc;
    char* command = "ls";
    char* argument_list[] = {"ls", "-tr", NULL}; // NULL terminated array of char* strings

    for(int i = 0; i < 5; i++){
        ranArr[i] = rand()%6;
    }

    for(int j = 0; j < sizeof(ranArr)/sizeof(ranArr[0]); j++){
        if(chdir(paths[ranArr[j]]) == 0){
            if (getcwd(cwd, sizeof(cwd)) == NULL){
                perror("getcwd() error");
            }
            else{
                printf("\nSelection #%d: [SUCCESS] -> Current Directory: %s\n", j+1, cwd);
                rc = fork();
                if(rc < 0){
                    fprintf(stderr, "fork failed\n");
                    exit(1);
                }
                else if(rc == 0){
                    printf("    [Child, PID: %d]: Executing 'ls -tr' command...\n", getpid());
                    int status_code = execvp(command, argument_list); // Will execute the command "ls -tr"
                    if (status_code == -1) {
                        printf("Terminated Incorrectly\n");
                        return 1;
                    }
                } else{
                    printf("[Parent]: I am waiting for PID %d to finish.\n", rc);
                    wait(NULL);
                }
            }   
        }
        else{
            perror("Changed Directory FAILED!");
        }
        
    }
    

    fclose(fPtr);
}
