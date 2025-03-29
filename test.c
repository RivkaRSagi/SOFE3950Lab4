#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// define system resources
#define MEMORY 1024
int sysPrinters = 2;
int sysScanners = 1;
int sysModems = 1;
int sysCDs = 2;
int availableMem = MEMORY;

// struct datatype to define a process
typedef struct process {
  bool suspended; // keep track of which processes have been started previously
  int arrivalTime;
  int priority;
  int processorTime; // in seconds
  int MBytes;        // memory location/block size
  // resources requested by the process
  int numPrinters;
  int numScanners;
  int numModems;
  int numCDs;
} process;

// struct datatype to define FIFO Queues
typedef struct queue {
  process proc;
  struct queue *next;
} fifoQueue;

// DISPATCHER QUEUES (0 - 3)
fifoQueue *RTQ = NULL;// FIFO RTQ: for all real time (priority 0) processes
fifoQueue *USER1 = NULL;// For first priority (priority 1) user processes
fifoQueue *USER2 = NULL;// For second priority (priority 2) user processes
fifoQueue *USER3 = NULL;// For lowest priority (priority 3)  user processes

/*  Process operations:
    printProcess() - print process id, priority, processor time remaining,
   memory location, resources

    FIFO Operations:
    push() - push a new process to end of queue
    pop() - remove a process from beginning of queue
    printQueue() - print contents in queue
*/
void push(process newProc, fifoQueue **headNode);
process pop(fifoQueue **headNode);
void printProcess(process proc);

bool shouldRun(process toRun);
void allocateResources(process toRun);
void deallocateResources(process toRun);
void printMemory();
void printAvailableResources();

int main();


// printProcess print process id, priority, processor time remaining, memory
// location, resources
void printProcess(process proc) {
  printf("arrivalTime: %d priority: %d time remaining: %d Mbytes: %d printers: "
         "%d scanners: %d modems: %d CDs: %d",
         proc.arrivalTime, proc.priority, proc.processorTime, proc.MBytes,
         proc.numPrinters, proc.numScanners, proc.numModems, proc.numCDs);
  printf(" suspended: %d\n\n", proc.suspended);
}

// push a new process to end of queue
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

// pop a process from the beginning of the queue
process pop(fifoQueue **headNode) {
  fifoQueue *temp = NULL;
  process popped;
  if (*headNode == NULL) {
    return popped;
  }

  temp = (*headNode)->next;
  printProcess((*headNode)->proc);
  popped = (*headNode)->proc;
  free(*headNode);
  *headNode = temp;

  return popped;
}

// printQueue prints the contents of a queue
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

// shouldRun checks if a process's requested resources are available if it
// hasn't been allocated already
bool shouldRun(process toRun) {
  return (toRun.suspended != true && availableMem >= toRun.MBytes &&
          sysPrinters >= toRun.numPrinters &&
          sysScanners >= toRun.numScanners && sysModems >= toRun.numModems &&
          sysCDs >= toRun.numCDs);
}

// allocateResources allocates the resources for a particular process
void allocateResources(process toRun) {
  availableMem -= toRun.MBytes;
  sysPrinters -= toRun.numPrinters;
  sysScanners -= toRun.numScanners;
  sysModems -= toRun.numModems;
  sysCDs -= toRun.numCDs;
}

// deallocateResources deallocates the resource for a process
void deallocateResources(process toRun) {
  availableMem += toRun.MBytes;
  sysPrinters += toRun.numPrinters;
  sysScanners += toRun.numScanners;
  sysModems += toRun.numModems;
  sysCDs += toRun.numCDs;
}

void printMemory() {
  printf("system memory: %d\nprinters: %d\nscanners: %d\nmodems: %d\ncds: %d\n",
         availableMem, sysPrinters, sysScanners, sysModems, sysCDs);
}
void printAvailableResources() {
  printf("Available system resources: \n");
  printf("Memory: %d, Printers: %d, Scanners: %d, Modems: %d, CDs: %d \n",
         availableMem, sysPrinters, sysScanners, sysModems, sysCDs);
}

