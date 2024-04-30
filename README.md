# Project-3-S24
## Project 3: Implementing a Simple File System

This project had us implement a file system through use of a virtual disk. The disk that I created is laid out as follows:

### Boot Sector:
This is a struct that holds the locations of all other structures used for this disk, including my FAT, and my root directory. These locations were integers that represented
blocks on the disk. This was first block on the disk.

### FAT
This was a struct that just held an array of 4096 shorts. Each one represented an actual block of data on the disk, where the value held in it pointed to the next index
that a file was part of. This way, a file can be followed through the FAT as a representation on the disk until reaching -1, marking the end of the file. As 0 is a valid place on the FAT for a value to point to, empty values in the FAT, which mark free blocks of data on the disk, are marked -2. The FAT takes up 2 blocks of data, the 2nd and 3rd blocks on the disk.

### Root Directory
This was a struct that held an array of 64 file structs, and started at the 4th block on the disk. Each file struct held a variety of information about the file, including its name (no larger than 15 bytes as specified in the lab instructions), the block in the disk that it started on (which also represented the place in the FAT to first look for the file), and the size of the file in bytes.

-------
These parts of the disk were all on a partitioned portion of the disk designated for metadata regarding files, file allocation, and other memory. The other portion of the disk was actual blocks of data, 4096 bytes each.

--------

When allocating data blocks, I chose to do so through use of the FAT. When writing data, I iterated through the FAT, searching for blocks that were marked as -2, or empty. I would then loop through the fat doing this, finding empty blocks, writing the correct index in the FAT, writing to disk, and continuing this system until all blocks for a particular write were allocated, or until the disk ran out of room

-------

When freeing data blocks, the opposite occurred. Blocks in the FAT would be marked as -2 as directed by the bytes removed, either by deletion or truncation. This meant that the next time a write operation was looking for places to write, these showed as valid options to write to. Otherwise, 'free' data blocks were left alone until needed. This system of using the FAT to search for free blocks, and to mark if a block is able to be used / written to allows for dynamic memory allocation, writing and deleting as needed until the disk is full.

-------
## Functions 

     int make_fs(char *disk_name);  

This function creates a new and empty disk of the specified name. With help from the built-in functions, it writes the metadata (boot, FAT, root directory) to the disk for the first time. It returns 0 on success, and -1 on failure.

     int mount_fs(char *disk_name);  
     
This function takes in the disk name in order to open it, and then reads all of the metadata from the disk to the global structs, allowing it to be accessed and edited. This makes the disk usable, or 'mounted'. It returns 0 on success, and -1 on failure.

     int unmount_fs(char *disk_name);  
     
This function takes in the disk name in order to find it, and 'unmounts' or closes the disk. This is done by first writing all of the appropriate metadata back to the disk so that it is accurate, closing all file descriptors, clearing other data, and then using the built-in commands to close the disk. It returns 0 on success, and -1 on failure.

     int fs_create(char *name);
     
This function creates an empty file (as a file struct) of the specified name and adds it to the root directory. It does create or return a file descriptor. It returns 0 on success, and -1 on failure.

     int fs_open(char *name);  
This function opens the file of the name specified and creates a file pointer. A file pointer in this system points to the place in the root directory where the file is stored. Up to 32 file descriptors can exist at once. It returns the file descriptor on success, and -1 on failure.

     int fs_close(int fildes);  
     
This function takes in the file descriptor created by fs_open, and uses it to locate the specified file, and 'close it' by removing the descriptor from the file table. It returns 0 on success, and -1 on failure.

     int fs_delete(char *name);  
     
This function takes in the specified name and finds it in the root directory. From there it removes it from the root directory, the FAT, and removes all metadata regarding the file. All blocks related to the file are now considered free and can be used by another file. The file must be closed in order to be deleted. It returns 0 on success, and -1 on failure

     int fs_read(int fildes, void *buf, size_t nbyte);  
     
This function takes in a file descriptor, a buffer to read data to, and the amount of bytes to read. Using the file descriptor to find the file, the built-in function block_read is used to read the specified amount of bytes from the disk to the buffer, which is a pointer to the initial buffer in main. The amount of bytes read is returned on success, and -1 on failure. 

     int fs_write(int fildes, void *buf, size_t nbyte);  
     
This function takes in a file descriptor, a buffer which holds the data to write, and the amount of bytes to write. Using the file descriptor to find the file, the built-in function block_write is used to write the specified amount of bytes from the buffer to the disk. The amount of bytes written (either the full amount, or the amount until a full disk) is returned on success, and -1 on failure.

     int fs_get_filesize(int fildes);  
     
This function takes in the file descriptor, and using it to access the file, finds the size of the file as updated by fs_write. It returns the size of the file on success, and -1 on failure.

     int fs_lseek(int fildes, off_t offset);  
     
This function takes in the file descriptor and an offset length. Using the file descriptor to find the file, it takes the offset length and adds it to the current file pointer, altering where the file is being written or read from. It returns 0 on success, and -1 on failure.

     int fs_truncate(int fildes, off_t length);  
     
<<<<<<< HEAD
This function takes in the file descriptor and a length to truncate. Using the file descriptor to find the file, it takes the truncation length and removes that many bytes from the file. This requires updating the FAT, the file pointer, and the file size in order to show the file as the correct size, to remove the proper amount of bytes, and if necessary, mark the necessary blocks as empty (-2). It returns 0 on success, and -1 on failure.
=======
This function takes in the file descriptor and a length to truncate. Using the file descriptor to find the file, it takes the truncation length and removes that many bytes from the file. This requires updating the FAT, the file pointer, and the file size in order to show the file as the correct size, to remove the proper amount of bytes, and if necessary, mark the necessary blocks as empty (-2). It returns 0 on success, and -1 on failure.
>>>>>>> 13d126651af3612885a9c9ab94e9005be25c3021
