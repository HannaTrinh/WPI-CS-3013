#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 * a page table entry is that it needs to store the VPN (3 bits),
 * the frame number (2 bits), and the read/write bit (1 bit)
 */

const int PHYS_MEM_SIZE = 64;
const int PAGE_SIZE = 16;
char *input = NULL;

typedef enum
{
    false,
    true
} bool;

struct freeList
{
    int frameAddr;
    int freeSpace; // 0 is free, 1 is not free

    struct LinkedList *next;
};

// Keeps track of PT location
// Each process will have 1 ptR
struct ptRegisters
{
    int ptAddr;             // Page table starting location
    uint8_t presentBit : 1; // Tells you whether the PT is in memory or on disk
    uint8_t validBit : 1;
};

// tells you where that Virtual Address is in physical memory
struct pagetableEntry
{
    uint8_t vpn : 2; // Allocates 2 bits for vpn
    uint8_t frameNum : 2;
    uint8_t rw : 1;
    uint8_t presentBit : 1; // In memory or on disk
    uint8_t validBit : 1;   // If we've already mapped it ---> 1 is valid, 0 is not valid
};

struct pageTable
{
    struct pagetableEntry pteArr[4];
};

/*
    Swap(){
        if( @ RoundRobinIndex (RRI) == Process's PageTable){
            RRI++
        }
        if( @ RRI == PT){
            Move page to disk
        } else if(Page's PT in memory){
            Move page to disk
        } else{
            pageTableSwap() @ RRI
            move page @ RRI to disk
        }
        if(Page's PT in memory){
            map or move page back into memory
        }
    }
*/

int rrSwap = 1;
void swapOut(struct ptRegisters ptRArr[], uint8_t memArr[], int process, int virtualAddr)
{
    int processSwapID = 0;
    int swapOffset = 0;
    if (rrSwap == 4) // If statement to reset roundrobin swap
    {
        rrSwap = 0;
    }
    else
    {
        rrSwap++;
    }

    // Check if rrSwap has a page table
    FILE *ptr;                      // Binary file
    ptr = fopen("disk.bin", "rb+"); // r for read, b for binary
    // Checks every single process's location and see if either pageTable or pageTableEntry matches the rrSwap aka frame number
    while (processSwapID < 4)
    {
        int PTStartAddress = ptRArr[processSwapID].ptAddr;                     // Index of pageTable
        struct pageTable *table = (struct pageTable *)&memArr[PTStartAddress]; // Table is initialized using starting address found in physical memory
        if (PTStartAddress == rrSwap)
          { // Pagetable exists in that frame
            if (ptRArr[processSwapID].presentBit == 1)
            { // Pagetable is on memory

                // Put Page table onto disk
                swapOffset += 16;
                uint8_t ptSwap[16];
                fseek(ptr, swapOffset, SEEK_SET);
                fwrite(ptSwap, sizeof(char), 16, ptr);
                ptRArr[processSwapID].ptAddr = swapOffset;
                ptRArr[processSwapID].presentBit = 0;

                // Remove PT from memory at current frame
                for (int byte = 0; byte < 16; byte++)
                {
                    memArr[byte + PTStartAddress] = 0;
                }
                printf("Swapped frame %i to disk at swap slot %i\n", rrSwap, swapOffset);

                // Swap in mapped thing in place of PT on physical memory
                processSwapID = 4; // Done swapping
            }
            else
            { // Pagetable is on disk
              // Pull from disk
            }
        }
        for (int pteID = 0; pteID < 4; pteID++)
        { // Check each pagetable entry
            if (table->pteArr[pteID].presentBit == 1)
            { // If page is on physical memory
                if (table->pteArr[pteID].frameNum == rrSwap)
                { // If that thing in frame is the page
                    fseek(ptr, swapOffset, SEEK_SET);
                    for (int i = 0; i < PAGE_SIZE; ++i)
                    {
                        fputc(memArr[rrSwap * 16 + i], ptr);
                        memArr[rrSwap * 16 + i] = 0;
                    }
                    table->pteArr[pteID].presentBit = 0;
                    table->pteArr[pteID].frameNum = swapOffset;
                    processSwapID = 4; // Exit while loop
                }
            }
        }
        processSwapID++;
    }
    printf("Swapped frame %i to disk at swap slot %i\n", rrSwap, swapOffset);
}

