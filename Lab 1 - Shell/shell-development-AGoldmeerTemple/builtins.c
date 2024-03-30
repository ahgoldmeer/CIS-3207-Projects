#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>

void help(){
    printf("Current Commands:\n");
    printf("exit: exits shell\n");
    printf("cd [path]: takes in a directory path, changes the working directory to that path\n");
    printf("pwd: prints the current working directory\n");
    printf("wait: Waits for all running processes to end before continuing\n");
}

void pwd(){
    char buf[256];
    printf("%s\n", getcwd(buf, sizeof(buf)));
}

void cd(char *path){
    if(path != NULL){
        chdir(path);
    }else{
        printf("No specified directory\n");
    }
}

void waitBuiltin(){
    //waitpid(-1, NULL, WNOHANG);
    waitpid(-1, NULL, 0);
}