# Project-0-S24
# Unix/Linux Utilities

***TULS Function (Lab 0, Part A)***

This lab is designed to help learn or relearn how to use C functions and libraries. 

Its objective was to create our own version of the Linux/Unix command 'ls', which prints out all files and directories in a given directory

To do this we had to utilize a variety of new skills, including the commands scandir() or opendir(), readdir(), and closedir(). 
We additionally had to use the stat library and the S_ISDIR() function in order to correctly ascertain if a given object was a file or a directory.

This program takes in the command for its calling from the command line, noting if there is an additional argument specifying where to look, if not, assuming the current directory.
* If there is no specification, it acts identically to the ls command
* * Scandir is called, and the objects inside the directory in question are looked at, with use of the stat library they are denoted as files or directories, and printed accordingly
  
* If there is a specification of path, that directory is opened
* Scandir is called. For each object in the directory, a certain logic is followed
* * If it is a file, it is printed normally
* * If it is a directory, it is opened, a recursive function is called and its contents are examined
* * This continues until all directories on that branch are opened, to which the program backs up and continues
* By the end, there is an indented list displaying all files and directories (including the files inside of each directory) displayed to the command line from which the program was called.


***TUCP Function (Lab 0, Part B)***

This lab is designed to help learn or relearn how to use C function and libraries

Its objective was to create our own version of the Linux/Unix command 'cp', which copies files from one specified location to another

The specific goals were to be able to
* Copy a file to a destination file, either by overwriting the file with that name, or creating it if it didn't exist
* Copy a file to a destination directory, either by overwriting the file with that name, or creating it in the specified directory if it didn't exist
* Copy multiple files into a destination directory,  either by overwriting the file with that name, or creating it in the specified directory if it didn't exist

To do this, multiple skills were used, including the commands fopen(), fwrite(), fread(), and fclose(), as well as skills from last weeks lab (Part A) such as the stat libary and S_ISDIR()

This program takes in a the command for its calling from the command line, parsing for different arguments
* If there were 3 arguments in total (including the initial call ./tucp), then the following happened
* * The program determined if the destination was a file or a directory using stat and S_ISDIR()
* * From there, it copied the specified file, assuming it existed, to the specified location
 
* If there were more than 3 arguments in total (including the initial call ./tucp), then the following happened
* * There was a check to determine if the last element in the call was a directory
* * If it was, then all the files were copied into the specified directory, assuming they initially existed
* * If it was not a directory, the program ended with an error message

* If there were less than 3 arguments in total (including the initial call ./tucp), then the program ended with an error message.