/*
SwapIn
    Open File
    Find the starting point of page given offset
    -- Move page from disk to Array --
        Use fgetc
        memArray = fgetc

    Update PT:
        Present Bit
        Frame number

*/
void swapIn(int offset, struct ptRegisters ptrArr[], uint8_t memArr[], int virtualAddr, int processID)
{
    swapOut(ptrArr,memArr,processID,virtualAddr);
    int frameToSwap = rrSwap--;
    FILE *fp;                      // Binary file
    fp = fopen("disk.bin", "rb+"); // r for read, b for binary

    fseek(fp, offset, SEEK_SET);
    for (int i = 0; i < PAGE_SIZE; ++i)
    {
        int readVal = fgetc(fp);
        memArr[frameToSwap * 16 + i] = readVal;
    }
    fclose(fp);

    // Update PT
    int ptStartAddress = ptrArr[processID].ptAddr;
    struct pageTable *table = (struct pageTable *)&memArr[ptStartAddress];
    table->pteArr[virtualAddr >> 4].presentBit = 1;
    table->pteArr[virtualAddr >> 4].frameNum = frameToSwap;
}

/*  Map:
    Check if memoryArr already has PT using ptR
        Yes:
            1. Check if page already exists in memArr
                Yes: Change the r/w bit
                No: Check memoryArr if there is a free frame to add virtual page, update freeList
                        go back to PT and add a PTE of the virtual page and fn and rw
        NO: check if there is free space in memArr, create a PT  & put into memoryArr, update ptR, update freeList
            Then check if memoryArr has a free space,
                Yes: add virtual page to free frame and update freeList, add PTE of virtual page
                No: error "virtual page already created" */

void map(uint8_t memArr[], int virtualAddr, struct freeList *head, int rw, int process, struct ptRegisters ptRArr[])
{
    int PTStartAddress = ptRArr[process].ptAddr; // Index of pageTable
    int page = virtualAddr >> 4;                 // Get's the page
    struct freeList *current = head;
    struct pageTable *table = (struct pageTable *)&memArr[PTStartAddress]; // Table is initialized using starting address found in physical memory

    if (ptRArr[process].validBit == 1)
    { // Check if PT exists and if it's on physical memory
        if (ptRArr[process].presentBit == 1)
        {
            // Check if PT is in physical memory
            //printf("Page table found on physical memory for process [%d]\n", process);
            if (table->pteArr[page].validBit == 1)
            {                                     // If page is mapped
                if (table->pteArr[page].rw != rw) // Check if read/write bit is different
                {                                 // Change read/write bit
                    table->pteArr[page].rw = rw;
                    printf("Updating permissions for virtual page %d (frame %d)\n", table->pteArr[page].vpn, table->pteArr[page].frameNum);
                }
                else // Page is already mapped and read/write bit wasn't changed
                {
                    printf("Error Virtual page already mapped in physical frame: %d\n", table->pteArr[page].frameNum);
                }
            }
            else
            { // Page has not been mapped, find a free frame using free list and create a page
                while (current != NULL)
                {
                    if (current->freeSpace == 0)
                    { // There is free space
                        table->pteArr[page].frameNum = current->frameAddr;
                        table->pteArr[page].presentBit = 1;
                        table->pteArr[page].rw = rw;
                        table->pteArr[page].validBit = 1;
                        table->pteArr[page].vpn = page;

                        current->freeSpace = 1; // Space is no longer free
                        printf("Mapped virtual address %d (page %d) into physical frame %d\n", virtualAddr, page, current->frameAddr);
                        break;
                    }
                    current = current->next;
                    if (current == NULL)
                    {
                        printf("Swap()\n");
                        swapOut(ptRArr, memArr, process, virtualAddr);
                    }
                }
            }
        }
        else
        {
            printf("Swap() PT from Disk to a free space on memory");
        }
    }
    else
    { // Create page table if there is free space in physical memory and initialize all PTE.
        while (current->next != NULL)
        {
            if (current->freeSpace == 0)
            { // There is free space
                struct pageTable *pt = (struct pageTable *)&memArr[current->frameAddr << 4];
                for (int i = 0; i < 4; i++)
                {
                    pt->pteArr[i].frameNum = 0;
                    pt->pteArr[i].presentBit = 0;
                    pt->pteArr[i].rw = 0;
                    pt->pteArr[i].validBit = 0;
                    pt->pteArr[i].vpn = 0;
                }
                ptRArr[process].ptAddr = current->frameAddr;
                ptRArr[process].presentBit = 1;
                ptRArr[process].validBit = 1;

                current->freeSpace = 1; // Updating freeList array
                printf("Put page table for PID %d into physical frame %d\n", process, current->frameAddr);
                break; // exit while loop
            }
            current = current->next;
            if (current == NULL)
            {
                printf("Swap()\n");
                swapOut(ptRArr, memArr, process, virtualAddr);
            }
        }
        map(memArr, virtualAddr, head, rw, process, ptRArr); // Calls it again because PageTable is created
    }
}

