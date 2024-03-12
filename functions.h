#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <unistd.h>
#include <stdlib.h>
int* listProcesses(int* size);
void displayProcess(int pid);
void displaySystemWide(int pid);
void displayVnodes(int pid);
void displayComposite(int pid);
void processArguments(int argc, char *argv[],int* processes, int size);

#endif