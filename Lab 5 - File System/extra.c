//NOTE: Should probably add 1 anywhere I'm dividing

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
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
        // .fat1 = ceil(sizeof(boot) + sizeof(logicSpace) / BLOCK_SIZE),
        // .fat2 = ceil(sizeof(boot) + sizeof(logicSpace) + sizeof(FAT) / BLOCK_SIZE),
        // .logicspace = 1,
        .fat1 = 1,
        // .fat2 = ceil(sizeof(boot) + sizeof(FAT) / BLOCK_SIZE),
        .logicspace = ceil(sizeof(boot) + sizeof(FAT) / BLOCK_SIZE)
};
FAT fat1;
// FAT fat2;
logicSpace logic;
fd fileDesc;

int main(){
    if(make_fs("Hello") == -1){
        perror("Disk Creation Failure");
        exit(EXIT_FAILURE);
    }
    // char test[100];
    // memset(test, 0, 100);
    boot boot1;

    puts("Mounting...");
    mount_fs("Hello");
    puts("Mounted");

    // block_read(0, test);

    block_read(0, (char *) &boot1);
    
    // memcpy(&boot1, test, 100);

    // printf("%d\n", boot1.fat1);
    // FAT testfat;
    // block_read(1, (char *) &testfat);
    // printf("%d\n", testfat.block[0]);
    // printf("%d\n", testfat.block[100]);
    // printf("%d\n", testfat.block[500]);
    // printf("%d\n", testfat.block[1000]);
    // printf("%d\n", testfat.block[4095]);
    // puts("");

    printf("Create: %d\n", fs_create("Test"));
    int file = fs_open("Test");
    printf("Open: %d\n", file);
    printf("File Name: %s\n", logic.root[0].name);
    char text[200] = "Test12345";
    // char text[200];
    // memset(text, 'a', sizeof(text));
    printf("Write: %d\n", fs_write(file, &text, sizeof(text)));
    printf("File Size: %d\n", fs_get_filesize(file));
    char read[200] = "\0";
    printf("Read: %d\n", fs_read(file, read, sizeof(read)));
    printf("Read: %s\n", read);
    puts("Unmounting...");
    unmount_fs("Hello");
    puts("Unmounted");
    puts("");

    // puts("Mounting...");
    // mount_fs("Hello");
    // puts("Mounted...");
    // int file2 = fs_open("Test");
    // printf("Open: %d\n", file2);
    // printf("File Name: %s\n", logic.root[0].name);
    // int close2 = fs_close(file2);
    // printf("Close: %d\n", close2);
    // printf("Delete: %d\n", fs_delete("Test"));
    
    // puts("Unmounting...");
    // unmount_fs("Hello");
    // puts("Unmounted");
    
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
    // memset(&fat2, 0, sizeof(FAT));
    memset(&logic, 0, sizeof(logicSpace));
    if(block_write(0, (char *) &BOOT) == -1){//Write boot to disk
        perror("Disk Write Failure");
        return -1;
    }
    //Loop to write to first block & second block
    // if(block_write(BOOT.fat1, (char *) &fat1) == -1){//Write fat1 to disk
    //     perror("Disk Write Failure");
    //     return -1;
    // }
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
    // if(block_read(BOOT.fat1, (char *) &fat1) == -1){
    //     perror("Disk Read Failure");
    //     return -1;
    // }
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

    for(int i = 0; i < 32; i++){
        fileDesc.fds[0][i] = -1;
        fileDesc.fds[1][i] = 0;
    }

    return 0;
}

int unmount_fs(char *disk_name){
    if(block_write(0, (char *) &BOOT) == -1){//Write boot to disk
        perror("Disk Write Failure");
        return -1;
    }
    // if(block_write(BOOT.fat1, (char *) &fat1) == -1){//Write fat1 to disk
    //     perror("Disk Write Failure");
    //     return -1;
    // }
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
    for(int i = 1; i < 33; i++){
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
            // for(int j = 4096; j < 8191; j++){
                if(fat1.block[j] == -2){
                    fat1.block[j] = -1;
                    newFile.startCluster = j;
                }
            }
            logic.root[i] = newFile;
            block_write(logic.root[i].startCluster, (char *) &logic.root[i]);//Maybe this doesn't go here? Maybe it does
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
    logic.root[root].name[0] = '\0';
    return 0;
}

//Does this work??? Who the hell knows. I think so
// int fs_read(int filedes, void *buf, size_t nbyte){
//     int blocks = ceil(nbyte/BLOCK_SIZE) + 1;
//     int filePointer = fileDesc.fds[1][filedes];
//     int start = ceil(filePointer/BLOCK_SIZE) + 1;
//     int CurrBlock = logic.root[fileDesc.fds[0][filedes]].startCluster;
//     int read = 0;
//     for(int i = 0; i < start; i++){
//         CurrBlock = fat1.block[CurrBlock];
//     }
//     for(int i = 0; i < blocks; i++){
//         char* file[BLOCK_SIZE];
//         block_read(CurrBlock, (char *) file);

//         int off = filePointer % BLOCK_SIZE;
//         int bytes = BLOCK_SIZE - off;
//         if(bytes > (nbyte - bytes)){
//             bytes = nbyte - bytes;
//         }

//         memcpy(buf + bytes, file + off, bytes);