int main() {
  // step 1: read from job list
  // read a line from file, create process
  char buffer[256], arriveTime[2], prior[1], processTime[10], memory[10],
      prints[10], scans[10], mods[10], cds[10];
  FILE *fp = fopen("dispatchList.txt", "r");
  if (fp == NULL) {
    perror("Failed to open file");
    return -1;
  }

  while (fgets(buffer, 256, fp)) {
    strcpy(arriveTime, strtok(buffer, ","));
    strcpy(prior, strtok(NULL, ","));
    strcpy(processTime, strtok(NULL, ","));
    strcpy(memory, strtok(NULL, ","));
    strcpy(prints, strtok(NULL, ","));
    strcpy(scans, strtok(NULL, ","));
    strcpy(mods, strtok(NULL, ","));
    strcpy(cds, strtok(NULL, ","));

    // create new process struct
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

    // step 2: sort/push() the job into correct queue
    switch (new.priority) {
    case 0:
      printf("this is a runtime process\n");
      push(new, &RTQ);
      break;
    case 1:
      push(new, &USER1);
      break;
    case 2:
      push(new, &USER2);
      break;
    case 3:
      push(new, &USER3);
      break;
    default:
      printf("invalid process priority %d, sorting to the lowest priority "
             "queue.\n",
             new.priority);
      new.priority = 3;
      break;
    }

    printMemory();
    process running;
    // step 3: if priority sorted ==0, run/pop from priority queue
    if (new.priority == 0) {
      printProcess(new);

      // pop from queue also prints the process
      running = pop(&RTQ);
      printf("RTQ:\n");
      printQueue(RTQ);

      allocateResources(running); // allocate memory while processing
      printf("allocate:\n");
      printMemory();

      printf("deallocate:\n");
      deallocateResources(running);
      printMemory();

      printf("RTQ:\n");
      printQueue(RTQ);

    } else if (USER1 != NULL) {
      printf("USER1:\n");
      printQueue(USER1);

      running = pop(&USER1); // pop from queue automatically prints process

      // check if requested resources are available
      if (shouldRun(running)) {
        printf("shouldRun is true\n");

        // then allocate resources
        allocateResources(running);
        printMemory();

        running.processorTime -= 1; // remove used processor time

        // check if process is finished
        if (running.processorTime > 0) { // if not finished
          printf("send to next queue\n");
          running.priority = 2;
          running.suspended = true;
          printProcess(running);
          push(running, &USER2);
        } else { // if it is finished
          printf("deallocate\n");
          deallocateResources(running);
          printMemory();
        }
      } else { // if requested resources are not available
        printf("shouldrun is false, send to next queue\n");
        running.priority = 2;
        printProcess(running);
        push(running, &USER2);
      }

    } else if (USER2 != NULL) {

      printf("USER2:\n");
      printQueue(USER2);

      printf("pop from queue:\n");
      running = pop(&USER2); // pop from queue

      if (shouldRun(running) || running.suspended == true) {
        printf("shouldrun is true\n");
        (running.suspended == true)
            ? printf("process resources already allocated\n")
            : allocateResources(running);
        printMemory();
        running.processorTime -= 1;
        if (running.processorTime > 0) {
          printf("send process to next queue\n");
          running.priority = 3;
          running.suspended = true;
          printProcess(running);
          push(running, &USER3);
        } else {
          printf("deallocate\n");
          deallocateResources(running);
          printMemory();
        }
      } else {
        printf("shouldrun is false, send to next queue\n");
        running.priority = 3;
        printProcess(running);
        push(running, &USER3);
      }
    } else {
      printf("USER3:\n");
      printQueue(USER3);

      running = pop(&USER3);
      printProcess(running);
      // check if requested resources are available or have already been
      // assigned
      if (shouldRun(running) || running.suspended == true) {
        (running.suspended == true)
            ? printf("process resources already allocated\n")
            : allocateResources(running);
        printMemory();
        running.processorTime -= 1;
        if (running.processorTime > 0) {
          printf("resubmitting process to USER3 queue\n");
          printProcess(running);
          push(running, &USER3);

        } else {
          printf("deallocate\n");
          deallocateResources(running);
          printMemory();
        }
      } else {
        printf("shouldrun is false, send back into queue\n");
        printProcess(running);
        push(running, &USER3);
      }
    }
    printMemory();
  }// END OF WHILE LOOP

  fclose(fp);// release file
  printf("RTQ:\n");
  printQueue(RTQ);
  printf("USER1:\n");
  printQueue(USER1);
  printf("USER2:\n");
  printQueue(USER2);
  printf("USER3:\n");
  printQueue(USER3);
  // end of while loop, dispatcher has finished reading processes from file

  // last step: make sure the rest of the queues are empty
  // at this point the priority q has finished, there may be content in one of
  //  the user queues.
  // completely empty USER1 first, then move to USER2, then USER3

  // print the total available system resources before reading any processes,
  // and reprint total avaiable after finishing the code, make sure they're the
  // same

  return 0;
}
