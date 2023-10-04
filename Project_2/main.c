#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
#include <semaphore.h>

pthread_mutex_t runwayMutex, landingMutex;
pthread_cond_t runwayCond, landingCond; // Condition for each segment
pthread_mutex_t seg1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t seg2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t seg3 = PTHREAD_MUTEX_INITIALIZER;
static int queueFly = 0;
static int queueLand = 0;

// Plane struct
struct Planes
{
    pthread_t p;
    int thread_id;
    int type;  // -----> 0; Small || 1: Large
} planes;

void *idle(int num){
    if(num < 15){
        printf("(Large) Plane Thread %d is idle \n", num);
    } else{
        printf("(Small) Plane Thread %d is idle \n", num);
    }
    usleep((rand()%6+1)*100000); // Sleeps for a random number of time
    return NULL;
}

void* plane(void *arg){
    // Philosphers code
    int num = (int) arg;

    // Initialize all possible outcomes of region in arr of large and small planes
    int sPlane_path[12][3] = {{1,2},{2,1},{1,4},{4,1},{2,3},{3,2},{3,4},{4,3},{3,5},{5,3},{4,6},{6,4}};
    int lPlane_path[4][3] = {{1,4,6},{6,4,1},{2,3,5},{5,3,2}};

    /*char* pSize = "small";
    if(num  < 15){
        pSize = "large";
    }*/

    while(1){
        idle(num);
        int ran;
        // 2. Awaiting Takeoff
        // Randomly selects a path for large plane
        int path[3];
        if(num < 15){
            printf("(Large) Plane Thread %d is awaiting takeoff \n", num);
            ran = rand()%4;
            path[0] = lPlane_path[ran][0];
            path[1] = lPlane_path[ran][1];
            path[2] = lPlane_path[ran][2];
            printf("Taking off at runway: %d %d %d\n", path[0], path[1], path[2]);
        } else{
            printf("(Small) Plane Thread %d is awaiting takeoff \n", num);
            ran = rand()%12;
            path[0] = sPlane_path[ran][0];
            path[1] = sPlane_path[ran][1];
            printf("Taking off at runway: %d %d\n", path[0], path[1]);
        }

        // -----------> Taking Off
        
        pthread_mutex_lock(&runwayMutex); // Lock runway so no other planes can access
        while(queueFly != num){
            pthread_cond_wait(&runwayCond, &runwayMutex);
        }
        if(num < 15){// Large Plane
             // Entering Segment 1
            printf("(Large) Plane Thread %d at (locked) segment %d", num, path[0]);
            usleep((rand()%5+1)*100000);
            printf("(Large) Plane Thread %d at (locked) segment %d", num, path[1]);
            usleep((rand()%5+1)*100000);
            printf("(Large) Plane Thread %d at (locked) segment %d", num, path[2]);
            usleep((rand()%5+1)*100000);
        } else{
            printf("(Large) Plane Thread %d at (locked) segment %d", num, path[0]);
            usleep((rand()%5+1)*100000);
            printf("(Large) Plane Thread %d at (locked) segment %d", num, path[1]);
            usleep((rand()%5+1)*100000);
        }
        queueFly++;
        pthread_cond_broadcast(&runwayCond);
        pthread_mutex_unlock(&runwayMutex);
        
        idle(num); // Waits a random amount of time to simulate flying

        // ------> Landing
        
        int landingPath[3];
        if(num < 15){
            printf("(Large) Plane Thread %d is awaiting takeoff \n", num);
            ran = rand()%4;
            landingPath[0] = lPlane_path[ran][0];
            landingPath[1] = lPlane_path[ran][1];
            landingPath[2] = lPlane_path[ran][2];
            printf("Taking off at runway: %d %d %d\n", landingPath[0], landingPath[1], landingPath[2]);
        } else{
            printf("(Small) Plane Thread %d is awaiting takeoff \n", num);
            ran = rand()%12;
            landingPath[0] = lPlane_path[ran][0];
            landingPath[1] = lPlane_path[ran][1];
            printf("Taking off at runway: %d %d\n", landingPath[0], landingPath[1]);
        }

        // ---------------> Landing 

        pthread_mutex_lock(&landingMutex); // Lock landing runway so no other planes can access
        while(queueLand != num){
            pthread_cond_wait(&landingCond, &landingMutex);
        }
        if(num < 15){// Large Plane
             // Entering Segment 1
            printf("(Large) Plane Thread %d at (locked) segment %d", num, path[0]);
            usleep((rand()%5+1)*100000);
            printf("(Large) Plane Thread %d at (locked) segment %d", num, path[1]);
            usleep((rand()%5+1)*100000);
            printf("(Large) Plane Thread %d at (locked) segment %d", num, path[2]);
            usleep((rand()%5+1)*100000);
        } else{
            printf("(Large) Plane Thread %d at (locked) segment %d", num, path[0]);
            usleep((rand()%5+1)*100000);
            printf("(Large) Plane Thread %d at (locked) segment %d", num, path[1]);
            usleep((rand()%5+1)*100000);
        }
        queueLand++;
        pthread_cond_broadcast(&landingCond);
        pthread_mutex_unlock(&landingMutex);
        
        idle(num); // Waits a random amount of time to simulate flying

    }
}

int main(){
    FILE *fPtr;
    fPtr = fopen("seed.txt", "r");
    char str[256];
    int seed;
    if (fPtr == NULL)
    { // Checks if file exists
        printf("Error opening file\n");
        exit(1);
    }
    while (fgets(str, 256, fPtr))
    { // Reads the seed.txt file and converts content to integer
        printf("Read seed value: ");
        printf("%s\n", str);

        seed = atoi(str);
        printf("\nRead seed value (converted to Integer): ");
        printf("%d\n", seed);
    }
    srand(seed);

    pthread_mutex_init(&runwayMutex, 0);
    pthread_mutex_init(&landingMutex, 0);
    pthread_cond_init(&runwayCond, 0);
    pthread_cond_init(&landingCond, 0);

    pthread_t threads[45]; // Creates 45 threads
    struct Planes planeArr[45];
    for(int i = 0; i < 45; i++){
            planeArr[i].thread_id = i;
            pthread_create(&threads[i], NULL, &plane, (int *)i);
        
    }
    for(int i = 0; i < 45; i++){
            pthread_join(&threads[i], NULL);
        
    }
}