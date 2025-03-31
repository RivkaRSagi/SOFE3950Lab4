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
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// Define system resources
#define MEMORY 1024
int sysPrinters = 2;
int sysScanners = 1;
int sysModems = 1;
int sysCDs = 2;
int availableMem = MEMORY;

// Struct to define a process
typedef struct {
  bool suspended;
  int arrivalTime;
  int priority;
  int processorTime;
  int MBytes;
  int numPrinters;
  int numScanners;
  int numModems;
  int numCDs;
} process;

// Struct to define FIFO Queues
typedef struct queue {
  process proc;
  struct queue *next;
} fifoQueue;

// Print a process
void printProcess(process proc) {
  printf("arrivalTime: %d priority: %d time remaining: %d MBytes: %d printers: %d scanners: %d modems: %d CDs: %d\n\n",
         proc.arrivalTime, proc.priority, proc.processorTime, proc.MBytes,
         proc.numPrinters, proc.numScanners, proc.numModems, proc.numCDs);
}

// Push a process into the queue
void push(process newProc, fifoQueue **headNode) {
  fifoQueue *newNode = (fifoQueue *)malloc(sizeof(fifoQueue));
  if (!newNode) {
    fprintf(stderr, "Memory allocation failed\n");
    return;
  }
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

// Pop a process from the queue
process pop(fifoQueue **headNode) {
  process popped = {0}; // Initialize popped process
  if (*headNode == NULL) {
    return popped;
  }

  fifoQueue *temp = (*headNode);
  *headNode = (*headNode)->next;
  popped = temp->proc;
  free(temp);

  return popped;
}

// Print the queue
void printQueue(fifoQueue *headNode) {
  if (headNode == NULL) {
    printf("Queue is empty.\n");
    return;
  }
  fifoQueue *temp = headNode;
  while (temp != NULL) {
    printProcess(temp->proc);
    temp = temp->next;
  }
}

// Check if a process can run based on available resources
bool shouldRun(process toRun) {
  return (!toRun.suspended && availableMem >= toRun.MBytes &&
          sysPrinters >= toRun.numPrinters &&
          sysScanners >= toRun.numScanners &&
          sysModems >= toRun.numModems &&
          sysCDs >= toRun.numCDs);
}

// Allocate resources to a process
void allocateResources(process toRun) {
  availableMem -= toRun.MBytes;
  sysPrinters -= toRun.numPrinters;
  sysScanners -= toRun.numScanners;
  sysModems -= toRun.numModems;
  sysCDs -= toRun.numCDs;
}

// Deallocate resources for a process
void deallocateResources(process toRun) {
  availableMem += toRun.MBytes;
  sysPrinters += toRun.numPrinters;
  sysScanners += toRun.numScanners;
  sysModems += toRun.numModems;
  sysCDs += toRun.numCDs;
}

// Print available system resources
void printAvailableResources() {
  printf("Available system resources: \n");
  printf("Memory: %d, Printers: %d, Scanners: %d, Modems: %d, CDs: %d\n",
         availableMem, sysPrinters, sysScanners, sysModems, sysCDs);
}

// FIFO Queues
fifoQueue *priorityQ = NULL;
fifoQueue *user1 = NULL;
fifoQueue *user2 = NULL;
fifoQueue *user3 = NULL;

int main() {
  char buffer[256], arriveTime[10], prior[10], processTime[10], memory[10];
  char prints[10], scans[10], mods[10], cds[10];

  FILE *fp = fopen("dispatchList.txt", "r");
  if (!fp) {
    fprintf(stderr, "Error: Unable to open dispatchList.txt\n");
    return EXIT_FAILURE;
  }

  printAvailableResources();
  printf("\n");

  while (fgets(buffer, sizeof(buffer), fp)) {
    strncpy(arriveTime, strtok(buffer, ","), sizeof(arriveTime) - 1);
    strncpy(prior, strtok(NULL, ","), sizeof(prior) - 1);
    strncpy(processTime, strtok(NULL, ","), sizeof(processTime) - 1);
    strncpy(memory, strtok(NULL, ","), sizeof(memory) - 1);
    strncpy(prints, strtok(NULL, ","), sizeof(prints) - 1);
    strncpy(scans, strtok(NULL, ","), sizeof(scans) - 1);
    strncpy(mods, strtok(NULL, ","), sizeof(mods) - 1);
    strncpy(cds, strtok(NULL, ","), sizeof(cds) - 1);

    process newProc = {false, atoi(arriveTime), atoi(prior), atoi(processTime),
                       atoi(memory), atoi(prints), atoi(scans), atoi(mods),
                       atoi(cds)};

    switch (newProc.priority) {
    case 0:
      push(newProc, &priorityQ);
      break;
    case 1:
      push(newProc, &user1);
      break;
    case 2:
      push(newProc, &user2);
      break;
    case 3:
      push(newProc, &user3);
      break;
    default:
      fprintf(stderr, "Invalid priority %d. Assigning to lowest queue.\n",
              newProc.priority);
      push(newProc, &user3);
      break;
    }

    printAvailableResources();
  }

  fclose(fp);
  return 0;
}
