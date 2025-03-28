#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

//define system resources
#define MEMORY 1024
int sysPrinters = 2;
int sysScanners = 1;
int sysModems = 1;
int sysCDs = 2;
int availableMem = MEMORY;

//struct datatype to define a process
typedef struct process{
    bool suspended; //keep track of which processes have been started previously
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
    printf("arrivalTime: %d priority: %d time remaining: %d Mbytes: %d printers: %d scanners: %d modems: %d CDs: %d", 
       proc.arrivalTime, proc.priority, proc.processorTime, proc.MBytes, proc.numPrinters, proc.numScanners, proc.numModems, proc.numCDs);
   printf(" suspended: %d\n\n", proc.suspended);
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

//shouldRun checks if a process's requested resources are available if it hasn't been allocated already
bool shouldRun(process toRun){
    return (toRun.suspended != true && 
        availableMem >= toRun.MBytes && 
        sysPrinters >= toRun.numPrinters && 
        sysScanners >= toRun.numScanners && 
        sysModems >= toRun.numModems && 
        sysCDs >= toRun.numCDs);
}

//allocateResources allocates the resources for a particular process
void allocateResources(process toRun){
    availableMem -= toRun.MBytes;
    sysPrinters -= toRun.numPrinters;
    sysScanners -= toRun.numScanners;
    sysModems -= toRun.numModems;
    sysCDs -= toRun.numCDs;
}

//deallocateResources deallocates the resource for a process
void deallocateResources(process toRun){
    availableMem += toRun.MBytes;
    sysPrinters += toRun.numPrinters;
    sysScanners += toRun.numScanners;
    sysModems += toRun.numModems;
    sysCDs += toRun.numCDs;
}

void printMemory(){
    printf("system memory: %d\nprinters: %d\nscanners: %d\nmodems: %d\ncds: %d\n",availableMem, sysPrinters, sysScanners, sysModems, sysCDs);
}
void printAvailableResources() {
    printf ("Available system resources: \n");
    printf("Memory: %d, Printers: %d, Scanners: %d, Modems: %d, CDs: %d \n",availableMem, sysPrinters, sysScanners, sysModems,sysCDs);
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
        new.suspended = false;

        //step 2: sort/push() the job into correct queue
        switch(new.priority){
            case 0: 
                printf("this is a runtime process\n");
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


        printMemory();
        process running;
        // step 3: if priority sorted ==0, run/pop from priority queue
        if(new.priority == 0){
            printProcess(new);

            //pop from queue also prints the process
            running = pop(&priorityQ);
            printf("priorityQ:\n");
            printQueue(priorityQ);

            allocateResources(running); //allocate memory while processing
            printf("allocate:\n");
            printMemory();

            printf("deallocate:\n");
            deallocateResources(running); 
            printMemory();

            printf("priorityQ:\n");
            printQueue(priorityQ);
            
        }
        else if(user1 != NULL){
            printf("user1:\n");
            printQueue(user1);

            running = pop(&user1);//pop from queue automatically prints process
            
            
            //check if requested resources are available
            if(shouldRun(running)){
                printf("shouldRun is true\n");
                
                //then allocate resources 
                allocateResources(running);
                printMemory();

                running.processorTime -=1;//remove used processor time

                //check if process is finished
                if(running.processorTime >0){//if not finished
                    printf("send to next queue\n");
                    running.priority = 2;
                    running.suspended = true;
                    printProcess(running);
                    push(running, &user2);
                }else{//if it is finished
                    printf("deallocate\n");
                    deallocateResources(running);
                    printMemory();
                }
            }
            else{//if requested resources are not available
                printf("shouldrun is false, send to next queue\n");
                running.priority = 2;
                printProcess(running);
                push(running, &user2);
            }
            
        }
        else if(user2 != NULL){

            printf("user2:\n");
            printQueue(user2);

            printf("pop from queue:\n");
            running = pop(&user2);//pop from queue

            if(shouldRun(running) || running.suspended==true){
                printf("shouldrun is true\n");
                (running.suspended==true)?printf("process resources already allocated\n"):allocateResources(running);   
                printMemory();  
                running.processorTime -= 1;
                if(running.processorTime >0){
                    printf("send process to next queue\n");
                    running.priority = 3;
                    running.suspended = true;
                    printProcess(running);
                    push(running, &user3);
                }else{
                    printf("deallocate\n");
                    deallocateResources(running);
                    printMemory();
                }   
            }
            else{
                printf("shouldrun is false, send to next queue\n");
                running.priority = 3;
                printProcess(running);
                push(running, &user3);
            }
        }else{
            printf("user3:\n");
            printQueue(user3);
            
            running = pop(&user3);
            printProcess(running);
            //check if requested resources are available or have already been assigned
            if(shouldRun(running) || running.suspended == true){
                (running.suspended==true)?printf("process resources already allocated\n"):allocateResources(running);   
                printMemory();
                running.processorTime -= 1;
                if(running.processorTime >0){
                    printf("resubmitting process to user3 queue\n");
                    printProcess(running);
                    push(running, &user3);
                    
                }else{
                    printf("deallocate\n");
                    deallocateResources(running);
                    printMemory();
                   
                }
            }
            else{
                printf("shouldrun is false, send back into queue\n");
                printProcess(running);
                push(running, &user3);
            }

        }
        printMemory();

    }
    printf("priorityQ:\n");
    printQueue(priorityQ);
    printf("user1:\n");
    printQueue(user1);
    printf("user2:\n");
    printQueue(user2);
    printf("user3:\n");
    printQueue(user3);
    //end of while loop, dispatcher has finished reading processes from file
    
    //last step: make sure the rest of the queues are empty
    //at this point the priority q has finished, there may be content in one of
    // the user queues.
    //completely empty user1 first, then move to user2, then user3

    //print the total available system resources before reading any processes, 
    //and reprint total avaiable after finishing the code, make sure they're the same


    return 0;
}
