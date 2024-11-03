#include <pthread.h>
#include<bits/stdc++.h> 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#define MAX_EVENTS 100
#define CAPACITY 500
#define BOOK_MIN 5
#define BOOK_MAX 10
#define MAX_THREADS 20
#define MAX_ACTIVE_QUERIES 5
#define MAX_TIME 1 // in seconds

pthread_mutex_t event_mutex[MAX_EVENTS]; 
pthread_mutex_t query_mutex;
pthread_cond_t query_cond;

struct query {
    int event_num;
    int type;
    int thread_num;
};

struct query active_queries[MAX_ACTIVE_QUERIES];
int num_active_queries = 0;

int available_seats[MAX_EVENTS];
pthread_t threads[MAX_THREADS];
int num_threads = 0;
pthread_barrier_t barrier;

int get_random_event() {
    return rand() % MAX_EVENTS;
}

int get_random_bookings() {
    return rand() % (BOOK_MAX - BOOK_MIN + 1) + BOOK_MIN;
}

void print_query(struct query q) {
    printf("Thread %d: ", q.thread_num);
    if(q.type == 1){
        printf("Inquiring %d\n", q.event_num);
    } else if(q.type == 2){
        printf("Booking %d seats in %d\n", q.thread_num, q.event_num);
    } else if(q.type == 3){
        printf("Cancelling a booking in %d\n", q.event_num);
    } else{
        printf("Invalid query type\n");
    }
}

void* worker_thread(void* arg) {
    int thread_num = *((int*)arg);
    free(arg);

    int bookings[MAX_EVENTS] = {0};
    int num_bookings = 0;
    srand(time(NULL) + thread_num);

    clock_t start = clock();
    while (true) {
        usleep(rand() % 10000);

        struct query q;
        q.thread_num = thread_num;
        q.event_num = get_random_event();
        q.type = rand() % 3 + 1;

        print_query(q);

        pthread_mutex_lock(&query_mutex);
        while (num_active_queries >= MAX_ACTIVE_QUERIES) {
            pthread_cond_wait(&query_cond, &query_mutex);
        }

        bool conflict = false;
        for (int i = 0; i < num_active_queries; i++) {
            struct query active_query = active_queries[i];
            if (active_query.event_num == q.event_num) {
                if (active_query.type == 2 || q.type == 2) {
                    conflict = true;
                    break;
                }
            }
        }

        if (conflict) {
            pthread_cond_wait(&query_cond, &query_mutex);
        }

        active_queries[num_active_queries++] = q;
        pthread_mutex_unlock(&query_mutex);

        switch (q.type) {
            case 1: 
                printf("Thread %d: %d seats available in %d\n", thread_num, available_seats[q.event_num], q.event_num);
                break;
            case 2:
                if (available_seats[q.event_num] >= BOOK_MIN) {
                    int num_booked = get_random_bookings();
                    if (num_booked > available_seats[q.event_num]) {
                        num_booked = available_seats[q.event_num];
                    }
                    printf("Thread %d: booking %d seats in %d\n", thread_num, num_booked, q.event_num);
                    bookings[num_bookings++] = num_booked;
                    available_seats[q.event_num] -= num_booked;
                } else {
                    printf("Thread %d: %d has no available seats\n", thread_num, q.event_num);
                }
                break;
            case 3: 
                if (num_bookings > 0) {
                    int index = rand() % num_bookings;
                    int num_cancelled = bookings[index];
                    printf("Thread %d: cancelling %d seats in %d\n", thread_num, num_cancelled, q.event_num);
                    bookings[index] = bookings[--num_bookings];
                    available_seats[q.event_num] += num_cancelled;
                } else {
                    printf("Thread %d: no bookings to cancel in %d\n", thread_num, q.event_num);
                }
                break;
            default:
                printf("Thread %d: Invalid query type\n", thread_num);
        }

        pthread_mutex_lock(&query_mutex);
        for (int i = 0; i < num_active_queries; i++) {
            if (active_queries[i].thread_num == thread_num) {
                active_queries[i] = active_queries[--num_active_queries];
                break;
            }
        }
        pthread_cond_broadcast(&query_cond);
        pthread_mutex_unlock(&query_mutex);

        clock_t end = clock();
        double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
        if (elapsed_time >= MAX_TIME) {
            break;
        }
    }

    pthread_barrier_wait(&barrier);
    return NULL;
}

int main() {
    for (int i = 0; i < MAX_EVENTS; i++) {
        pthread_mutex_init(&event_mutex[i], NULL);
        available_seats[i] = CAPACITY;
    }

    pthread_mutex_init(&query_mutex, NULL);
    pthread_cond_init(&query_cond, NULL);

    pthread_barrier_init(&barrier, NULL, MAX_THREADS);

    for (int i = 0; i < MAX_THREADS; i++) {
        int* thread_num = (int*)malloc(sizeof(int));
        *thread_num = i;
        pthread_create(&threads[i], NULL, worker_thread, thread_num);
        num_threads++;
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&query_mutex);
    pthread_cond_destroy(&query_cond);
    for (int i = 0; i < MAX_EVENTS; i++) {
        pthread_mutex_destroy(&event_mutex[i]);
    }

    return 0;
}
