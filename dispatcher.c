#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

//struct datatype to define a process
typedef struct process{
    int pid;
    int arrivalTime;
    int priority;
    int processorTime; //in seconds
    int MBytes; //memory location/block size
    //resources requested by the process
    int numPrinters;
    int numScanners;
    int numModems;
    int numCDs;
}process;

//struct datatype to define FIFO Queues
typedef struct queue{
    process proc;
    struct queue *next;
}fifoQueue;

/*  Process operations:
    printProcess() - print process id, priority, processor time remaining, memory location, resources

    FIFO Operations:
    push() - push a new process to end of queue
    pop() - remove a process from beginning of queue
    printQueue() - print contents in queue
*/

//printProcess print process id, priority, processor time remaining, memory location, resources
void printProcess(process proc){
    printf("PID: %d\npriority: %d\ntime remaining: %d\nMbytes: %d\nprinters: %d\nscanners: %d\nmodems: %d\nCDs: %d\n\n", 
        proc.pid, proc.priority, proc.processorTime, proc.MBytes, proc.numPrinters, proc.numScanners, proc.numModems, proc.numCDs);
}

//push a new process to end of queue
void push(process newProc, fifoQueue **headNode){
    fifoQueue *newNode = (fifoQueue*)malloc(sizeof(fifoQueue));
    newNode->proc = newProc;
    newNode->next = NULL;

    if(*headNode == NULL){
        *headNode = newNode;
    }
    else{
        fifoQueue *last = *headNode;

        while(last->next != NULL){
            last = last->next;
        }
        last->next = newNode;
    }
}

//pop a process from the beginning of the queue
process pop(fifoQueue **headNode){
    fifoQueue *temp = NULL;
    process popped;
    if(*headNode == NULL){return popped;}

    temp = (*headNode)->next;
    printProcess((*headNode)->proc);
    popped = (*headNode)->proc;
    free(*headNode);
    *headNode = temp;

    return popped;
}

//printQueue prints the contents of a queue
void printQueue(fifoQueue *headNode){
    if(headNode == NULL){
        printf("Queue is empty.\n");
        return;
    }
    fifoQueue *temp = (fifoQueue*)malloc(sizeof(fifoQueue));
    temp = headNode;
    do{
        printProcess(temp->proc);
        temp = temp->next;
    }while(temp != NULL);
}

//FIFO 1 priorityQ: populate with real time (priority 0) processes
fifoQueue *priorityQ = NULL;

//FIFO 2 user1: populate with user highest priority (priority 1) processes
fifoQueue *user1 = NULL;

//FIFO 3 user2: populate with user second highest priority (priority 2) processes
fifoQueue *user2 = NULL;

//FIFO 4 user3: populate with user lowest priority (priority 3) processes
fifoQueue *user3 = NULL;


int main(){
    //step 1: read from job list 
        //read a line from file, create process
    char buffer[256], arriveTime[2], prior[1], processTime[10], memory[10], prints[10], scans[10], mods[10], cds[10];
    FILE *fp = fopen("dispatchList.txt", "r");

    while(fgets(buffer, 256, fp)){
        strcpy(arriveTime, strtok(buffer, ","));
        strcpy(prior, strtok(NULL, ","));
        strcpy(processTime, strtok(NULL, ","));
        strcpy(memory, strtok(NULL, ","));
        strcpy(prints, strtok(NULL, ","));
        strcpy(scans, strtok(NULL, ","));
        strcpy(mods, strtok(NULL, ","));
        strcpy(cds, strtok(NULL, ","));
    
        //create new process struct
        process new;
        new.arrivalTime = atoi(arriveTime);
        new.priority = atoi(prior);
        new.processorTime = atoi(processTime);
        new.MBytes = atoi(memory);
        new.numPrinters = atoi(prints);
        new.numScanners = atoi(scans);
        new.numModems = atoi(mods);
        new.numCDs = atoi(cds);

        //sort process to intended queue
        switch(new.priority){
            case 0: 
                push(new, &priorityQ);
                break;
            case 1: 
                push(new, &user1);
                break;
            case 2:
                push(new, &user2);
                break;
            case 3:
                push(new, &user3);
                break;
            default:
                printf("invalid process priority %d, sorting to the lowest priority queue.\n", new.priority);
                new.priority = 3;
                break;
        }

    }
    printf("printing priorityQ: \n");
    printQueue(priorityQ);
    printf("printing user1 queue: \n");
    printQueue(user1);
    printf("printing user2 queue: \n");
    printQueue(user2);
    printf("printing user3 queue: \n");
    printQueue(user3);

    //step 2: sort/push() the job into correct queue

    //step 3: if priority sorted ==0, run/pop from priority queue

    //step 4: if priority sorted>0, check queues
    
    //step 5: run the highest priority queue for one second


    return 0;
}
