#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getSeed(char num[]){
    FILE *fPtr;
    char mainFile[] = "seed_slug_";
    char endFile[] = ".txt";
    strcat(mainFile, num);
    strcat(mainFile, endFile);
    fPtr = fopen(mainFile, "r");
    char str[256];
    int seed;

    // Checks if file exists
    if(fPtr == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    // Reads the seed.txt file and converts content to integer
    while(fgets(str, 256, fPtr)){
        printf("\nRead seed value: %s\n", str);

        seed = atoi(str);
        printf("Read seed value (converted to Integer): %d\n", seed);
    }
    return seed;
}