#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void copy(const char *SN, const char *DN);//Declaration of copy method

char buffer[512];//declaration of the buffer w/ size
size_t read;//declaration of the size of the read object

int main(int argc, char* argv[]){
    struct stat info;//Initializes struct for entering in stat objects
    
    if(argc < 3){//If there are less than 3 arguments, print error and end program - not enough arguments to run
        printf("There are not enough arguments in this call to copy files");
        exit(EXIT_FAILURE);
    }else if(argc == 3){//if argc == 3, then copying file to a new name, or to a new directory
        char SourceName[10000] = ""; strcat(SourceName, "./"); strcat(SourceName, argv[1]);//Declare initial file name

        stat(argv[2], &info);
        if(S_ISDIR(info.st_mode) != 0){//Check if the place its going is a file or a directory
            //If the deestination is a directory, create a destination file name according to the destination directory, and initial file name
            //Call copy
            char DestName[10000] = ""; 
            strcat(DestName, "./"); strcat(DestName, argv[2]); 
            strcat(DestName, "/"); strcat(DestName, argv[1]);
            copy(SourceName, DestName);
        }else{
            //If the destination is a file, create a destination file name according to the destination file given
            //Call Copy
            char DestName[10000] = ""; strcat(DestName, "./"); strcat(DestName, argv[2]);
            copy(SourceName, DestName);
        }
    }else if(argc > 3){//If argc > 3, there is an attempt to copy multiple files to a directory
        stat(argv[argc-1], &info);
        if(S_ISDIR(info.st_mode) != 0){//Check if the destination is actually directory
            for(int i = 1; i < argc-1; i++){
                //If the destination is a directory, initialize the destination file accordingly for each file, and call copy
                char SourceName[10000] = ""; strcat(SourceName, "./"); strcat(SourceName, argv[i]);
                char DestName[10000] = ""; 
                strcat(DestName, "./"); strcat(DestName, argv[argc-1]); 
                strcat(DestName, "/"); strcat(DestName, argv[i]);
                copy(SourceName, DestName);
            }
        }else{//If the destination is not a directory, print error and end program
            printf("The specified location to copy the files is not a directory");
            exit(EXIT_FAILURE);
        }
    }
}

void copy(const char *SN, const char *DN){
    FILE *source = fopen(SN, "rb");//Open the initial file in read binary mode
        if(source == NULL){
            printf("\nSource file could not be opened\n");
            exit(EXIT_FAILURE);
        }
        FILE *dest = fopen(DN, "ab");//Open the new file in append binary mode
        if(dest == NULL){
            printf("\nDestination file could not be opened\n");
            fclose(source);
            exit(EXIT_FAILURE);
        }
        for(int i = 0; ; i++){//For each run of the for loop, until eof is hit, copy data into the buffer, and then write it to the new file
            read = fread(buffer, 1, sizeof(buffer), source);
            if(read > 0){
                fwrite(buffer, 1, read, dest);
            }else{
                break;
            }
        }
    fclose(source); fclose(dest);//When finished with the copy for the specified file, close both the initial and the new file
}