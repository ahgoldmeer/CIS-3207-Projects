#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include "disk.c"
#include "disk.h"

//Struct for boot - contains locations for the FAT and Root Directory as pointers
typedef struct boot{
    int logicspace;
    int fat1;
    // int fat2;
}boot;

//Struct for FAT - contains location of files - start block number, and pointer of current block number
typedef struct FAT{
    short block[4096];
}FAT;

//32 Bytes - For any file
typedef struct file{
    char name[15];
    // char attribute[1];
    // char createTime[2];
    // char createDate[2];
    // char LastAccessTime[2];
    // char lastModifiedTime[2];
    // char lastModifiedDate[2];
    int16_t startCluster;
    int32_t fileSize;
}file;

//Array of structs, with a maximum of 64 files
typedef struct logicSpace{
    file root[64];
}logicSpace;

typedef struct filedescriptors{
    int fds[2][32];
    int open;
}fd;

int make_fs(char *disk_name);
int mount_fs(char *disk_name);
int unmount_fs(char *disk_name);
int fs_create(char *name);
int fs_open(char *name);
int fs_close(int filedes);
int fs_delete(char *name);
int fs_read(int fildes, void *buf, size_t nbyte);
int fs_write(int fildes, void *buf, size_t nbyte);
int fs_lseek(int fildes, off_t offset);
int fs_truncate(int fildes, off_t length);
int fs_get_filesize(int fildes);

boot BOOT = {
        .fat1 = 1,
        .logicspace = ceil(sizeof(boot) + sizeof(FAT) / BLOCK_SIZE)
};
FAT fat1;
logicSpace logic;
fd fileDesc;

void* testThread(){
    puts("Mounting disk...");
    mount_fs("Hello");
    puts("Mounted");
    int file2 = fs_open("Test");
    printf("Open: %d\n", file2);
    printf("File Name: %s\n", logic.root[file2].name);

    char preread[200] = "\0";
    printf("File Size: %d\n", fs_get_filesize(file2));
    printf("Call fs_read: %d\n", fs_read(file2, preread, sizeof(preread)));
    printf("Read from Test: %s\n", preread);

    printf("Truncate to length 8: %d\n", fs_truncate(file2, 8));

    char read[200] = "\0";
    printf("File Size: %d\n", fs_get_filesize(file2));
    printf("Call fs_read: %d\n", fs_read(file2, read, sizeof(read)));
    printf("Read from Test: %s\n", read);

    printf("\nCreate file: %d\n", fs_create("Test 2"));
    int file3 = fs_open("Test 2");
    printf("Open : %d\n", file3);
    printf("File Name: %s\n", logic.root[file3].name);
    printf("Write copied text from \"Test\" to file \"Test 2\": %d\n", fs_write(file3, &read, sizeof(read)));

    // printf("Lseek: %d\n", fs_lseek(file3, 1));
    char read2[200] = "\0";
    printf("Call fs_read on \"Test 2\": %d\n", fs_read(file3, read2, sizeof(read2)));
    printf("Read from Test 2: %s\n", read2);

    int close2 = fs_close(file2);
    printf("Close: %d\n", close2);
    printf("Delete Test: %d\n", fs_delete("Test"));
    
    puts("Unmounting...");
    unmount_fs("Hello");
    puts("Unmounted");

    fflush(stdout);
    pthread_exit(NULL);
}

int main(){
    if(make_fs("Hello") == -1){
        perror("Disk Creation Failure");
        exit(EXIT_FAILURE);
    }
    puts("Mounting...");
    mount_fs("Hello");
    puts("Mounted");

    printf("Create file: %d\n", fs_create("Test"));
    int file = fs_open("Test");
    printf("Open file: %d\n", file);
    printf("File Name: %s\n", logic.root[file].name);
    
    puts("\nDemonstration of fs_lseek");
    printf("File pointer: %d\n", fileDesc.fds[1][file]);
    printf("Using fs_lseek to set pointer to 1: %d\n", fs_lseek(file, 1));
    printf("File pointer: %d\n", fileDesc.fds[1][file]);
    printf("Using fs_lseek to set pointer to 0: %d\n", fs_lseek(file, 0));
    printf("File pointer: %d\n", fileDesc.fds[1][file]);
    puts("");
    
    char text[200] = "This is a test";
    // char text[4000];
    // memset(text, 'a', sizeof(text));
    printf("Writing \"This is a test\" to file: %d\n", fs_write(file, &text, sizeof(text)));
    printf("File Size: %d\n", fs_get_filesize(file));

    char read[200] = "\0";
    printf("Call fs_read: %d\n", fs_read(file, read, sizeof(read)));
    printf("Read from Test: %s\n", read);

    printf("Close file: %d\n", fs_close(file));
    puts("Unmounting disk...");
    unmount_fs("Hello");
    puts("Unmounted");
    puts("");

    pthread_t test;
    pthread_create(&test, NULL, testThread, NULL);
    pthread_join(test, NULL);
    
    exit(EXIT_SUCCESS);
}

