# System Wide FD TABLE



## What does this do

This is a tool to display tables to keep track of open files, File Descriptors and processes,
We generate:

process FD table
System-wide FD Table
Vnodes FD table, and
a composed view of the previous table

## How I solved this problem

Most of the necessary information is contained inside the /proc directory for linux based OS.
I sub directory of /proc might look like this:
/proc/*pid*/ 
and from this directory we can access /proc/*pid*/fd and /proc/*pid*/fdinfo
which contain the information.

A major obstacle is finding out which process belongs to us since we can only access process information that belong to the us, the user.
We use the unist library to find out user information 
then we can look into /proc/*pid*/status to check whether the process ID matches our user ID, if it does that we know this process belongs to us and we can access the information.

The basic operation flow of our program runs like this:
Loop through all the processes then
Check whether this process belongs to us
If it does then we store the process ID 
and if it doesn't match, we disregard the process.

We then use the stored process information to build our tables.

I thought storing all the information into arrays or structs then using that to build and print our tables was too much of a hassle with memory and I instead opted to build the tables 
as we go along as you'll see in the documentation.

## How to use this tool

I've created a Makefile for this tool that contains all the information needed to compile.
this project is only split into 3 seperate files: functions.c, functions.h, FD.c

you can use the command make or make all which will compile everything into a executable called FD

to run the executable just call ./FD with your necessary flag such as ./FD --composite --threshold==10

I've also implemented --output_TXT and output_binary which stores the composite table in a text and binary file respectively

IF you want to information about a specific process pass the process id into the command line arugment like ./FD 19902
or the tool will parse through all processes

## Implementation and FLAGS

--per-process, indicates that only the process FD table will be displayed. For this it will list the PID and number of assigned FDs, e.g. PID (FD)

--systemWide, indicates that only the system-wide FD table will be displayed. PID FD FILENAME will be outputed 

--Vnodes, indicates that the Vnodes FD table will be displayed.  FD and INODE will be outputed

--composite, indicates that only the composed table will be displayed. PID FD FILENAME INODE will be output 

--threshold=X, where X denotes an integer, indicating that processes which have a number of FD assigned larger than X should be flagged in the output.

the default behaviour IE NO flags will just output a composite table.

and --output_TXT and --output_Binary stores the composite table in a text and binary file respectively

**NOTE** 
If using ./FD --threshold=X only the offending processes will be printed you have to combine --threshold with another flag to actually get the other tables. 


## DOCUMENTATION

int* listProcesses(int* size);
void displayProcess(int pid);
void displaySystemWide(int pid);
void displayVnodes(int pid);
void displayComposite(int pid);
void processArguments(int argc, char *argv[],int* processes, int size);
int countFD(int pid);
void compositeToText(int pid);
void compositeToBinary(int pid);

these are my functions, int pid represents the numerical value of a process id.

**int* listProcesses(int* size);**

    This function returns an array containing all the valid PID that we can accesses.
    We input a size pointer which is intially set to 0. As we add values to our array we increment Size to keep track. 
    NOTE THE ARRAY SIZE IS SET TO 500 MEANING if we have over 500 processes we will encounter unexpected behaviour

    we use the directory library to loop through the /proc directory then we check the /proc/pid/status to see if this process belongs to us.

**void processArguments(int argc, char *argv[],int* processes, int size);**

    This function is the parses through our command line arguments and then changes flags to decide the output.
    You can see we pass all the valid process IDS and the size which is represented by int* processes and int size respectively.
    
    If the use does not pass in a process ID, we loop through all the process IDs in our processes array then we pass them each individually off to 
    our helper functions:
    void displayProcess(int pid);
    void displaySystemWide(int pid);
    void displayVnodes(int pid);
    void displayComposite(int pid);

    which prints their corrosponding table.

    If a threshold size is passed through, we count all the FDs in a process beforehand using countFD(int pid) and check if this particular process is in violation.

**void displayProcess(int pid);**
**void displaySystemWide(int pid);**
**void displayVnodes(int pid);**
**void displayComposite(int pid);**

    These functions are all similar, We use a function to print each table respectively.
    The general outline for each function is very similar,
    we open the /proc/pid/fd or /proc/pid/fdinfo for each pid that is passed to these functions.
    Looking at these directories we gain information about everyting we need.

    displayProcesses only loops through the directory to find the FDs
    displaySystemWide gets the filenames for each FD using readlink 
    displayVnodes gets inode information form /proc/pid/fdinfo/fd
    displayComposite combines all the information

**void compositeToText(int pid);**
**void compositeToBinary(int pid);**

    These functions are the same as display composite, but instead of printing to the terminal,
    we print to their respective files in binary and text.


## Comparing --output_TXT and --output_binary

    These are the average times for time ./FD --output_TXT and time ./FD --output_binary (10 runs) for all PIDS 

    Binary
    real    0m0.128s
    user    0m0.003s
    sys     0m0.010s

    TEXT 
    real    0m0.082s
    user    0m0.002s
    sys     0m0.011s

    This is the output for 1 Process (avg of 10)

    Binary
    real    0m0.014s
    user    0m0.003s
    sys     0m0.004s


    TEXT
    real    0m0.012s
    user    0m0.003s
    sys     0m0.004s

    and the size of the files(using du-h compositeTable.txt and du-h compositeTable.bin)

    144 kilobytes text
    24 kilobyte binary 


    Performance: When processing all PIDs, the text output is faster in real time compared to the binary output. This might be because the text generation is computationally less intensive, even though the binary data size is smaller. However, for a single process, the performance difference is minimal, with the text format being slightly faster.

    File Size: The binary file is significantly smaller than the text file. This is expected because binary format is more space-efficient, especially for numerical data, as it doesn't require additional characters for human readability (like spaces, newlines, or digits representation)