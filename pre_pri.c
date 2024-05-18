#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define NUM_PROCESSES 5

int global_time = 0;
int is_done = 0;
int is_timing = 1;
int is_scheduling = 0;
int is_in = 0;

int arrival_time[NUM_PROCESSES] = {0,1,2,3,4};
int execution_time[NUM_PROCESSES] = {10,28,6,4,14};
int process_priority[NUM_PROCESSES] = {3, 2, 4, 1, 2};

int process_runtime[NUM_PROCESSES];
int process_droptime[NUM_PROCESSES];
int process_starttime[NUM_PROCESSES];
int process_endtime[NUM_PROCESSES];

int finish_order[NUM_PROCESSES];
int order_index = 0;
char buf[999];
char gant[999];

pthread_t tid[NUM_PROCESSES];

struct Task {
    struct Task* next;
    int process_number;
    int priority;
};
struct Task* head_task = NULL;

void add_to_schedule(struct Task* task, int first) {
    if(first == 1) {
        while(is_scheduling != task->process_number) {}

        if(head_task == NULL) {
            head_task = task;
            is_scheduling++;
            is_in = 0;
            return;
        }

        struct Task* prev = NULL;
        struct Task* cur = head_task;

        while(cur != NULL && task->priority <= cur->priority) {
            prev = cur;
            cur = cur->next;
        }
        if(prev == NULL) {
            sprintf(buf, "P%d (%d-%d)\n", head_task->process_number+1, process_starttime[head_task->process_number], global_time);
            strcat(gant, buf);
            process_starttime[head_task->process_number] = -1;

            task->next = head_task;
            head_task = task;
        }
        else {
            prev->next = task;
            task->next = cur;
        }
        
        is_scheduling++;
        is_in = 0;
    }
    else {
        while(is_in == 1) {}

        struct Task* temp;
        while(task->next != NULL && task->priority < task->next->priority) {
            temp = task->next;
            task->next = task->next->next;
            temp->next = task;
            head_task = temp;
        }
        if(task != head_task) {
            sprintf(buf, "P%d (%d-%d)\n", head_task->process_number+1, process_starttime[head_task->process_number], global_time);
            strcat(gant, buf);
            process_starttime[head_task->process_number] = -1;
        }
    }
}

void* process_task(void* args) {
    struct Task* input_task = (struct Task*)args;

    while(process_runtime[input_task->process_number] < execution_time[input_task->process_number]) {
        int cnt = 0;

        while(is_timing == 1 && is_done == 0) {}
        if(input_task == head_task) {
            if(process_starttime[input_task->process_number] == -1) process_starttime[input_task->process_number] = global_time;
            printf("P%d: %d X %d = %d\n", input_task->process_number + 1, process_runtime[input_task->process_number] + 1, input_task->process_number + 1, (process_runtime[input_task->process_number] + 1)*(input_task->process_number + 1));
            process_runtime[input_task->process_number]++;
            input_task->priority = process_priority[input_task->process_number];
            add_to_schedule(input_task, 0);
            global_time++;
            cnt++;
            is_timing = 1;
        }
    }
    process_endtime[input_task->process_number] = global_time;
    head_task = input_task->next;
    finish_order[order_index++] = input_task->process_number;
    sprintf(buf, "P%d (%d-%d)\n", input_task->process_number + 1, process_starttime[input_task->process_number], process_endtime[input_task->process_number]);
    strcat(gant, buf);

    free(input_task);
    pthread_exit(NULL);
}

void initialize_variables() {
    for(int i = 0; i < NUM_PROCESSES; i++) process_starttime[i] = -1;
    for(int i = 0; i < NUM_PROCESSES; i++) process_droptime[i] = -1;
}

void create_new_process(int num) {
    struct Task* newp = (struct Task*)malloc(sizeof(struct Task));
    newp->priority = process_priority[num];
    newp->next = NULL;
    newp->process_number = num;
    
    is_in = 1;
    add_to_schedule(newp, 1);
    pthread_create(&tid[num], NULL, process_task, (void*)newp);
}

void wait_for_threads(int num) {
    for (int i = 0; i < num; i++) pthread_join(tid[i], NULL);
}

void print_process_info() {
    int total_return_time = 0;
    int total_wait_time = 0;
    for(int i = 0; i < NUM_PROCESSES; i++) {
        int return_time = process_endtime[i]-arrival_time[i];
        int wait_time = process_endtime[i]-process_runtime[i]-arrival_time[i];
        printf("Process %d Return Time: %2d Wait time: %2d\n", i+1, return_time, wait_time);
        total_return_time += return_time;
        total_wait_time += wait_time;
    }
    printf("Average Return Time: %0.2f Average Wait time: %0.2f\n", (float)total_return_time/NUM_PROCESSES, (float)total_wait_time/NUM_PROCESSES);
}

int main() {
    initialize_variables();
    int num = 0;

    while(global_time <= arrival_time[NUM_PROCESSES-1]) {
        if(global_time == arrival_time[num] && is_timing == 1) {
            create_new_process(num);
            num++;
            is_timing = 0;
        }
    }
    is_done = 1;
    wait_for_threads(num);
    printf("%s",gant);
    print_process_info();

    return 0;
}
