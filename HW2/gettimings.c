// Shep Trundle
// dvf5rd
// CSO2 - HW2

#define _XOPEN_SOURCE 700
#include <time.h>
#include <stdio.h>
#include <stdlib.h>    
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>  



// Helper function for calculating time in elapsed nanoseconds
long calc_time_ns(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
}

// Times "nothing" so I can subtract from final times
long measure_overhead() {
    const int reps = 100000;
    struct timespec t1, t2;
    long total = 0;

    for (int i = 0; i < reps; i++) {
        clock_gettime(CLOCK_MONOTONIC, &t1);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        total += (t2.tv_sec - t1.tv_sec) * 1000000000L +
                 (t2.tv_nsec - t1.tv_nsec);
    }
    return total / reps;
}

// Function that does nothing (case 1)
void __attribute__((noinline)) empty_function() {}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Error: Must provide 1 argument"); 
        return 1;
    }

    long overhead = measure_overhead();
    int selection = atoi(argv[1]); 
    struct timespec t_start; 
    struct timespec t_end;
    pid_t pid;

    switch (selection) {

        // Empty function
        case 1:
            clock_gettime(CLOCK_MONOTONIC, &t_start);
            empty_function();
            clock_gettime(CLOCK_MONOTONIC, &t_end);
            // Not subtracting overhead here because its such a small time frame anyways
            printf("Case 1: %ld ns\n", calc_time_ns(t_start, t_end) - overhead);
            break;

        // Generating random number
        case 2:
            clock_gettime(CLOCK_MONOTONIC, &t_start);
            drand48();  
            clock_gettime(CLOCK_MONOTONIC, &t_end);
            printf("Case 2: %ld ns\n", calc_time_ns(t_start, t_end) - overhead);
            break;

        // getppid() function
        case 3:
            clock_gettime(CLOCK_MONOTONIC, &t_start);
            getppid();
            clock_gettime(CLOCK_MONOTONIC, &t_end);
            printf("Case 3: %ld ns\n", calc_time_ns(t_start, t_end) - overhead);
            break;
            
        // Running fork() and returning in parent process
        case 4:
            clock_gettime(CLOCK_MONOTONIC, &t_start);
            pid = fork();
            if (pid == 0) {
                // Child process
                _exit(0);   
            } else {
                // Parent process
                clock_gettime(CLOCK_MONOTONIC, &t_end);
                waitpid(pid, NULL, 0); 
                printf("Case 4: %ld ns\n", calc_time_ns(t_start, t_end) - overhead);
            }
            break;
        
        // Using waitpid() on a process that has already terminated
        case 5:
            pid = fork();
            if (pid == 0) {
                _exit(0);
            } else {
                waitpid(pid, NULL, 0); 
                clock_gettime(CLOCK_MONOTONIC, &t_start);
                waitpid(pid, NULL, WNOHANG); 
                clock_gettime(CLOCK_MONOTONIC, &t_end);
                printf("Case 5: %ld ns\n", calc_time_ns(t_start, t_end) - overhead);
            }
            break;

        // New process that immediately exits then retrieving exit status
        case 6: 
            clock_gettime(CLOCK_MONOTONIC, &t_start);
            pid = fork();
            if (pid == 0) {
                _exit(0);
            } else {
                waitpid(pid, NULL, 0);
                clock_gettime(CLOCK_MONOTONIC, &t_end);
                printf("Case 6: %ld ns\n", calc_time_ns(t_start, t_end) - overhead);
            }
            break;

        // System call
        case 7:
            clock_gettime(CLOCK_MONOTONIC, &t_start);
            // Line below needed for compiling without "ignoring value of system"
            int unused __attribute__((unused)) = system("/usr/bin/true");
            clock_gettime(CLOCK_MONOTONIC, &t_end);
            printf("Case 7: %ld ns\n", calc_time_ns(t_start, t_end) - overhead);
            break;
        
        // Create and remove directory
        case 8:
            clock_gettime(CLOCK_MONOTONIC, &t_start);
            mkdir("/tmp/test", 0700);
            rmdir("/tmp/test");
            clock_gettime(CLOCK_MONOTONIC, &t_end);
            printf("Case 8: %ld ns\n", calc_time_ns(t_start, t_end) - overhead);
            break;

        default:
            break;
    }
}
