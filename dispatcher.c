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
#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <algorithm>
#include <memory>

// Constants
#define MEMORY_POOL_SIZE 1024  // 1024 MB of total memory
#define REAL_TIME_MEMORY_SIZE 64  // 64 MB reserved for real-time processes

// Resource management: number of available resources
std::map<std::string, int> available_resources = {
    {"printers", 5},
    {"scanners", 3},
    {"modems", 2},
    {"CDs", 4}
};

// Define the process structure
struct Process {
    int process_id;
    int priority;
    int arrival_time;
    int cpu_time;
    int mem_size;
    int printers;
    int scanners;
    int modems;
    int cds;
    bool memory_allocated;
    std::string state;

    Process(int id, int pri, int arr_time, int cpu, int mem, int print, int scan, int mod, int cd)
        : process_id(id), priority(pri), arrival_time(arr_time), cpu_time(cpu), mem_size(mem),
          printers(print), scanners(scan), modems(mod), cds(cd), memory_allocated(false), state("Ready") {}
};

// Memory management: Simulate memory allocation and deallocation
class MemoryManager {
public:
    MemoryManager(int size) : memory(size, nullptr), real_time_memory_used(0) {}

    bool allocate_memory(Process &process) {
        // Check if enough memory is available
        if (process.mem_size > (MEMORY_POOL_SIZE - real_time_memory_used)) {
            return false;  // Not enough memory
        }
        // First-fit memory allocation (simplified for demonstration)
        for (int i = 0; i < MEMORY_POOL_SIZE - process.mem_size; ++i) {
            bool can_allocate = true;
            for (int j = i; j < i + process.mem_size; ++j) {
                if (memory[j] != nullptr) {
                    can_allocate = false;
                    break;
                }
            }
            if (can_allocate) {
                for (int j = i; j < i + process.mem_size; ++j) {
                    memory[j] = &process;
                }
                process.memory_allocated = true;
                real_time_memory_used += process.mem_size;
                return true;
            }
        }
        return false;
    }

    void release_memory(Process &process) {
        if (process.memory_allocated) {
            for (int i = 0; i < MEMORY_POOL_SIZE; ++i) {
                if (memory[i] == &process) {
                    memory[i] = nullptr;
                }
            }
            process.memory_allocated = false;
            real_time_memory_used -= process.mem_size;
        }
    }

private:
    std::vector<Process*> memory;  // Simulate memory pool
    int real_time_memory_used;
};

// Resource management: Check if enough resources are available
bool check_resources(const Process &process) {
    return available_resources["printers"] >= process.printers &&
           available_resources["scanners"] >= process.scanners &&
           available_resources["modems"] >= process.modems &&
           available_resources["CDs"] >= process.cds;
}

// Process scheduling: Add process to priority queue
class Dispatcher {
public:
    Dispatcher() : time(0) {}

    void add_process(Process process) {
        processes.push_back(process);
    }

    void schedule_processes() {
        std::sort(processes.begin(), processes.end(), [](const Process &a, const Process &b) {
            return a.arrival_time < b.arrival_time;  // Sort by arrival time
        });

        // Start dispatching processes
        for (auto &process : processes) {
            time = std::max(time, process.arrival_time);  // Ensure time is greater than or equal to arrival time
            std::cout << "Time " << time << ": Attempting to run Process ID: " << process.process_id
                      << " with Priority: " << process.priority << std::endl;

            if (memory_manager.allocate_memory(process) && check_resources(process)) {
                process.state = "Running";
                run_process(process);
                memory_manager.release_memory(process);  // Release memory after process completes
            } else {
                process.state = "Blocked";
                std::cout << "Process ID: " << process.process_id << " is blocked due to insufficient resources or memory" << std::endl;
                blocked_processes.push_back(process);  // Add to blocked list to try again later
            }
            time += 1;  // Simulate time passing (1 unit)
        }
    }

private:
    void run_process(Process &process) {
        std::cout << "Running Process ID: " << process.process_id << ", CPU Time: " << process.cpu_time
                  << ", State: " << process.state << std::endl;
        // Simulate running process
        for (int i = 0; i < process.cpu_time; ++i) {
            // Simulating process running by decreasing CPU time
            std::cout << "Executing Process ID: " << process.process_id << " for 1 unit of time" << std::endl;
        }
        process.state = "Terminated";
        std::cout << "Process ID: " << process.process_id << " has finished executing." << std::endl;
    }

    int time;  // Simulate the current time in the system
    MemoryManager memory_manager{MEMORY_POOL_SIZE};  // Memory manager instance
    std::vector<Process> processes;  // List of processes
    std::vector<Process> blocked_processes;  // List of blocked processes
};

// Main function
int main() {
    Dispatcher dispatcher;

    // Adding processes
    dispatcher.add_process(Process(1, 2, 1, 10, 128, 1, 1, 0, 0));
    dispatcher.add_process(Process(2, 1, 2, 5, 64, 1, 0, 1, 0));
    dispatcher.add_process(Process(3, 3, 3, 20, 256, 0, 1, 0, 2));
    dispatcher.add_process(Process(4, 0, 4, 15, 32, 0, 0, 1, 1));

    // Running the dispatcher
    std::cout << "Starting Process Scheduling..." << std::endl;
    dispatcher.schedule_processes();

    return 0;
}




