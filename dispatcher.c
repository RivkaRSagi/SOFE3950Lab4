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













// IQRAS CODEE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PROCESSES 100
#define MEMORY_SIZE 1024
#define TIME_QUANTUM 2

// Process Structure
typedef struct Process {
    int id;
    int arrival_time;
    int priority;
    int cpu_time;
    int memory;
    int printers;
    int scanners;
    int modems;
    int cds;
    int remaining_time;
    struct Process* next;
} Process;

// Queue Structure
typedef struct Queue {
    Process* front;
    Process* rear;
} Queue;

// Function to initialize queue
void init_queue(Queue* q) {
    q->front = q->rear = NULL;
}

// Function to enqueue process
void enqueue(Queue* q, Process* p) {
    p->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = p;
    } else {
        q->rear->next = p;
        q->rear = p;
    }
}

// Function to dequeue process
Process* dequeue(Queue* q) {
    if (q->front == NULL) return NULL;
    Process* temp = q->front;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    return temp;
}

// Function to check if queue is empty
bool is_empty(Queue* q) {
    return q->front == NULL;
}

// Memory allocation tracking
int available_memory = MEMORY_SIZE;

// Function to check if enough memory is available
bool allocate_memory(int size) {
    if (available_memory >= size) {
        available_memory -= size;
        return true;
    }
    return false;
}

// Function to release memory
void release_memory(int size) {
    available_memory += size;
}

// Function to read dispatch list
void read_dispatch_list(const char* filename, Queue* submission_queue) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        Process* p = (Process*)malloc(sizeof(Process));
        sscanf(line, "%d,%d,%d,%d,%d,%d,%d,%d,%d", 
               &p->id, &p->arrival_time, &p->priority, &p->cpu_time, &p->memory, 
               &p->printers, &p->scanners, &p->modems, &p->cds);
        p->remaining_time = p->cpu_time;
        enqueue(submission_queue, p);
    }
    fclose(file);
}

// Dispatcher function
void dispatcher(Queue* submission_queue) {
    Queue real_time_queue, user_queue_1, user_queue_2, user_queue_3;
    init_queue(&real_time_queue);
    init_queue(&user_queue_1);
    init_queue(&user_queue_2);
    init_queue(&user_queue_3);
    
    int current_time = 0;
    while (!is_empty(submission_queue) || !is_empty(&real_time_queue) || !is_empty(&user_queue_1) || !is_empty(&user_queue_2) || !is_empty(&user_queue_3)) {
        
        // Load processes into queues
        while (!is_empty(submission_queue) && submission_queue->front->arrival_time <= current_time) {
            Process* p = dequeue(submission_queue);
            if (p->priority == 0) {
                enqueue(&real_time_queue, p);
            } else if (p->priority == 1) {
                enqueue(&user_queue_1, p);
            } else if (p->priority == 2) {
                enqueue(&user_queue_2, p);
            } else {
                enqueue(&user_queue_3, p);
            }
        }
        
        // Execute Real-Time Queue
        if (!is_empty(&real_time_queue)) {
            Process* p = dequeue(&real_time_queue);
            printf("Time %d: Running Real-Time Process %d\n", current_time, p->id);
            current_time += p->cpu_time;
            release_memory(p->memory);
            free(p);
            continue;
        }
        
        // Execute User-Level Queues
        Queue* user_queues[3] = { &user_queue_1, &user_queue_2, &user_queue_3 };
        for (int i = 0; i < 3; i++) {
            if (!is_empty(user_queues[i])) {
                Process* p = dequeue(user_queues[i]);
                int exec_time = (p->remaining_time > TIME_QUANTUM) ? TIME_QUANTUM : p->remaining_time;
                printf("Time %d: Running User Process %d (Priority %d) for %d units\n", current_time, p->id, p->priority, exec_time);
                p->remaining_time -= exec_time;
                current_time += exec_time;
                
                if (p->remaining_time > 0) {
                    if (i < 2) {
                        enqueue(user_queues[i + 1], p); // Lower priority
                    } else {
                        enqueue(&user_queue_3, p); // Round Robin
                    }
                } else {
                    release_memory(p->memory);
                    free(p);
                }
                break;
            }
        }
    }
    printf("All processes completed.\n");
}

int main() {
    Queue submission_queue;
    init_queue(&submission_queue);
    
    // Read processes from file
    read_dispatch_list("dispatch_list.txt", &submission_queue);
    
    // Start dispatcher
    dispatcher(&submission_queue);
    
    return 0;
}

