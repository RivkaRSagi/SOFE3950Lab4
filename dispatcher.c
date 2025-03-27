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





//IQRA'S CODEEEEEEEEE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the process structure
struct process {
    int process_id;
    int priority;
    char state[10];   // Process state: "Running", "Ready", "Blocked"
    struct process *next;
};

// Declare a head for the list (initially NULL)
struct process *head = NULL;

// Function to add a process to the list in priority order
void add_process(int process_id, int priority, const char *state) {
    struct process *new_process = malloc(sizeof(struct process));
    new_process->process_id = process_id;
    new_process->priority = priority;
    strcpy(new_process->state, state);
    new_process->next = NULL;

    // Add process to the list in priority order
    if (head == NULL || head->priority > priority) {
        // Insert at the beginning
        new_process->next = head;
        head = new_process;
    } else {
        // Find the correct position to insert the process
        struct process *current = head;
        while (current->next != NULL && current->next->priority <= priority) {
            current = current->next;
        }
        new_process->next = current->next;
        current->next = new_process;
    }
}

// Function to run processes based on priority
void run_process() {
    struct process *current = head;
    while (current != NULL) {
        printf("Running Process ID: %d, Priority: %d, State: %s\n", current->process_id, current->priority, current->state);
        current = current->next;
    }
}

// Main function
int main() {
    // Adding processes
    add_process(1, 2, "Ready");
    add_process(2, 1, "Blocked");
    add_process(3, 3, "Ready");
    add_process(4, 0, "Running");

    // Running processes
    printf("Running Processes in Priority Order:\n");
    run_process();

    return 0;
}




