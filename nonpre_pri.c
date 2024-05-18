#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_PROCESSES 5

int global_time = 0;
int is_done = 0;
int is_timing = 1;
int is_scheduling = 0;

int arrival_time[NUM_PROCESSES] = {0, 1, 2, 3, 4};
int execution_time[NUM_PROCESSES] = {10, 28, 6, 4, 14};
int process_priority[NUM_PROCESSES] = {3, 2, 4, 1, 2};

int process_runtime[NUM_PROCESSES];
int process_waittime[NUM_PROCESSES];
int process_starttime[NUM_PROCESSES];
int process_endtime[NUM_PROCESSES];

int finish_order[NUM_PROCESSES];
int order_index = 0;

pthread_t tid[NUM_PROCESSES];

struct Task {
    struct Task* next;
    int process_number;
    int priority;
};
struct Task* head_task = NULL;

void add_to_schedule(struct Task* task) {
    while (is_scheduling != task->process_number) {}

    struct Task* prev = head_task;
    struct Task* cur = head_task->next;

    while (cur != NULL && task->priority <= cur->priority) {
        prev = cur;
        cur = cur->next;
    }
    prev->next = task;
    task->next = cur;

    is_scheduling++;
}

void* process_task(void* args) {
    struct Task* input_task = (struct Task*)args;

    while (process_runtime[input_task->process_number] < execution_time[input_task->process_number]) {
        while (is_timing == 1 && is_done == 0) {}
        if (input_task == head_task) {
            if (process_starttime[input_task->process_number] == -1) process_starttime[input_task->process_number] = global_time;

            printf("Process %d: %d X %d = %d\n", input_task->process_number + 1, process_runtime[input_task->process_number] + 1, input_task->process_number + 1, (process_runtime[input_task->process_number] + 1) * (input_task->process_number + 1));
            process_runtime[input_task->process_number]++;
            global_time++;
            is_timing = 1;
        }
    }
    process_endtime[input_task->process_number] = global_time;
    head_task = input_task->next;
    finish_order[order_index++] = input_task->process_number;

    free(input_task);
    pthread_exit(NULL);
}

void initialize_variables() {
    for (int i = 0; i < NUM_PROCESSES; i++) process_starttime[i] = -1;
}

void create_new_process(int num) {
    struct Task* new_task = (struct Task*)malloc(sizeof(struct Task));
    new_task->priority = process_priority[num];
    new_task->next = NULL;
    new_task->process_number = num;
    if (num == 0) head_task = new_task;

    add_to_schedule(new_task);
    pthread_create(&tid[num], NULL, process_task, (void*)new_task);
}

void wait_for_threads(int num) {
    for (int i = 0; i < num; i++) pthread_join(tid[i], NULL);
}

void print_process_info() {
    for (int i = 0; i < NUM_PROCESSES; i++) printf("Process %d (%d-%d)\n", finish_order[i] + 1, process_starttime[finish_order[i]], process_endtime[finish_order[i]]);

    int total_return_time = 0;
    int total_wait_time = 0;
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int return_time = process_endtime[i] - arrival_time[i];
        int wait_time = process_endtime[i] - process_runtime[i] - arrival_time[i];
        printf("Process %d Return Time: %2d wait Time: %2d\n", i + 1, return_time, wait_time);
        total_return_time += return_time;
        total_wait_time += wait_time;
    }
    printf("Average Return Time: %0.2f Average Wait Time: %0.2f\n", (float)total_return_time / NUM_PROCESSES, (float)total_wait_time / NUM_PROCESSES);
}

int main() {
    initialize_variables();
    int num = 0;

    while (global_time <= arrival_time[NUM_PROCESSES - 1]) {
        if (global_time == arrival_time[num] && is_timing == 1) {
            create_new_process(num);
            num++;
            is_timing = 0;
        }
    }
    is_done = 1;
    wait_for_threads(num);
    print_process_info();

    return 0;
}
