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

void *idleLarge(int num, int state){
    int ran = (rand()%6+1)*100000;
    if(state == 0){ // Idle
        printf("(Large) Plane Thread %d is idle\n", num);
        usleep(ran); // Sleeps for a random number of time
        return NULL;
    } if (state == 1){ // Flying
        printf("(Small) Plane Thread %d is flying for %d millisecs\n", num, ran);
        usleep(ran); // Sleeps for a random number of time
        return NULL;
    }
}
 void *idleSmall(int num, int state){
    int ran = (rand()%6+1)*100000;
    if(state == 0){ // Idle
        printf("(Small) Plane Thread %d is idle\n", num);
        usleep(ran); // Sleeps for a random number of time
        return NULL;
    } if (state == 1){ // Flying
        printf("(Small) Plane Thread %d is flying for %d millisecs\n", num, ran);
        usleep(ran); // Sleeps for a random number of time
        return NULL;
    }
}

void* smallPlane(void *arg){
    // Philosphers code
    int num = (int) arg;

    // Initialize all possible outcomes of region in arr of large and small planes
    int sPlane_path[12][3] = {{1,2},{2,1},{1,4},{4,1},{2,3},{3,2},{3,4},{4,3},{3,5},{5,3},{4,6},{6,4}};

    while(1){
        printf("(Small) Plane Thread %d is idle\n", num);
        idleSmall(num, 0);
        int ran;
        // 2. Awaiting Takeoff
        // Randomly selects a path for large plane
        int path[2];

            printf("(Small) Plane Thread %d is awaiting takeoff \n", num);
            ran = rand()%12;
            path[0] = sPlane_path[ran][0];
            path[1] = sPlane_path[ran][1];

        // -----------> Taking Off
        
        pthread_mutex_lock(&runwayMutex); // Lock runway so no other planes can access
        while(queueFly != num){
            pthread_cond_wait(&runwayCond, &runwayMutex);
        }
            printf("(Small) Plane Thread %d taking off at runway: %d %d\n", num, path[0], path[1]);
            int ranTime = (rand()%5+1)*100000;

            //  Entering Segment 1
            printf("(Small) Plane Thread %d at (locked) segment %d for %d milliseconds\n", num, path[0], ranTime);
            usleep(ranTime);

            //  Entering Segment 2
            printf("(Small) Plane Thread %d at (locked) segment %d for %d milliseconds\n", num, path[1], ranTime);
            usleep(ranTime);

        queueFly++;
        pthread_cond_broadcast(&runwayCond);
        pthread_mutex_unlock(&runwayMutex);
        
        printf("(Small) Plane Thread %d is flying\n", num);
        idleSmall(num, 1); // Waits a random amount of time to simulate flying

        // ------> Awaiting Landing
        
        int landingPath[2];
            printf("(Small) Plane Thread %d is awaiting landing\n", num);
            ran = rand()%12;
            landingPath[0] = sPlane_path[ran][0];
            landingPath[1] = sPlane_path[ran][1];

        // ---------------> Landing 

        pthread_mutex_lock(&landingMutex); // Lock landing runway so no other planes can access
        while(queueLand != num){
            printf("(Small) Plane Thread %d Delay landing\n", num);
            pthread_cond_wait(&landingCond, &landingMutex);
        }
            printf("(Small) Plane Thread %d landing at runway: %d %d\n",num, landingPath[0], landingPath[1]);
            int ranTime1 = (rand()%5+1)*100000;

            //  Entering Segment 1
            printf("(Small) Plane Thread %d at (locked) segment %d for %d milliseconds\n", num, path[0], ranTime1);
            usleep(ranTime1);

            //  Entering Segment 2
            printf("(Small) Plane Thread %d at (locked) segment %d for %d milliseconds\n", num, path[1], ranTime1);
            usleep(ranTime1);

        queueLand++;
        pthread_cond_broadcast(&landingCond);
        pthread_mutex_unlock(&landingMutex);
    }
}

