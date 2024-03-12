#include "functions.h"
int main(int argc, char *argv[]) {
    int size = 0;
    int* processes;
    processes = listProcesses(&size);
    processArguments(argc, argv, processes, size);
    free(processes);
    return 0;
}