int make_fs(char *disk_name){
    if(make_disk(disk_name) == -1){
        perror("Disk Creation Failure");
        return -1;
    }
    if(open_disk(disk_name) == -1){
        perror("Disk Opening Failure");
        return -1;
    }
    for (int i = 0; i < 4096; i++) {
        fat1.block[i] = -2;
    }
    fat1.block[4095] = -2;
    if(block_write(0, (char *) &BOOT) == -1){//Write boot to disk
        perror("Disk Write Failure");
        return -1;
    }
    for(int i = 0; i < 2; i++){
        if(block_write(BOOT.fat1+i, ((char*) &fat1) + i * BLOCK_SIZE) == -1){
            perror("Disk Write Failure");
            return -1;
        }
    }
    if(block_write(BOOT.logicspace, (char *) &logic) == -1){//Write logic space / root directory to disk
        perror("Disk Write Failure");
        return -1;
    }
    if(close_disk() == -1){
        perror("Disk Closing Failure");
        return -1;
    }
    return 0;
}

int mount_fs(char *disk_name){
    if(open_disk(disk_name) == -1){
        perror("Disk Open Failure");
        return -1;
    }
    if(block_read(0, (char *) &BOOT) == -1){
        perror("Disk Read Failure");
        return -1;
    }
    for(int i = 0; i < 2; i++){
        if(block_read(BOOT.fat1+i, ((char*) &fat1) + i * BLOCK_SIZE) == -1){
            perror("Disk Read Failure");
            return -1;
        }
    }
    if(block_read(BOOT.logicspace, (char *) &logic) == -1){
        perror("Disk Read Failure");
        return -1;
    }
    for(int i = 0; i < 32; i++){//Set file descriptors and file pointers to initial values
        fileDesc.fds[0][i] = -1;
        fileDesc.fds[1][i] = 0;
    }
    fileDesc.open = 0;
    return 0;
}

int unmount_fs(char *disk_name){
    if(block_write(0, (char *) &BOOT) == -1){//Write boot to disk
        perror("Disk Write Failure");
        return -1;
    }
    for(int i = 0; i < 2; i++){
        if(block_write(BOOT.fat1+i, ((char*) &fat1) + i * BLOCK_SIZE) == -1){
            perror("Disk Write Failure");
            return -1;
        }
    }
    if(block_write(BOOT.logicspace, (char *) &logic) == -1){//Write logic space / root directory to disk
        perror("Disk Write Failure");
        return -1;
    }
    for(int i = 1; i < 33; i++){//Set file descriptors and file pointers to initial values
        fileDesc.fds[0][i] = -1;
        fileDesc.fds[1][i] = 0;
    }
    fileDesc.open = 0;
    if(close_disk() == -1){
        perror("Disk Closing Error");
        return -1;
    }
    return 0;
}

int fs_create(char *name){
    int len = 1;
    while(name[len] != '\0'){
        len++;
    }
    if(len > 15){
        perror("File name exceeds maximum size");
        return -1;
    }
    file newFile = {0};
    strncpy(newFile.name, name, 15);
    for(int i = 0; i < 64; i++){
        if(strcmp(logic.root[i].name, name) == 0){
            perror("File with this name already exists");
            return -1;
        }
        if(logic.root[i].name[0] == '\0'){
            for(int j = 0; j < 4095; j++){
                if(fat1.block[j] == -2){
                    fat1.block[j] = -1;
                    newFile.startCluster = j;
                }
            }
            logic.root[i] = newFile;
            block_write(logic.root[i].startCluster, (char *) &logic.root[i]);
            break; 
        }
        if(i == 63){
            perror("File limit reached");
            return -1;
        }
    }
    return 0;
}

