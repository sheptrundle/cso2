// Shep Trundle
// dvf5rd
// CSO 2

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    char line[128];

    // Runs 10000 times and calculates average for each case 1-8
    for (int c = 1; c < 9; c++) {
        long total = 0;

        int reps = 0;
        clock_t start = clock(); 
        double duration = 1.0;    
        while (((double)(clock() - start)) / CLOCKS_PER_SEC < duration) {
            char cmd[64];
            snprintf(cmd, sizeof(cmd), "./gettimings %d", c);
            FILE *fp = popen(cmd, "r");
            if (fgets(line, sizeof(line), fp) != NULL) {
                long t;
                sscanf(line, "Case %*d: %ld", &t);  
                total += t;
            }
            pclose(fp);
            reps++;
        }
        printf("Average time for case %d: %f ns\n", c, (double)total / reps);
    }
}
