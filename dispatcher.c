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
    printf("arrivalTime: %d priority: %d time remaining: %d Mbytes: %d printers: %d scanners: %d modems: %d CDs: %d\n\n", 
       proc.arrivalTime, proc.priority, proc.processorTime, proc.MBytes, proc.numPrinters, proc.numScanners, proc.numModems, proc.numCDs);
   
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
        availableMem > toRun.MBytes && 
        sysPrinters > toRun.numPrinters && 
        sysScanners > toRun.numScanners && 
        sysModems > toRun.numModems && 
        sysCDs > toRun.numCDs);
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
    printf("INSIDE THE DEALLOCATE FUNCTION\n");
    availableMem += toRun.MBytes;
    sysPrinters += toRun.numPrinters;
    sysScanners += toRun.numScanners;
    sysModems += toRun.numModems;
    sysCDs += toRun.numCDs;
}

void printMemory(){
    printf("system memory: %d\nprinters: %d\nscanners: %d\nmodems: %d\ncds: %d\n",availableMem, sysPrinters, sysScanners, sysModems, sysCDs);
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

        //step 2: sort/push() the job into correct queue
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

        printMemory();
        process running;
        //step 3: if priority sorted ==0, run/pop from priority queue
        if(new.priority == 0){
            //print process info
            printf("PRIORITY QUEUE: information on process to be run: \n");
            printProcess(new);

            //pop from queue
            running = pop(&priorityQ);

            allocateResources(running); //allocate memory while processing

            //running process
            printf("running for %d seconds... \n", running.processorTime);
            sleep(running.processorTime);

            //reallocate memory
            printf("process is complete, reallocating memory\n");
            deallocateResources(running); 
        }
        else if(user1 != NULL){

            //print process info
            printf("USER1 QUEUE: information on process to be run: \n");
            running = pop(&user1);//pop from queue
            printProcess(running);
            
            //check if requested resources are available
            if(shouldRun(running)){
                //then allocate resources 
                allocateResources(running);

                //run process
                printf("running for 1 second...\n");
                sleep(1);
                running.processorTime -=1;//remove used processor time

                printf("process has %d seconds of runtime left.\n", running.processorTime);

                //check if process is finished
                if(running.processorTime >0){//if not finished
                    printf("sending process to user2 queue.\n"); 
                    running.priority = 2;
                    running.suspended = true;
                    push(running, &user2);
                }else{//if it is finished
                    printf("process complete, reallocating memory\n");
                    deallocateResources(running);
                }
            }
            else{//if requested resources are not available
                printf("there are not enough resources available to run this process. sending process to user2 queue.\n");
                running.priority = 2;
                push(running, &user2);
            }
            
        }
        else if(user2 != NULL){
            printf("USER2 QUEUE: information on process to be run:\n");
            printProcess(user2->proc);
            running = pop(&user2);
            //check if requested resources are available or have already been assigned
            if(shouldRun(running) || running.suspended==true){
                (running.suspended==true)?allocateResources(running):printf("hi\n");           
                printf("running for 1 second...\n");
                sleep(1);
                running.processorTime -= 1;
                printf("process has %d seconds of runtime left.\n", running.processorTime);
                if(running.processorTime >0){
                    printf("sending process to user3 (lowest priority) queue.\n");
                    running.priority = 3;
                    running.suspended = true;
                    push(running, &user3);
                }else{
                    printf("process complete, reallocating memory\n");
                    deallocateResources(running);
                }   
            }
            else{
                printf("there are not enough resources available to run this process. sending process to user3 queue.\n");
                running.priority = 3;
            }
        }else{
            printf("USER3 QUEUE: information on process to be run: \n");
            printProcess(user3->proc);
            running = pop(&user3);
            //check if requested resources are available or have already been assigned
            if(shouldRun(running) || running.suspended == true){
                (running.suspended==true)?allocateResources(running): printf("hi\n");  
                printf("running for 1 second...\n");
                sleep(1);
                running.processorTime -= 1;
                printf("process has %d seconds of runtime left.\n", running.processorTime);
                if(running.processorTime >0){
                    printf("resubmitting process to user3 queue\n");
                    running.suspended = true;
                    push(running, &user3);
                }else{
                    printf("process complete, reallocating memory\n");//NOTE: code currently does not check that the memory stays a positive number
                    deallocateResources(running);
                }
            }
            else{
                printf("there are not enough resources available to run this process. sending process to user3 queue.\n");
                running.priority = 3;
            }

        }
        printMemory();

    }
    //end of while loop, dispatcher has finished reading processes from file
    
    //last step: make sure the rest of the queues are empty
    //at this point the priority q has finished, there may be content in one of
    // the user queues.
    //completely empty user1 first, then move to user2, then user3

    //print the total available system resources before reading any processes, 
    //and reprint total avaiable after finishing the code, make sure they're the same


    return 0;
}
