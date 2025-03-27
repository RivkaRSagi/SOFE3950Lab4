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

#define MAX_PROCESSES 1000
#define MEMORY_SIZE 1024
#define REAL_TIME_MEMORY 64
#define USER_MEMORY (MEMORY_SIZE - REAL_TIME_MEMORY)
#define TIME_QUANTUM 1

// Resource constraints
#define PRINTERS 2
#define SCANNERS 1
#define MODEMS 1
#define CDS 2

// Process structure
typedef struct Process {
    int arrival_time;
    int priority;
    int processor_time;
    int memory;
    int printers;
    int scanners;
    int modems;
    int cds;
    int pid;
    struct Process *next;
} Process;

// Queue structure
typedef struct Queue {
    Process *front, *rear;
} Queue;

void initializeQueue(Queue *q) {
    q->front = q->rear = NULL;
}

bool isEmpty(Queue *q) {
    return q->front == NULL;
}

void enqueue(Queue *q, Process *p) {
    if (q->rear == NULL) {
        q->front = q->rear = p;
    } else {
        q->rear->next = p;
        q->rear = p;
    }
    p->next = NULL;
}

Process* dequeue(Queue *q) {
    if (isEmpty(q)) return NULL;
    Process *temp = q->front;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    return temp;
}

// Memory and Resource Tracking
int available_memory = USER_MEMORY;
int available_printers = PRINTERS;
int available_scanners = SCANNERS;
int available_modems = MODEMS;
int available_cds = CDS;

void load_dispatch_list(const char *filename, Queue *rt_queue, Queue *user_queue[]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    int arrival_time, priority, processor_time, memory, printers, scanners, modems, cds, pid = 0;
    while (fscanf(file, "%d, %d, %d, %d, %d, %d, %d, %d", &arrival_time, &priority, &processor_time, &memory, &printers, &scanners, &modems, &cds) != EOF) {
        Process *p = (Process*)malloc(sizeof(Process));
        p->arrival_time = arrival_time;
        p->priority = priority;
        p->processor_time = processor_time;
        p->memory = memory;
        p->printers = printers;
        p->scanners = scanners;
        p->modems = modems;
        p->cds = cds;
        p->pid = pid++;
        p->next = NULL;
        
        if (priority == 0) {
            enqueue(rt_queue, p);
        } else {
            enqueue(&user_queue[priority - 1], p);
        }
    }
    fclose(file);
}

void execute_process(Process *p) {
    printf("Executing Process ID: %d | Priority: %d | Time Remaining: %d sec | Memory: %d MB | Printers: %d | Scanners: %d | Modems: %d | CDs: %d\n",
           p->pid, p->priority, p->processor_time, p->memory, p->printers, p->scanners, p->modems, p->cds);
    free(p);
}

void dispatcher(Queue *rt_queue, Queue *user_queue[]) {
    while (!isEmpty(rt_queue) || !isEmpty(&user_queue[0]) || !isEmpty(&user_queue[1]) || !isEmpty(&user_queue[2])) {
        if (!isEmpty(rt_queue)) {
            Process *p = dequeue(rt_queue);
            execute_process(p);
        } else {
            for (int i = 0; i < 3; i++) {
                if (!isEmpty(&user_queue[i])) {
                    Process *p = dequeue(&user_queue[i]);
                    execute_process(p);
                    break;
                }
            }
        }
    }
}

int main() {
    char filename[256];
    printf("Enter the dispatch list file name: ");
    scanf("%s", filename);
    
    Queue rt_queue;
    Queue user_queue[3];
    initializeQueue(&rt_queue);
    for (int i = 0; i < 3; i++) initializeQueue(&user_queue[i]);
    
    load_dispatch_list(filename, &rt_queue, user_queue);
    dispatcher(&rt_queue, user_queue);
    
    return 0;
}
