#include "life.h"
#include <pthread.h>
#include <stdlib.h>

// Struct to hold all the data each thread needs.
typedef struct {
    LifeBoard *current_board;
    LifeBoard *next_board;
    int thread_id;
    int total_threads;
    int total_steps;
    pthread_barrier_t *barrier_after_compute;
    pthread_barrier_t *barrier_after_swap;
} ThreadArgs;


// Gets execute by each thread to compute its portion of the board
static void *life_thread(void *arg) {
    ThreadArgs *args = arg;

    int width = args->current_board->width;
    int height = args->current_board->height;
    int interior_rows = height - 2; 

    // Divide interior rows evenly among threads and distribute remainder
    int rows_per_thread = interior_rows / args->total_threads;
    int extra_rows = interior_rows % args->total_threads; 

    // Compute number of rows assigned to thread
    int rows_assigned = rows_per_thread;
    if (args->thread_id < extra_rows) {
        rows_assigned += 1;
    }

    // Compute first and last row thread should work on
    int start_row = 1 + args->thread_id * rows_per_thread;
    if (args->thread_id < extra_rows) {
        start_row += args->thread_id;
    } else {
        start_row += extra_rows;
    }
    int end_row = start_row + rows_assigned - 1;

    // Iterate over each simulation step
    for (int step = 0; step < args->total_steps; step++) {
        // Get the next board values for current threads rows
        for (int y = start_row; y <= end_row; y++) {
            for (int x = 1; x < width - 1; x++) {
                int live_neighbors = 0;

                // Count all neighbors (adjacent and diagonal)
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (LB_get(args->current_board, x + dx, y + dy))
                            live_neighbors++;
                    }
                }

                // Game of life rules, see if it lives or dies
                LB_set(args->next_board, x, y,
                       live_neighbors == 3 ||
                       (live_neighbors == 4 && LB_get(args->current_board, x, y))
                );
            }
        }
        // Synchronize all threads
        int barrier_result = pthread_barrier_wait(args->barrier_after_compute);

        // Serial thread swaps boards
        if (barrier_result == PTHREAD_BARRIER_SERIAL_THREAD) {
            LB_swap(args->current_board, args->next_board);
        }
        pthread_barrier_wait(args->barrier_after_swap);
    }
    return NULL;
}

void simulate_life_parallel(int threads, LifeBoard *state, int steps) {
    // Allocate next_board once; threads write their updates here
    LifeBoard *next_board = LB_new(state->width, state->height);
    pthread_t thread_ids[threads];     
    ThreadArgs thread_args[threads];   

    // Synchronization barriers
    pthread_barrier_t barrier_after_compute;
    pthread_barrier_t barrier_after_swap;
    pthread_barrier_init(&barrier_after_compute, NULL, threads);
    pthread_barrier_init(&barrier_after_swap, NULL, threads);
    
    // Create threads
    for (int i = 0; i < threads; i++) {
        thread_args[i] = (ThreadArgs){
            .current_board = state,
            .next_board = next_board,
            .thread_id = i,
            .total_threads = threads,
            .total_steps = steps,
            .barrier_after_compute = &barrier_after_compute,
            .barrier_after_swap = &barrier_after_swap
        };
        pthread_create(&thread_ids[i], NULL, life_thread, &thread_args[i]);
    }

    // Wait for all to finish
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    // Destroy barriers now and free memory
    pthread_barrier_destroy(&barrier_after_compute);
    pthread_barrier_destroy(&barrier_after_swap);
    LB_del(next_board);
}
