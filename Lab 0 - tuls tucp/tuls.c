#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void recPrint(const char *path, const char *name, int num);//Declaration of recursive print statemnt

int main(int argc, char* argv[]){
    struct dirent **list;//initialize struct for entering in scandir objects
    int x;
    if(argc >= 0){
        //printf("%d\n", argc); //Shows the value of argc - tester to determine how inputs are seen
        if(argc == 1){//If no additional entry, argc = 1 because of the initial call
            x = scandir(".", &list, NULL, NULL);//Scandir call, calls the current path, no sort or filter
        }
        else if(argc == 2){//If additional entry/path, argc = 2 with argument
            x = scandir(argv[1], &list, NULL, NULL);//Scandir call, calls the current path, no sort or filter
        }else{
            printf("tuls cannot open directory \n");
            exit(EXIT_FAILURE);
        }
    }else{
        printf("tuls cannot open directory \n");
        exit(EXIT_FAILURE);
    }
    printf("Files:\n");
    if (x == -1) {
        perror("scandir");
        printf("\n");
        printf("tuls cannot open directory \n");
        exit(EXIT_FAILURE);
    }
    while (x--){
        struct stat info;//Initializes struct for entering in stat objects

        if(argc == 2){//If there was an additional argument
            //printf("\n%s", argv[1]);
            //printf("%s\n\n", list[x]->d_name);


            char s[10000] = "";//Initialize a string to create the relative path through concatanation
            strcat(s, argv[1]);
            //printf("\n\n%s\n", s);
            strcat(s, "/");
            //printf("\n%s\n", s);
            strcat(s, list[x]->d_name);
            //printf("\n%s\n\n", s);
            stat(s, &info);//Runs the stat buffer
            if(list[x]->d_name[0] != '.'){//Checks if the file starts with '.', and ignores it if it does
                if(S_ISDIR(info.st_mode)){//Checks if the current item is a directory
                    printf("[[%s]]\n",list[x]->d_name);
                    recPrint(argv[1], list[x]->d_name, 1);//If its a directory, calls the recursive print function
                }else{
                    printf("->%s\n",list[x]->d_name);//if not, just prints the current file
                } 
            }
        }else{//Checks for directory or no directory, and prints it (no recursive call, because no additional argument)
            stat(list[x]->d_name, &info);
            if(list[x]->d_name[0] != '.'){
                if(S_ISDIR(info.st_mode)){
                    printf("[[%s]]\n",list[x]->d_name);
                }else{
                    printf("->%s\n",list[x]->d_name);
                } 
            }
        }
        free(list[x]);//After all is done, frees the memory to avoid memory leaks/seg faults
    }
    exit(EXIT_SUCCESS);
}

void recPrint(const char *par, const char *name, int num){
    struct dirent **list;
    int x;
    char path[10000];//Initialize a string to create the relative path through concatanation
    strcpy(path, par);
    strcat(path, "/");
    strcat(path, name);

    x = scandir(path, &list, NULL, NULL);//Scandir call, calls the current path, no sort or filter

    if(x == -1){
        perror("scandir");
        printf("\n");
        printf("tuls cannot open directory \n");
        exit(EXIT_FAILURE);
    }
    
    while (x--){//This loop, with the exception of the continued recursive call, acts identical to the initial while loop for printing in the main method
        //printf("|%s|\n", path);
        struct stat info;
        char s[10000] = "";//Initialize a string to create the relative path through concatanation
        strcat(s, path);
        strcat(s, "/");
        strcat(s, list[x]->d_name);
        stat(s, &info);
        if(list[x]->d_name[0] != '.'){
            if(S_ISDIR(info.st_mode)){
                for(int i = 0; i < num; i++){
                    printf("\t");
                }
                printf("[[%s]]\n",list[x]->d_name);
                recPrint(path, list[x]->d_name, num+1);
                free(list[x]);
            }else{
                for(int i = 0; i < num; i++){
                    printf("\t");
                }
                printf("->%s\n",list[x]->d_name);
                free(list[x]);  
            } 
        }
    }
}