// Store function
void store(int virtualAddr, int processID, int value, uint8_t memArr[], struct ptRegisters ptRArr[])
{
    int PTStartAddress = ptRArr[processID].ptAddr; // Index of pageTable
    int page = virtualAddr >> 4;
    struct pageTable *table = (struct pageTable *)&memArr[PTStartAddress]; // Table is initialized using starting address found in physical memory
    //int pFN = table->pteArr[page].frameNum;

    if (table->pteArr[page].rw == 1)
    { // Read and write
        if (table->pteArr[page].presentBit == 0)
        {
            int offset = processID * 80 + (virtualAddr / 16) * 16 + 16;
            swapIn(offset, ptRArr, memArr, virtualAddr, processID);
        }
        int pFN = table->pteArr[page].frameNum;
        int physicalAddr = (pFN * 16) + virtualAddr;
        memArr[physicalAddr] = value;
        printf("Stored value %d at virtual address %d (physical address %d)\n", value, virtualAddr, physicalAddr);
    }
    else
    {
        printf("Error: writes are not allowed to this page\n");
    }
}

void load(int virtualAddr, int processID, uint8_t memArr[], struct ptRegisters ptRArr[])
{
    int PTStartAddress = ptRArr[processID].ptAddr; // Index of pageTable
    int page = virtualAddr >> 4;
    struct pageTable *table = (struct pageTable *)&memArr[PTStartAddress]; // Table is initialized using starting address found in physical memory
    //int pFN = table->pteArr[page].frameNum;

    if (table->pteArr[page].presentBit == 0)
    {
        int offset = processID * 80 + (virtualAddr / 16) * 16 + 16;
        swapIn(offset, ptRArr, memArr, virtualAddr, processID);
    }
    int pFN = table->pteArr[page].frameNum;
    int physicalAddr = (pFN * 16) + virtualAddr;
    int value = memArr[physicalAddr];
    printf("The value %d is virtual address %d (physical address %d)\n", value, virtualAddr, physicalAddr);
}

struct freeList *head = NULL; // Creates a list of free physical space -> PhysMemSize/PageSize
int main(int argc, char **argv)
{

    printf("Size Of Page table entry struct: %ld\n", sizeof(struct pagetableEntry));
    int process_id;         // [0-3]
    char *instruction_type; // map, store, load
    int virtual_address;    // [0-63]
    int value;              //[0-255]
    uint8_t memory[PHYS_MEM_SIZE]; // Array of bytes
    struct ptRegisters arr_ptReg[4];
    char o[10], f[10], s[10], t[10];

    struct freeList *previous = NULL;
    int isHead = 1;

    for (int i = 0; i < 4; i++)
    {
        struct freeList *current = NULL;
        current = malloc(sizeof(struct freeList));

        arr_ptReg[i].ptAddr = 0;
        arr_ptReg[i].presentBit = 0; // Initializing Page table registers
        arr_ptReg[i].validBit = 0;

        current->frameAddr = i;
        current->freeSpace = 0;
        if (isHead == 1)
        {
            isHead = 0;
            head = current;
        }
        else
        {
            previous->next = current;
        }
        previous = current;
    }

    

    FILE *disk;
    disk = fopen("disk.bin", "rb+");
    fseek(disk, 0, SEEK_SET);
    for (int i = 0; i < 20 * 16; ++i) // initialize the disk to its max size to be full of 0s
    {
        fputc(0, disk);
    }
    fclose(disk);
    
    while (1)
    {
        printf("Instruction? ");
        scanf("%[^,],%[^,],%[^,],%s", o, s, t, f);

        process_id = atoi(o);
        instruction_type = s;
        virtual_address = atoi(t);
        value = atoi(f);

        if (strcmp("map", instruction_type) == 0)
        {
            if (virtual_address > 63)
            {
                printf("Invalid Virtual Address. Virtual Address must be in range [0-63].\n");
            }
            else
            {
                map(memory, virtual_address, head, value, process_id, arr_ptReg);
            }
        }
        else if (strcmp("store", instruction_type) == 0)
        {
            if (value > 255 || value < 0)
            {
                printf("Invalid value. Value must be in range [0-255]");
            }
            else
                store(virtual_address, process_id, value, memory, arr_ptReg);
        }
        else if (strcmp("load", instruction_type) == 0)
        {
            if (value > 255 || value < 0)
            {
                printf("Invalid value. Value must be in range [0-255]");
            }
            else
                load(virtual_address, process_id, memory, arr_ptReg);
        }
    }
}