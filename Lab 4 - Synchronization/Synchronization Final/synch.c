#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

typedef struct product{
    char id[100];
    int count;
}Prod;

int main(int argc, char *argv[]){
    int pipe_arr[2];
    if (pipe(pipe_arr) == -1) {
        perror("Pipe Creation Failed");
        exit(EXIT_FAILURE);
    }
    int pid1 = fork();
    if(pid1 == 0){
        dup2(pipe_arr[1], STDOUT_FILENO);
        close(pipe_arr[0]);
        close(pipe_arr[1]);
        char* arr[] = {"producer", "product_1", NULL};
        execv("./producer", arr);
    }else{
        int pid2 = fork();
        if(pid2 == 0){
            dup2(pipe_arr[1], STDOUT_FILENO);
            close(pipe_arr[0]);
            close(pipe_arr[1]);
            char* arr[] = {"producer", "product_2", NULL};
            execv("./producer", arr);
        }else{
            int pid3 = fork();
            if(pid3 == 0){
                dup2(pipe_arr[0], STDIN_FILENO);
                close(pipe_arr[1]);
                close(pipe_arr[0]);
                char* arr[] = {"consumer", NULL};
                execv("./consumer", arr);
            }else{
                waitpid(-1, NULL, 0);
            }
        }
    }
    return 1;
}