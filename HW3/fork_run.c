// Shep Trundle
// dvf5rd
// CSO2: HW3 - fork

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

void writeoutput(const char *command, const char *out_path, const char *err_path) {
    pid_t pid = fork();
    // Fork failed
    if (pid < 0) {
        perror("fork failure");
        return;
    }

    // In child process
    if (pid == 0) {
        // std out
        int out_fd = open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        // std error
        int err_fd = open(err_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (err_fd < 0) {
            _exit(1);
        }
        // stdout goes to out fd
        if (dup2(out_fd, STDOUT_FILENO) < 0) {
            _exit(1);
        }
        // stderr goes to err fd
        if (dup2(err_fd, STDERR_FILENO) < 0) {
            _exit(1);
        }
        close(out_fd);
        close(err_fd);
        // run command
        execl("/bin/sh", "sh", "-c", command, (char *)NULL);
    }
    // Parent process
    else {
        int status;
        waitpid(pid, &status, 0);
    }
}

void parallelwriteoutput(int count, const char **argv_base, const char *out_file) {
    pid_t *pids = malloc(sizeof(pid_t) * count);

    // open output file
    int out_fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    // iterate over count number of children
    for (int i = 0; i < count; i++) {
        pid_t pid = fork();

        // parent process
        if (pid == 0) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);

            // get argv_base length
            int argc_base = 0;
            while (argv_base[argc_base] != NULL) argc_base++;

            // allocate mem for new argv array
            char **child_argv = malloc(sizeof(char*) * (argc_base + 2));
            for (int j = 0; j < argc_base; j++) {
                child_argv[j] = (char*)argv_base[j];  
            }

            // create string and execute
            char index_str[20];
            snprintf(index_str, sizeof(index_str), "%d", i);
            child_argv[argc_base] = index_str;
            child_argv[argc_base + 1] = NULL;
            execv(argv_base[0], child_argv);
        } 
        // child process
        else {
            pids[i] = pid;
        }
    }
    close(out_fd);
    // wait for children and free
    for (int i = 0; i < count; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }
    free(pids);
}