//         filePointer += bytes;
//         read += bytes;

//         CurrBlock = fat1.block[CurrBlock];
//         if(CurrBlock == EOF){
//             break;
//         }
//     }
//     fileDesc.fds[1][filedes] = filePointer;
//     return read;
// }

int fs_read(int filedes, void *buf, size_t nbyte) {
    if (nbyte == 0) {
        return 0;
    }
    int filePointer = fileDesc.fds[1][filedes];
    int start_block = (filePointer / BLOCK_SIZE);
    int CurrBlock = logic.root[fileDesc.fds[0][filedes]].startCluster;

    for (int i = 0; i < start_block; i++) {
        CurrBlock = fat1.block[CurrBlock];
        if (CurrBlock == -1) {
            return 0;
        }
    }
    int read = 0;
    int bytes_remaining = nbyte;
    char *buffer_ptr = (char *)buf;
    while (bytes_remaining > 0) {
        char block_data[BLOCK_SIZE];
        block_read(CurrBlock, (char *) block_data);

        // int off = (filePointer % BLOCK_SIZE);
        int off = 0; //NOT A PERMANENT SOLUTION
        int bytes_to_copy = BLOCK_SIZE - off;
        if (bytes_to_copy > bytes_remaining) {
            bytes_to_copy = bytes_remaining;
        }

        memcpy(buffer_ptr, block_data + off, bytes_to_copy);
        buffer_ptr += bytes_to_copy;
        read += bytes_to_copy;
        bytes_remaining -= bytes_to_copy;
        filePointer += bytes_to_copy;

        CurrBlock = fat1.block[CurrBlock];
        if (CurrBlock == -1) {
            break;
        }
    }
    fileDesc.fds[1][filedes] = filePointer;
    return read;
}

// int fs_write(int fildes, void *buf, size_t nbyte){
//     int blocks = ceil(nbyte/BLOCK_SIZE) + 1;
//     int filePointer = fileDesc.fds[1][fildes];
//     // int pointer_real = ceil(filePointer/BLOCK_SIZE) + 1;
//     int pointer_real = filePointer % BLOCK_SIZE;
//     int bytes_written = 0;
//     int CurrBlock = logic.root[fileDesc.fds[0][fildes]].startCluster;
//     while(fat1.block[CurrBlock] != -1){
//         CurrBlock = fat1.block[CurrBlock];
//     }
//     for(int i = 0; i < blocks; i++){
//         int new_block = 0;
//         for(int j = 0; j < 4096; j++){
//             if(fat1.block[j] == -2){
//                 new_block = j;
//             }
//             if(j == 4096){
//                 perror("No disk space remains");
//                 return bytes_written;
//             }
//         }
//         char* file[4096 - (filePointer % BLOCK_SIZE)];
//         // memcpy(file, buf, sizeof());
//     }
// }

int fs_write(int fildes, void *buf, size_t nbyte) {
    // if (fildes < 0 || fildes >= 32 || fileDesc.fds[0][fildes] == 0) {
    //     // Invalid file descriptor
    //     perror("fs_write: invalid file descriptor");
    //     return -1;
    // }

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
                // Disk is full
                return 0;
            }
            CurrBlock = fat1.block[CurrBlock];
        }
    }

    int written = 0;
    char *buffer_ptr = (char *)buf;

    while (nbyte > 0) {
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

        memcpy(block_data + off, buffer_ptr, bytes_to_write);
        block_write(CurrBlock, block_data);

        buffer_ptr += bytes_to_write;
        written += bytes_to_write;
        nbyte -= bytes_to_write;
        file_pointer += bytes_to_write;

        CurrBlock = fat1.block[CurrBlock];
    }
    // ((char *)buf)[written] = '\0';

    logic.root[root_index].fileSize += written;
    fileDesc.fds[1][fildes] = file_pointer;

    return written;
}

//Does this work??? Hell no.
int fs_lseek(int filedes, off_t offset){
    if(offset < 0){
        perror("Invalid offset");
        return -1;
    }
    // off_t pointer = 0;
    // int blocks = 0;
    // int ptr = fileDesc.fds[0][filedes];
    // int cluster = logic.root[ptr].startCluster;
    // while(fat1.block[cluster] != -1){
    //     // char *file = malloc(sizeof(char));
    //     char* file;
    //     block_read(cluster, (char *) file);
    //     fileDesc.fds[1][filedes] += offset;
    //     // if(fileDesc.fds[1][filedes] > ){

    //     // }
    //     // while(file != EOF){
    //     //     pointer++;
    //     //     file++;
    //     //     if(pointer == offset){
    //     //         fileDesc.fds[1][filedes] = pointer;
    //     //         return 0;
    //     //     }
    //     // }
    //     if(file == EOF && pointer == offset){
    //         fileDesc.fds[1][filedes] = pointer;
    //         return 0;
    //     }else{
    //         perror("Out of bounds");
    //         return -1;
    //     }
    //     blocks++;
    //     cluster = fat1.block[cluster];
    // }
    // blocks++; //While loop times out when = -1, but thats still a valid block
    fileDesc.fds[1][filedes] = offset;
    return 0;
}

//Does this work??? Yes. I think.
int fs_get_filesize(int fildes){
    return logic.root[fileDesc.fds[0][fildes]].fileSize;
}