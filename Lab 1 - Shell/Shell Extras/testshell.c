#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "helpers.h"
#include "builtins.h"

void arr_size(char **prearray, int *in, int *out, char *infile, char *outfile, int *size_arr, int *new, int *pipenum);

bool builtins(char **array, int *shell);

void execute_command(char **array, int in, int out, int pipenum, int size_arr, char *infile, char *outfile, int *sleep, int pipeLoc[]);

int main(int argc, char *argv[]) {
    int shell = 1; 
    char infile[256] = "";
    char outfile[256] = "";

    while (shell == 1) {
        fflush(stdout);
        printf("TUShell$ ");
        char str[10000] = "";
        fgets(str, 10000, stdin);
        char *inp = strdup(str);
        char **Prearray = parse(inp, " \n");
        int in = 0, out = 0, new = 0, pipenum = 0, size_arr = 0, sleep = 0, pipelocnum = 0;
        arr_size(Prearray, &in, &out, infile, outfile, &size_arr, &new, &pipenum);
        char *array[size_arr - new];
        int pipeLoc[pipenum];
        int arr_index = 0;
        for(int i = 0; i < size_arr; i++){
            if(strcmp(Prearray[i], "<") == 0 || strcmp(Prearray[i], ">") == 0 || strcmp(Prearray[i], infile) == 0 || strcmp(Prearray[i], outfile) == 0){
                continue;
            }else if(strcmp(Prearray[i], "|") == 0){
                pipeLoc[pipelocnum] = i;
                pipelocnum++;
            }else if(strcmp(Prearray[i], "&") == 0){ 
                sleep = 1;   
                size_arr = size_arr - 1;
            }else{
                array[arr_index] = strdup(Prearray[i]);
                printf("Array[%d]: %s\n", arr_index, array[arr_index]);
                arr_index++;
            }
        }
        size_arr = size_arr - new; 
        for(int test = 0; test < pipelocnum; test++){
            printf("PipeLoc[%d]: %d\n", test, pipeLoc[test]);
        }

        if(builtins(array, &shell) == false){
            execute_command(array, in, out, pipenum, size_arr, infile, outfile, &sleep, pipeLoc);
        }
    }
    exit(EXIT_SUCCESS);
}

void arr_size(char **prearray, int *in, int *out, char *infile, char *outfile, int *size_arr, int *new, int *pipenum){
    while (prearray[*size_arr] != NULL) {
        if (strcmp(prearray[*size_arr], "<") == 0) {
            if (*in == 1) {
                puts("Cannot redirect a file to a process twice\n");
                exit(EXIT_FAILURE);
            }
            *in = 1;
            strcpy(infile, prearray[*size_arr + 1]);
            *new = *new + 2;
        } else if (strcmp(prearray[*size_arr], ">") == 0) {
            if (*out == 1) {
                puts("Cannot redirect a process to a file twice\n");
                exit(EXIT_FAILURE);
            }
            *out = 1;
            strcpy(outfile, prearray[*size_arr + 1]);
            *new = *new + 2;
        } else if (strcmp(prearray[*size_arr], "|") == 0) {
            (*new)++;
            (*pipenum)++;
        }
        (*size_arr)++;
    }
}

bool builtins(char **array, int *shell){
    if(strcmp(array[0],"exit") == 0){
        printf("Exiting Shell\n");
        *shell = 0;
        return true;
    }else if(strcmp(array[0], "help") == 0){
        help();
        return true;
    }else if(strcmp(array[0], "pwd") == 0){
        pwd();
        return true;
    }else if(strcmp(array[0], "cd") == 0){
        cd(array[0+1]);
        return true;
    }else if(strcmp(array[0], "wait") == 0){
        waitBuiltin();
        return true;
    }else{
        return false;
    }
}

