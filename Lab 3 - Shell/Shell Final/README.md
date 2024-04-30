# Project-1-S24
## Project 1: Shell
The purpose of this lab was to create a basic Unix shell, with capabilities that mirror that of a bash shell, which would teach us how to use a variety of skills taught in class including the use of fork() and exec() family functions, as well as dup2() and the usage of the file table.

This program has one main file, with 2 supporting files. The supporting files are as follows:

helpers.c: A file provided with the code as a parse function to help parse the command line, as well as other items as needed

builtins.c: A file that separately holds all of the builtin commands created for this shell

The main file (shell.c) is formatted as follows

## main(): 
The main method in this file houses the while loop that is the shell, takes in user input and parses it, and is the main place where the other methods interact to pass various arrays, file descriptors, and key/flag values that make this shell work

## arr_size(): 
This function takes in a variety of items, including the initial parsed array, the flags and descriptors for files involved in file redirection, and a counter for the number of pipes. This function is where along with calculating the size of the initial command, which is necessary throughout the rest of the program, file descriptors are set for redirection, and checks are put in place to make sure redirection does not happen multiple times in the same direction, and the number of pipes are calculated.

Although not directly part of the arr_size() array, it sets up what happens immediately after in main(), where the initial array is parsed to remove any operators (redirection, pipe, background, etc.), and a new array is created to be passed on in the future for the rest of the program to work with.

## builtins(): 
This function takes in only the new input array and the pointer value for the shell loop. It simply checks if the current command is built in to this shell, and if so, calls the appropriate function from builtins.c

## execute_command(): 
This function is the main portion of the shell. It takes in a multitude of items, including the new input array and its size, the flags and descriptors for redirection, a flag for background operators, and an array noting the location of all pipes, should there be any. The first few lines in this function initialize the size of the for loop it runs in, which is related to how many pipes, if any, are in place. After that, the program is forked, and the rest of the function can be split into 3 main portions within the child

1) Redirection:
In place are three separate checks, one for redirect in, one for redirect out, and one for pipes. Depending on which flags are active, the program uses dup2() and close() 
to correctly manipulate the file table so that standard in and standard out are correctly replaced with either files or input/output from other processes, and that read and write ends are appropriately closed so that the program does not hang anywhere

2) Path Resolution
The first argument that execv() takes in is the path of the program that is being run. This comes in a few different 'fonts'. A program that is entirely external and user-written is formatted with './program'. This is acceptable as is, and can be passed on. A program that is built into the computer, but not the shell, can either be given a full path (ex. bin/ls) which is acceptable to be passed on, or a shortened version (ex. ls) which is not. If the shortened version is given, its path needs to be resolved. The program parses the PATH environment variable of the machine it's used on and tests the shortened command on each path given by the environment variable. If one of them is correct, it is passed on. If none are correct, a warning is given to the user, and the attempt fails.

3) Input Parsing
The second argument execv() takes in is an array with the name of the program executable being run, and its arguments. This program parses the initial command for the name of the executable and adds it to the input array. It then goes on to get the rest of the arguments. If no pipes are being used, it simply goes through the rest of the initial array, adding arguments to the input array. If there were pipes used, there is an array called pipeLoc with the locations of where the pipe operator used to be. The program now adds NULL where that used to be, along with the arguments in their respective places, and passes it on. The reason for this is that execv() takes in a null-terminated array, and therefore this can be exploited to have NULL multiple times, and adjust the counter to where execv() is looking.

Now that both arguments for execv() have been created, execv() is called, and the child ends.

In the parent, all pipe endings are closed.

Additionally, there is a check for if the flag for background processes was enabled. If it is not, the program waits for whatever is being run by the child to finish. If the flag is enabled, this wait does not happen, and the shell moves on to ask for its next command.

This entire cycle repeats, starting back in main() and moving through the specified functions until the user chooses to exit, which sets the variable 'shell' to 0, and the while loop monitoring the shell process ends.

## Testing

Shell Built-ins: These worked as intended, no other testing was needed

Fork() & Execv(): These have worked with all the intended functionalities. When doing initial testing, programs such as a custom program that prints "Hello World" were used as simple tests, as well as running ls with a variety of modifiers including -a, -r, -l, and my personal favorite, --color

PATH Environment Variable: As mentioned above, I quite enjoyed testing ls with a variety of different modifiers, I also tested wc, echo, and sleep as other functions that required usage of the PATH environment variable

File Redirection: As mentioned above, I used multiple custom programs. Along with my simple Hello World program, I also wrote another simple program that utilized fgets() to read from standard in, and printed out what it received along with the tag 'New!!!', called 'outputTest'. By utilizing these two programs, I was able to test redirection in & out both separately and together, writing 'hello world' to a file (./hello > helloworld.txt) , and then giving that file to outputTest, which itself wrote to a new file (./outputTest < helloworld.txt > newhello.txt).

Pipes: By far the most difficult thing for me to accomplish in this program, pipes were tested using both of the programs mentioned above (./hello | ./outputTest) as well as various usages of ls and wc (./hello | ./outputTest | wc) (ls | ./outputTest | wc | wc)

Infinite pipes: As showcased above, once I got pipes working, I had no issue extending a singular pipe to effectively indefinite pipes on a single command line

Background Commands: Background commands were implemented simply, and tested simply. For effect, as well as easy visual testing, I used 'sleep 10' and 'sleep 10 &' to show the difference between a program in the background, one a program that is not

Wait Built-In Command: In relationship with background commands, I used wait with 'sleep 10 &' as well, to showcase how before using wait other programs could be called and immediately respond, while after using wait, they could not