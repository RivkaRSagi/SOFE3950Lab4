#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// define system resources
#define MEMORY 1024
unsigned short int PRINTERS = 2;
unsigned short int SCANNERS = 1;
unsigned short int MODEMS = 1;
unsigned short int CD_DRIVES = 2;
unsigned short int SYS_MEM = 96;
unsigned short int USR_MEM = MEMORY - 96;

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

bool resourcesAvailable(process toRun);
void allocateResources(process toRun);
void deallocateResources(process toRun);
void logUsage();
void printAvailableResources();

int main();


// printProcess print process id, priority, processor time remaining, memory
// location, resources
void printProcess(process proc) {
  printf("arrivalTime:%d priority:%d time remaining:%d Mbytes:%d printers:%d scanners:%d modems:%d CDs: %d",
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

// resourcesAvailable checks if a process's requested resources are available if it
// hasn't been allocated already
bool resourcesAvailable(process toRun) {
  return (toRun.suspended != true && USR_MEM >= toRun.MBytes &&
          PRINTERS >= toRun.numPrinters &&
          SCANNERS >= toRun.numScanners && MODEMS >= toRun.numModems &&
          CD_DRIVES >= toRun.numCDs);
}

// allocateResources allocates the resources for a particular process
void allocateResources(process toRun) {
  USR_MEM -= toRun.MBytes;
  PRINTERS -= toRun.numPrinters;
  SCANNERS -= toRun.numScanners;
  MODEMS -= toRun.numModems;
  CD_DRIVES -= toRun.numCDs;
}

// deallocateResources deallocates the resource for a process
void deallocateResources(process toRun) {
  USR_MEM += toRun.MBytes;
  PRINTERS += toRun.numPrinters;
  SCANNERS += toRun.numScanners;
  MODEMS += toRun.numModems;
  CD_DRIVES += toRun.numCDs;
}

void logUsage() {
    FILE *fp = fopen("usage_log.txt", "a"); // Open file in append mode
    if (fp == NULL) {
        perror("Failed to open log file"); // Print error to stderr if file opening fails
        return;
    }

    fprintf(fp, "MEM_MB: %d/1024   PRINTERS: %d/2   SCANNERS: %d/1   MODEMS: %d/1   CD: %d/2\n",
            USR_MEM, PRINTERS, SCANNERS, MODEMS, CD_DRIVES);

    fclose(fp); // Close the file to free resources
}

void printAvailableResources() {
  printf("Available system resources: \n");
  printf("Memory: %d, Printers: %d, Scanners: %d, Modems: %d, CDs: %d \n",
         USR_MEM, PRINTERS, SCANNERS, MODEMS, CD_DRIVES);
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

    logUsage();
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
      logUsage();

      printf("deallocate:\n");
      deallocateResources(running);
      logUsage();

      printf("RTQ:\n");
      printQueue(RTQ);

    } else if (USER1 != NULL) {
      printf("USER1:\n");
      printQueue(USER1);

      running = pop(&USER1); // pop from queue automatically prints process

      // check if requested resources are available
      if (resourcesAvailable(running)) {
        printf("ALLOCATING...\n");
        // then allocate resources
        allocateResources(running);
        logUsage();

        running.processorTime -= 1; // remove used processor time

        // check if process is finished
        if (running.processorTime > 0) { // if not finished
          printf("send to next queue\n");
          running.priority = 2;
          running.suspended = true;
          printProcess(running);
          push(running, &USER2);
        } else { // if it is finished
          printf("DEALLOCATE...\n");
          deallocateResources(running);
          logUsage();
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

      if (resourcesAvailable(running) || running.suspended == true) {
        printf("shouldrun is true\n");
        (running.suspended == true)
            ? printf("process resources already allocated\n")
            : allocateResources(running);
        logUsage();
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
          logUsage();
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
      if (resourcesAvailable(running) || running.suspended == true) {
        (running.suspended == true)
            ? printf("process resources already allocated\n")
            : allocateResources(running);
        logUsage();
        running.processorTime -= 1;
        if (running.processorTime > 0) {
          printf("resubmitting process to USER3 queue\n");
          printProcess(running);
          push(running, &USER3);

        } else {
          printf("deallocate\n");
          deallocateResources(running);
          logUsage();
        }
      } else {
        printf("shouldrun is false, send back into queue\n");
        printProcess(running);
        push(running, &USER3);
      }
    }
    logUsage();
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

  return 0;
}
