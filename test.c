#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
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
    if (USR_MEM >= toRun.MBytes &&
        PRINTERS >= toRun.numPrinters &&
        SCANNERS >= toRun.numScanners &&
        MODEMS >= toRun.numModems &&
        CD_DRIVES >= toRun.numCDs) {
      
      USR_MEM -= toRun.MBytes;
      PRINTERS -= toRun.numPrinters;
      SCANNERS -= toRun.numScanners;
      MODEMS -= toRun.numModems;
      CD_DRIVES -= toRun.numCDs;
    } else {
      // Handle insufficient resources (e.g., print error or throw exception)
      printf("Error: Not enough resources to allocate.\n");
    }
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

// helper function to parse processes
process parseProcess(char *buffer) {
    process new;
    sscanf(buffer, "%d,%d,%d,%d,%d,%d,%d,%d", 
           &new.arrivalTime, &new.priority, &new.processorTime, &new.MBytes, 
           &new.numPrinters, &new.numScanners, &new.numModems, &new.numCDs);
    new.suspended = false;
    return new;
  }

// Process user queues
// Processes a queue of processes, moving them to the next queue based on resource availability and remaining time
void processUserQueue(fifoQueue **q, fifoQueue **nextQ, int nextPriority) {
    // Check if the current queue is empty; if so, exit the function
    if (*q == NULL) return;
    
    // Display the priority level of the queue being processed
    printf("Processing queue at priority %d:\n", nextPriority - 1);
    
    // Print the contents of the current queue for debugging/viewing
    printQueue(*q);
    
    // Remove and get the first process from the current queue
    process running = pop(q);
  
    // Check if resources are available or if process is already suspended
    if (resourcesAvailable(running) || running.suspended) {
        printf("shouldrun is true\n");
        
        // If process isn't suspended, allocate resources to it
        if (!running.suspended) {
            allocateResources(running);
        } else {
            // If suspended, resources were already allocated previously
            printf("process resources already allocated\n");
        }
        
        // Log the current resource usage
        logUsage();
        
        // Decrease the process's remaining execution time
        running.processorTime -= 1;
        
        // Check if process still needs more execution time
        if (running.processorTime > 0) {
            printf("send to next queue\n");
            // Update priority and mark as suspended for next queue
            running.priority = nextPriority;
            running.suspended = true;
            printProcess(running);
            // Add process to the next priority queue
            push(running, nextQ);
        } else {
            // Process is complete, clean up
            printf("deallocate\n");
            deallocateResources(running);
            // Log final resource usage
            logUsage();
        }
    } else {
        // Resources not available and process not suspended
        printf("shouldrun is false, send to next queue\n");
        // Update priority for next queue
        running.priority = nextPriority;
        printProcess(running);
        // Move process to next queue without running it
        push(running, nextQ);
    }
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
    process new = parseProcess(buffer);

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
        processUserQueue(&USER1, &USER2, 2);
      } else if (USER2 != NULL) {
        processUserQueue(&USER2, &USER3, 3);
      } else if (USER3 != NULL) {
        printf("USER3:\n");
        printQueue(USER3);
        process running = pop(&USER3);
        printProcess(running);
        if (resourcesAvailable(running) || running.suspended) {
          if (!running.suspended) allocateResources(running);
          else printf("process resources already allocated\n");
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