void* largePlane(void *arg){
    // Philosphers code
    int num = (int) arg;

    // Initialize all possible outcomes of region in arr of large and small planes
    int lPlane_path[4][3] = {{1,4,6},{6,4,1},{2,3,5},{5,3,2}};

    while(1){
        printf("(Large) Plane Thread %d is idle\n", num);
        idleLarge(num, 0);
        int ran;
        // 2. Awaiting Takeoff
        // Randomly selects a path for large plane
        int path[3];
            printf("(Large) Plane Thread %d is awaiting takeoff \n", num);
            ran = rand()%4;
            path[0] = lPlane_path[ran][0];
            path[1] = lPlane_path[ran][1];
            path[2] = lPlane_path[ran][2];

        // -----------> Taking Off
        
        pthread_mutex_lock(&runwayMutex); // Lock runway so no other planes can access
        while(queueLand != num){
            pthread_cond_wait(&landingCond, &landingMutex);
        }
        printf("Locking runway at segment %d %d %d\n", path[0], path[1], path[2]);

            printf("(Large) Plane Thread %d Taking off at runway: %d %d %d\n",num, path[0], path[1], path[2]);
            int ranNum = (rand()%5+1)*100000;

            //  Entering Segment 1
            printf("(Large) Plane Thread %d at (locked) segment %d for %d milliseconds\n", num, path[0], ranNum);
            usleep(ranNum);

            //  Entering Segment 2
            printf("(Large) Plane Thread %d at (locked) segment %d for %d milliseconds\n", num, path[1], ranNum);
            usleep(ranNum);

            //  Entering Segment 3
            printf("(Large) Plane Thread %d at (locked) segment %d for %d milliseconds\n", num, path[2], ranNum);
            usleep(ranNum);
        
        queueFly++;
        pthread_cond_broadcast(&runwayCond);
        pthread_mutex_unlock(&runwayMutex);
        
        printf("(Large) Plane Thread %d is flying for\n", num);
        idleLarge(num, 1); // Waits a random amount of time to simulate flying

        // ------> Awaiting Landing
        
        int landingPath[3];
            printf("(Large) Plane Thread %d is awaiting landing \n", num);
            ran = rand()%4;
            landingPath[0] = lPlane_path[ran][0];
            landingPath[1] = lPlane_path[ran][1];
            landingPath[2] = lPlane_path[ran][2];
        

        // ---------------> Landing 

        pthread_mutex_lock(&landingMutex); // Lock landing runway so no other planes can access
        while(queueLand != num){
            pthread_cond_wait(&landingCond, &landingMutex);
        }
            printf("(Large) Plane Thread %d landing off at runway: %d %d %d\n", num, landingPath[0], landingPath[1], landingPath[2]);
            int ranNum1 = (rand()%5+1)*100000;

            //  Entering Segment 1
            printf("(Large) Plane Thread %d at (locked) segment %d for %d milliseconds\n", num, path[0], ranNum1);
            usleep(ranNum1);
            
            //  Entering Segment 2
            printf("(Large) Plane Thread %d at (locked) segment %d for %d milliseconds\n", num, path[1], ranNum1);
            usleep(ranNum1);

            //  Entering Segment 3
            printf("(Large) Plane Thread %d at (locked) segment %d for %d milliseconds\n", num, path[2], ranNum1);
            usleep(ranNum1);
        
        queueLand++;
        pthread_cond_broadcast(&landingCond);
        pthread_mutex_unlock(&landingMutex);

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

    pthread_mutex_init(&runwayMutex, NULL);
    pthread_mutex_init(&landingMutex, NULL);
    pthread_cond_init(&runwayCond, NULL);
    pthread_cond_init(&landingCond, NULL);

    pthread_t threads[45]; // Creates 45 threads
    struct Planes planeArr[45];
    for(int i = 0; i < 45; i++){
        if(i < 15){
            //planeArr[i].thread_id = i;
            pthread_create(&threads[i], NULL, &largePlane, (void *)i);
        } else {
            pthread_create(&threads[i], NULL, &smallPlane, (void *)i);
        }
    }
    for(int i = 0; i < 45; i++){
            pthread_join(threads[i], NULL);
    }
}