void execute_command(char **array, int in, int out, int pipenum, int size_arr, char *infile, char *outfile, int *sleep, int pipeLoc[]){
    int len = 0;//Create variable for length of for loop --> If there are no pipes, run once, if there are, run as many processes as required
    if(pipenum != 0){
        len = pipenum + 1;
    }else{
        len = 1;
    }
    int pipe_arr[pipenum][2];  // Create array of n-1 pipes
    for (int i = 0; i < pipenum; i++) {//Creating outside of the if statement as if its made inside the if statment, later dup2() usage will not work, as the compiler sees a possibility where the array was not actually created
        if (pipe(pipe_arr[i]) == -1) {
            perror("Pipe Creation Failed");
            exit(EXIT_FAILURE);
        }
    }
    for(int i = 0; i < len; i++){
        int child = fork();
        if(child < 0){
            printf("Execution failed\n");
            exit(EXIT_FAILURE);
        }else if(child == 0){//Child
            int add = 0;
            for(int a = 0; a < i; a++){
                add = pipeLoc[a];
                if(a != 0){
                    add = add - pipeLoc[a-1];
                }
            }
            //add = pipeLoc[i]; 
            // if(i != 0){add = add - pipeLoc[i-1];}
             if(add != 0){add = add - 1;} 
            // if(array[i+add] == NULL){
            //     add++;
            // }
            printf("add: %d\n", add);
            printf("array[i+ add]:  %s\n", array[add + i]);
            char inp[strlen(array[i+add])+1];
            strcpy(inp, array[i + add]);
            printf("%s\n", inp);
            if(inp[0] == 47){
                strcpy(inp, array[i + add]);
            }else if(inp[0] == 46){
                char inp2[strlen(inp)-1];
                strcpy(inp2, inp + 2);
            }else{
                struct stat info;
                char *path = getenv("PATH");
                char **paths = parse(path, ":");
                int size_path = 0;
                while(paths[size_path] != NULL){
                    size_path++;
                }
                for(int i = 0; i < size_path; i++){
                    char newPath[256] = "";
                    strcat(newPath, paths[i]);
                    strcat(newPath, "/");
                    strcat(newPath, inp);
                    printf("inp: %s\n", inp); 
                    if(stat(newPath, &info) == 0){
                        strcpy(inp, newPath);
                        printf("inp: %s\n", inp);  
                        break;
                    }
                    if(i == size_path - 1){
                        printf("Not a valid program start\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            if(in == 1){//Redirect in
                int inFileID = open(infile, O_RDWR | O_CREAT, 0777);
                if(inFileID == -1){
                    puts("file open failed");
                    exit(EXIT_FAILURE);
                }
                dup2(inFileID, 0);
                close(inFileID);
            }
            if(out == 1){//Redirect out
                int outFileID = open(outfile, O_RDWR | O_CREAT | O_TRUNC, 0777);
                if(outFileID == -1){
                    puts("file open failed");
                    exit(EXIT_FAILURE);
                }
                dup2(outFileID, 1);
                close(outFileID);
            }if(pipenum != 0){//Pipe redirection --> if there are pipes, change the read and write ends appropriately
                if (i != 0) {
                    dup2(pipe_arr[i - 1][0], STDIN_FILENO);
                    close(pipe_arr[i - 1][0]);
                    close(pipe_arr[i - 1][1]);
                }
                if (i != pipenum) {
                    dup2(pipe_arr[i][1], STDOUT_FILENO);
                    close(pipe_arr[i][0]);
                    close(pipe_arr[i][1]);
                }
            }
            // int add = 0;
            // for(int a = 0; a < i; a++){
            //     add = pipeLoc[a];
            //     if(a != 0){
            //         add = add - pipeLoc[a-1];
            //     }
            // }
            // //add = pipeLoc[i]; 
            // // if(i != 0){add = add - pipeLoc[i-1];}
            //  if(add != 0){add = add - 1;} 
            // // if(array[i+add] == NULL){
            // //     add++;
            // // }
            // //printf("add: %d\n", add);
            // //printf("array[i+ add]:  %s\n", array[add + i]);
            // char inp[strlen(array[i+add])+1];
            // strcpy(inp, array[i + add]);
            // //printf("%s\n", inp);
            // if(inp[0] == 47){
            //     strcpy(inp, array[i + add]);
            // }else if(inp[0] == 46){
            //     char inp2[strlen(inp)-1];
            //     strcpy(inp2, inp + 2);
            // }else{
            //     struct stat info;
            //     char *path = getenv("PATH");
            //     char **paths = parse(path, ":");
            //     int size_path = 0;
            //     while(paths[size_path] != NULL){
            //         size_path++;
            //     }
            //     for(int i = 0; i < size_path; i++){
            //         char newPath[256] = "";
            //         strcat(newPath, paths[i]);
            //         strcat(newPath, "/");
            //         strcat(newPath, inp);
            //         //printf("inp: %s\n", inp); 
            //         if(stat(newPath, &info) == 0){
            //             strcpy(inp, newPath);
            //             //printf("inp: %s\n", inp);  
            //             break;
            //         }
            //         if(i == size_path - 1){
            //             printf("Not a valid program start\n");
            //             exit(EXIT_FAILURE);
            //         }
            //     }
            // } 

            // int loc = 0;
            // int add = 0;
            // for(int a = i; a < 0; a--){
            //     add = pipeLoc[loc];
            //     loc++; 
            // }
            // char inp[strlen(array[i])+1];
            // strcpy(inp, array[i + add]);
            // if(inp[0] == 47){
            //     strcpy(inp, array[i]);
            // }else if(inp[0] == 46){
            //     char inp2[strlen(inp)-1];
            //     strcpy(inp2, inp + 2);
            // }else{
            //     struct stat info;
            //     char *path = getenv("PATH");
            //     char **paths = parse(path, ":");
            //     int size_path = 0;
            //     while(paths[size_path] != NULL){
            //         size_path++;
            //     }
            //     for(int i = 0; i < size_path; i++){
            //         char newPath[256] = "";
            //         strcat(newPath, paths[i]);
            //         strcat(newPath, "/");
            //         strcat(newPath, inp);
            //         if(stat(newPath, &info) == 0){
            //             strcpy(inp, newPath);
            //             break;
            //         }
            //         if(i == size_path - 1){
            //             printf("Not a valid program start\n");
            //             exit(EXIT_FAILURE);
            //         }
            //     }
            // }


            char **exe = parse(array[i+add], "/");
            int size_exe = 0;
            while(exe[size_exe] != NULL){
                size_exe++; 
            }
            char **input = NULL;
            if(pipenum == 0){
                input = (char **)malloc((size_arr + 1) * sizeof(char *));
                input[0] = strdup(exe[size_exe-1]);
                for(int i = 1; i < size_arr; i++){
                    input[i] = strdup(array[i]);
                }
                input[size_arr] = NULL;
                //printf("size:  %d\n", size_arr);
            }else{
                // printf("Array[i]: %s\n", array[i]); 
                char **new = parse(array[i + add], " ");
                int newsize = 0;
                while(new[newsize] != NULL){
                //     printf("New[%d]: %s\n ", newsize, new[newsize]);
                    newsize++;
                }
                input = (char **)malloc((newsize + 1 + pipenum) * sizeof(char *));
                input[0] = strdup(exe[size_exe-1]);
                int pipelocnum = 0;
                for(int i = 1; i < newsize; i++){
                    if(i == pipeLoc[pipelocnum]){
                        input[i] = NULL;
                        pipelocnum++;
                    }else{
                        input[i] = strdup(new[i]);
                    }
                }
                input[newsize] = NULL;
            }
            printf("inp: %s\n ", inp); 
            int z = 0;
            while(input[z] != NULL) {
                printf("input[%d]: %s\n", z, input[z]);
                z++;
            }
            if(strcmp(input[0], "grep") == 0){ 
                input[1] = "txt";
            }
            printf("input[1]: %s\n",input[1 ]);
            int add2 = 0;
            for(int a = 0; a < i; a++){
                add2 = pipeLoc[a];
            }
            execv(inp, (input + add2));
        }else{
            if(pipenum != 0){//If there are pipes, close all pipe ends each time there is a return to the parent, so as to not create 'hanging ends' for pipes
                if (i != 0) {
                    close(pipe_arr[i - 1][0]);
                    close(pipe_arr[i - 1][1]);
                }
                if (i != pipenum) {
                    close(pipe_arr[i][1]);
                }
            }
            for (int i = 0; i < len; i++) {
        if (*sleep == 0) {
            wait(NULL);
        }
    }
        }
    }
    for (int i = 0; i < pipenum; i++) {
        close(pipe_arr[i][0]);
        close(pipe_arr[i][1]);
    }
}