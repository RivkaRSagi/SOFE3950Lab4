/*
Iqra Zahid - 100824901
Rhea Mathias - 100825543
Rivka Sagi - 100780926 
Julian Olano Medina - 100855732 
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

// Define system resources
#define MEMORY 1024
unsigned short int PRINTERS = 2;
unsigned short int SCANNERS = 1;
unsigned short int MODEMS = 1;
unsigned short int CD_DRIVES = 2;
unsigned short int SYS_MEM = 96;
unsigned short int USR_MEM = MEMORY - 96;

// Struct datatype to define a process
typedef struct process {
    bool suspended;
    int arrivalTime;
    int priority;
    int processorTime; // in seconds
    int MBytes;
    int numPrinters;
    int numScanners;
    int numModems;
    int numCDs;
} process;

// Struct datatype to define FIFO Queues
typedef struct queue {
    process proc;
    struct queue *next;
} fifoQueue;

// Dispatcher Queues (0 - 3)
fifoQueue *RTQ = NULL;
fifoQueue *USER1 = NULL;
fifoQueue *USER2 = NULL;
fifoQueue *USER3 = NULL;

// Function declarations
void push(process newProc, fifoQueue **headNode);
process pop(fifoQueue **headNode);
void printProcess(process proc);
bool resourcesAvailable(process toRun);
void allocateResources(process toRun);
void deallocateResources(process toRun);
void logUsage();
void printAvailableResources();
void printQueue(fifoQueue *headNode);
process parseProcess(char *buffer);
void processUserQueue(fifoQueue **q, fifoQueue **nextQ, int nextPriority);
void processAllQueues();

// Print process details
void printProcess(process proc) {
    printf("arrivalTime:%d priority:%d time remaining:%d Mbytes:%d printers:%d scanners:%d modems:%d CDs:%d suspended:%d\n",
           proc.arrivalTime, proc.priority, proc.processorTime, proc.MBytes,
           proc.numPrinters, proc.numScanners, proc.numModems, proc.numCDs, proc.suspended);
}

// Push a new process to end of queue
void push(process newProc, fifoQueue **headNode) {
    fifoQueue *newNode = (fifoQueue *)malloc(sizeof(fifoQueue));
    newNode->proc = newProc;
    newNode->next = NULL;

    if (*headNode == NULL) {
        *headNode = newNode;
    } else {
        fifoQueue *last = *headNode;
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = newNode;
    }
}

// Pop a process from the beginning of the queue
process pop(fifoQueue **headNode) {
    if (*headNode == NULL) {
        process empty = {0}; // Return an initialized empty process
        return empty;
    }

    fifoQueue *temp = (*headNode)->next;
    printProcess((*headNode)->proc);
    process popped = (*headNode)->proc;
    free(*headNode);
    *headNode = temp;
    return popped;
}

// Print queue contents
void printQueue(fifoQueue *headNode) {
    if (headNode == NULL) {
        printf("Queue is empty.\n");
        return;
    }
    fifoQueue *temp = headNode;
    do {
        printProcess(temp->proc);
        temp = temp->next;
    } while (temp != NULL);
}

// Check resource availability
bool resourcesAvailable(process toRun) {
    return (!toRun.suspended && USR_MEM >= toRun.MBytes &&
            PRINTERS >= toRun.numPrinters && SCANNERS >= toRun.numScanners &&
            MODEMS >= toRun.numModems && CD_DRIVES >= toRun.numCDs);
}

// Allocate resources
void allocateResources(process toRun) {
    if (USR_MEM >= toRun.MBytes && PRINTERS >= toRun.numPrinters &&
        SCANNERS >= toRun.numScanners && MODEMS >= toRun.numModems &&
        CD_DRIVES >= toRun.numCDs) {
        USR_MEM -= toRun.MBytes;
        PRINTERS -= toRun.numPrinters;
        SCANNERS -= toRun.numScanners;
        MODEMS -= toRun.numModems;
        CD_DRIVES -= toRun.numCDs;
    } else {
        printf("Error: Not enough resources to allocate.\n");
    }
}

// Deallocate resources
void deallocateResources(process toRun) {
    USR_MEM += toRun.MBytes;
    PRINTERS += toRun.numPrinters;
    SCANNERS += toRun.numScanners;
    MODEMS += toRun.numModems;
    CD_DRIVES += toRun.numCDs;
}

// Log resource usage
void logUsage() {
    FILE *fp = fopen("usage_log.txt", "a");
    if (fp == NULL) {
        perror("Failed to open log file");
        return;
    }
    fprintf(fp, "MEM_MB: %d/1024 PRINTERS: %d/2 SCANNERS: %d/1 MODEMS: %d/1 CD: %d/2\n",
            USR_MEM, PRINTERS, SCANNERS, MODEMS, CD_DRIVES);
    fclose(fp);
}

// Print available resources
void printAvailableResources() {
    printf("Available system resources:\n");
    printf("Memory: %d, Printers: %d, Scanners: %d, Modems: %d, CDs: %d\n",
           USR_MEM, PRINTERS, SCANNERS, MODEMS, CD_DRIVES);
}

// Parse process from buffer
process parseProcess(char *buffer) {
    process new = {0}; // Initialize to avoid garbage values
    sscanf(buffer, "%d,%d,%d,%d,%d,%d,%d,%d",
           &new.arrivalTime, &new.priority, &new.processorTime, &new.MBytes,
           &new.numPrinters, &new.numScanners, &new.numModems, &new.numCDs);
    new.suspended = false;
    return new;
}

// Process user queues
void processUserQueue(fifoQueue **q, fifoQueue **nextQ, int nextPriority) {
    if (*q == NULL) return;

    printf("Processing queue at priority %d:\n", nextPriority - 1);
    printQueue(*q);

    process running = pop(q);
    if (resourcesAvailable(running) || running.suspended) {
        printf("shouldrun is true\n");
        if (!running.suspended) {
            allocateResources(running);
            running.suspended = true;
        } else {
            printf("process resources already allocated\n");
        }
        logUsage();
        running.processorTime -= 1;
        printf("User queue process time remaining: %d\n", running.processorTime);

        if (running.processorTime > 0) {
            printf("send to next queue\n");
            running.priority = nextPriority;
            printProcess(running);
            push(running, nextQ);
        } else {
            printf("deallocate\n");
            deallocateResources(running);
            logUsage();
        }
    } else {
        printf("shouldrun is false, send to next queue\n");
        running.priority = nextPriority;
        printProcess(running);
        push(running, nextQ);
    }
}

// Process all queues with 1-second quantum
void processAllQueues() {
    while (RTQ != NULL || USER1 != NULL || USER2 != NULL || USER3 != NULL) {
        printf("\nProcessing quantum...\n");

        if (RTQ != NULL) {
            process running = pop(&RTQ);
            if (resourcesAvailable(running) || running.suspended) {
                if (!running.suspended) {
                    allocateResources(running);
                    running.suspended = true;
                }
                printf("Realtime mode is still active\n");
                logUsage();
                running.processorTime -= 1;
                printf("RT process time remaining: %d\n", running.processorTime);

                if (running.processorTime > 0) {
                    push(running, &RTQ);
                } else {
                    deallocateResources(running);
                    logUsage();
                }
            } else {
                push(running, &RTQ);
            }
        } else if (USER1 != NULL) {
            processUserQueue(&USER1, &USER2, 2);
        } else if (USER2 != NULL) {
            processUserQueue(&USER2, &USER3, 3);
        } else if (USER3 != NULL) {
            process running = pop(&USER3);
            if (resourcesAvailable(running) || running.suspended) {
                if (!running.suspended) {
                    allocateResources(running);
                    running.suspended = true;
                } else {
                    printf("process resources already allocated\n");
                }
                logUsage();
                running.processorTime -= 1;
                printf("USER3 process time remaining: %d\n", running.processorTime);

                if (running.processorTime > 0) {
                    push(running, &USER3);
                } else {
                    deallocateResources(running);
                    logUsage();
                }
            } else {
                push(running, &USER3);
            }
        }

        printAvailableResources();
        printf("RTQ:\n"); printQueue(RTQ);
        printf("USER1:\n"); printQueue(USER1);
        printf("USER2:\n"); printQueue(USER2);
        printf("USER3:\n"); printQueue(USER3);

        sleep(1);
    }
}

// Main function
int main() {
    FILE *fp = fopen("dispatchList.txt", "r");
    if (fp == NULL) {
        perror("Failed to open file");
        return -1;
    }

    char buffer[256];
    printf("Initial resource state:\n");
    printAvailableResources();
    logUsage();

    printf("\nStarting process simulation with 1-second quantum...\n");

    while (fgets(buffer, 256, fp)) {
        process new = parseProcess(buffer);

        switch (new.priority) {
            case 0:
                printf("Real-time process detected\n");
                push(new, &RTQ);
                processAllQueues();
                break;
            case 1:
                push(new, &USER1);
                processAllQueues();
                break;
            case 2:
                push(new, &USER2);
                processAllQueues();
                break;
            case 3:
                push(new, &USER3);
                processAllQueues();
                break;
            default:
                printf("Invalid priority %d, sorting to USER3\n", new.priority);
                new.priority = 3;
                push(new, &USER3);
                processAllQueues();
                break;
        }
    }

    fclose(fp);

    if (RTQ != NULL || USER1 != NULL || USER2 != NULL || USER3 != NULL) {
        printf("\nAll processes read, continuing execution of remaining queues...\n");
        processAllQueues();
    }

    printf("\nFinal resource state:\n");
    printAvailableResources();

    return 0;
}