int fs_open(char *name){
    if(fileDesc.open == 32){
        perror("No remaining file descriptors");
        return -1;
    }
    for(int i = 0; i < 64; i++){
        if(strcmp(logic.root[i].name, name) == 0){
            for(int j = 0; j < 32; j++){
                if(fileDesc.fds[0][j] == -1){
                    fileDesc.fds[0][j] = i;
                    fileDesc.fds[1][j] = 0;
                    fileDesc.open++;
                    return j;
                }
            }
        }
    }
    perror("No file of that name");
    return -1;
}

int fs_close(int filedes) {
    for(int i = 0; i < 32; i++){
        if(fileDesc.fds[0][filedes] == fileDesc.fds[0][i]){
            break;
        }
        if(i == 31){
            perror("Invalid file descriptor");
            return -1;
        }
    }

    if (fileDesc.fds[0][filedes] == -1) {
        perror("File descriptor is not open");
        return -1;
    }

    fileDesc.fds[0][filedes] = -1;
    fileDesc.fds[1][filedes] = 0;
    fileDesc.open--;

    return 0;
}

int fs_delete(char *name){
    int root = -1;
    for(int i = 0; i < 64; i++){
        if(strcmp(logic.root[i].name, name) == 0){
            root = i;
        }
    }
    if(root == -1){
        perror("No file with that name");
        return -1;
    }
    for(int i = 0; i < 32; i++){
        if(fileDesc.fds[0][i] == root){
            perror("File is open");
            return -1;
        }
    }
    int CurrBlock = logic.root[root].startCluster;
    int temp = 0;
    while(fat1.block[CurrBlock] != -1){
        temp = CurrBlock;
        CurrBlock = fat1.block[CurrBlock];
        fat1.block[temp] = -2;
    }
    if(fat1.block[CurrBlock] == -1){
        fat1.block[CurrBlock] = -2;
    }
    memset(logic.root[root].name, 0, sizeof(logic.root[root].name));//Empties whole file, not just first character
    return 0;
}

int fs_read(int filedes, void *buf, size_t nbyte) {
    for(int i = 0; i < 32; i++){//Check for valid filedes
        if(fileDesc.fds[0][filedes] == fileDesc.fds[0][i]){
            break;
        }
        if(i == 31){
            perror("Invalid file descriptor");
            return -1;
        }
    }
    
    if (nbyte == 0) {
        return 0;
    }
    int filePointer = fileDesc.fds[1][filedes];
    int start_block = (filePointer / BLOCK_SIZE);
    int CurrBlock = logic.root[fileDesc.fds[0][filedes]].startCluster;

    for (int i = 0; i < start_block; i++) {//Finding correct block to start at
        CurrBlock = fat1.block[CurrBlock];
        if (CurrBlock == -1) {
            return 0;
        }
    }
    int read = 0;
    int bytes_remaining = nbyte;
    char *buffer_ptr = (char *)buf;
    while (bytes_remaining > 0) {//Reads until no more bytes
        char block_data[BLOCK_SIZE];
        block_read(CurrBlock, (char *) block_data);

        int off = (filePointer % BLOCK_SIZE);
        if(off > 0){
            off -= fs_get_filesize(filedes);
        }
        // printf("offset: %d\n", off);
        // int off = 0; //NOT A PERMANENT SOLUTION
        int bytes_to_copy = BLOCK_SIZE - off;
        if (bytes_to_copy > bytes_remaining) {
            bytes_to_copy = bytes_remaining;
        }

        memcpy(buffer_ptr, block_data + off, bytes_to_copy);//Actually doing the copy to the buffer
        buffer_ptr += bytes_to_copy;
        read += bytes_to_copy;
        bytes_remaining -= bytes_to_copy;
        filePointer += bytes_to_copy;

        CurrBlock = fat1.block[CurrBlock];
        if (CurrBlock == -1) {
            break;
        }
    }
    fileDesc.fds[1][filedes] = filePointer;//updates file pointer
    return read;
}


int fs_write(int fildes, void *buf, size_t nbyte) {
    for(int i = 0; i < 32; i++){//Check for valid filedes
        if(fileDesc.fds[0][fildes] == fileDesc.fds[0][i]){
            break;
        }
        if(i == 31){
            perror("Invalid file descriptor");
            return -1;
        }
    }

    //initial definitions
    int root_index = fileDesc.fds[0][fildes];
    int file_pointer = fileDesc.fds[1][fildes];
    int start_block = file_pointer / BLOCK_SIZE;
    // int start_block = (file_pointer / BLOCK_SIZE) + 1;
    int CurrBlock = logic.root[root_index].startCluster;

    // Move to the start block
    for (int i = 0; i < start_block; i++) {
        CurrBlock = fat1.block[CurrBlock];
        if (CurrBlock == -1) {
            int block = 0;
            for (int i = 0; i < 4096; i++) {
                if (fat1.block[i] == -2) {
                    fat1.block[i] = -1;
                    block = i;
                }
                if(i == 4095){
                    perror("No open spots in disk");
                    return -1;
                }
            }
            fat1.block[CurrBlock] = block;
            if (fat1.block[CurrBlock] == -1) {
                return 0;
            }
            CurrBlock = fat1.block[CurrBlock];
        }
    }

    int written = 0;
    char *buffer_ptr = (char *)buf;

    while (nbyte > 0) {//Write until there are no more bytes
        if (CurrBlock == -1) {
            // Allocate a new block
            int new_block = 0;
            for (int i = 0; i < 4096; i++) {
                if (fat1.block[i] == -2) {
                    fat1.block[i] = -1;
                    new_block = i;
                }
                if(i == 4095){
                    perror("No open spots in disk");
                    return -1;
                }
            }
            if (new_block == -1) {
                // Disk is full
                break;
            }
            fat1.block[CurrBlock] = new_block;
            CurrBlock = new_block;
        }

        char block_data[BLOCK_SIZE];
        block_read(CurrBlock, block_data);

        int off = file_pointer % BLOCK_SIZE;
        int bytes_to_write = BLOCK_SIZE - off;
        if (bytes_to_write > nbyte) {
            bytes_to_write = nbyte;
        }

        memcpy(block_data + off, buffer_ptr, bytes_to_write);//Update buffer
        block_write(CurrBlock, block_data);

        buffer_ptr += bytes_to_write;
        written += bytes_to_write;
        nbyte -= bytes_to_write;
        file_pointer += bytes_to_write;

        CurrBlock = fat1.block[CurrBlock];
    }

    logic.root[root_index].fileSize += written; //update file size
    fileDesc.fds[1][fildes] = file_pointer; //update file pointer

    return written;
}

int fs_lseek(int filedes, off_t offset){
    for(int i = 0; i < 32; i++){//Check for valid file descriptors
        if(fileDesc.fds[0][filedes] == fileDesc.fds[0][i]){
            break;
        }
        if(i == 31){
            perror("Invalid file descriptor");
            return -1;
        }
    }
    
    if(offset < 0){
        perror("Invalid offset");
        return -1;
    }
    fileDesc.fds[1][filedes] = offset;
    return 0;
}

int fs_get_filesize(int fildes){
    for(int i = 0; i < 32; i++){//Check for valid file descriptors
        if(fileDesc.fds[0][fildes] == fileDesc.fds[0][i]){
            break;
        }
        if(i == 31){
            perror("Invalid file descriptor");
            return -1;
        }
    }
    
    return logic.root[fileDesc.fds[0][fildes]].fileSize;
}

int fs_truncate(int filedes, off_t length) {
    for(int i = 0; i < 32; i++){//Check for valid file descriptors
        if(fileDesc.fds[0][filedes] == fileDesc.fds[0][i]){
            break;
        }
        if(i == 31){
            perror("Invalid file descriptor");
            return -1;
        }
    }
    
    //initial declarations
    int root = fileDesc.fds[0][filedes];
    int size = logic.root[root].fileSize;

    if (length > size) {
        perror("Cannot add to file via truncation");
        return -1;
    }

    //more initial declarations :)
    int CurrBlock = logic.root[root].startCluster;
    int bytes_remaining = length;

    //navigating to correct block
    while (bytes_remaining >= BLOCK_SIZE) {
        CurrBlock = fat1.block[CurrBlock];
        if (CurrBlock == -1) {
            break;
        }
        bytes_remaining -= BLOCK_SIZE;
    }

    int next = fat1.block[CurrBlock];
    while (next != -1) {
        int temp = next;
        next = fat1.block[next];
        fat1.block[temp] = -2;
    }

    if (bytes_remaining > 0) {//Removing appropriate bytes, blocks, updating FAT, etc.
        char block_data[BLOCK_SIZE];
        block_read(CurrBlock, block_data);
        memset(block_data + bytes_remaining, 0, BLOCK_SIZE - bytes_remaining);
        block_write(CurrBlock, block_data);
    } else {
        fat1.block[CurrBlock] = -1;
    }

    logic.root[root].fileSize = length; //updating file size
    fileDesc.fds[1][filedes] = length; //updating file pointer

    return 0;
}