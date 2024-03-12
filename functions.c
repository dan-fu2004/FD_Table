#include "functions.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdbool.h>

//this function returns an array of size 500 containing all processes controlled by current user
int* listProcesses(int* size) {
    DIR *dir;
    uid_t user_uid;
    struct dirent *entry;
    user_uid = getuid(); //GET USER ID TO CHECK WHICH PROCESS WE CAN ACCESS

    int* processes = (int*)calloc(500,sizeof(int));

    dir = opendir("/proc");
    if (dir == NULL) {
        printf("Failed to open /proc directory\n");
        exit(1);
    }

    while ((entry = readdir(dir)) != NULL) {
        int pid;
        //loop through all "files" that are processes
        if ((pid = atoi(entry->d_name)) > 0) {
            char path[256];
            sprintf(path, "/proc/%d/status", pid);
            FILE* stat = fopen(path, "r");

            if(stat == NULL){
                printf("Failed to open /proc/%d/status ",pid);
                exit(1);
            }

            char line[256];
            while(fgets(line, 255, stat ) ) {
                if (strncmp(line, "Uid:", 4) == 0) {
                    int id;
                    sscanf(line, "Uid:\t%d", &id); //Get the first UID from the line
                    if (id == user_uid) { 
                        processes[*size] = pid; //We check if it matches our ID then we add it to our list.
                        (*size)++;
                    }
                }
            }   
            fclose(stat);

        }
    }

    closedir(dir);
    return processes;
}

void displayProcess(int pid) {
    char path[256];
    sprintf(path, "/proc/%d/fd", pid); //Get Directory path from PID
    DIR* dir = opendir(path);
    struct dirent *entry;

    if (dir == NULL) {
        printf("Failed to open /proc/%d/fd directory\n", pid);
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) { //LOOP through Directory and it's file names are the FDS
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        printf("%d  %s\n", pid, entry->d_name);
    }
    
    closedir(dir);
}


void displaySystemWide(int pid) {
    char path[256];
    sprintf(path, "/proc/%d/fd", pid);
    DIR* dir = opendir(path);
    struct dirent *entry;
    if (dir == NULL) {
        printf("Failed to open /proc/%d/fd directory\n", pid);
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) { // Loop through dir
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char fd_path[500];
        char link[1024];
        ssize_t len;

        sprintf(fd_path, "/proc/%d/fd/%s", pid,entry->d_name);
        len = readlink(fd_path, link, sizeof(link)-1); // GET FILE NAME
        if(len !=-1){
            link[len] = '\0';
            printf("%d  %s       %s\n", pid, entry->d_name, link);
        }
    }
    closedir(dir);
}


void displayVnodes(int pid) {

    char path[256];
    sprintf(path, "/proc/%d/fdinfo", pid); //Get Directory path from PID
    DIR* dir = opendir(path);
    struct dirent *entry;

    if (dir == NULL) {
        printf("Failed to open /proc/%d/fdinfo directory\n", pid);
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) { //LOOP through Directory and it's file names are the FDS
        int inode = -1;
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char fd_path[500];
        sprintf(fd_path, "/proc/%d/fdinfo/%s", pid, entry->d_name);
        FILE* file = fopen(fd_path, "r");

        char line[256];
        while(fgets(line, 255, file) ){
            if (strncmp(line, "ino:", 4) == 0) {
                sscanf(line, "ino:\t%d", &inode); // GET inode info from /proc/fdinfo/<fd>
                break;
            }
        }
        fclose(file);
        printf("%s              %d\n",entry->d_name, inode);
    }
    
    closedir(dir);
}

void displayComposite(int pid) {
    
    char path[256];
    sprintf(path, "/proc/%d/fd", pid);
    DIR* dir = opendir(path);
    struct dirent *entry;
    if (dir == NULL) {
        printf("Failed to open /proc/%d/fd directory\n", pid);
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        int inode = -1;
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char fd_path[500]; //Use these variables for finding file name
        char link[1024];
        ssize_t len;

        sprintf(fd_path, "/proc/%d/fd/%s", pid,entry->d_name);
        len = readlink(fd_path, link, sizeof(link)-1); //GET file name

        if(len !=-1){
            link[len] = '\0';
        }
        
        char inode_path[500];
        sprintf(inode_path, "/proc/%d/fdinfo/%s", pid, entry->d_name);
        FILE* file = fopen(inode_path, "r");

        char line[256];
        while(fgets(line, 255, file) ){
            if (strncmp(line, "ino:", 4) == 0) {
                sscanf(line, "ino:\t%d", &inode); //GET Inode 
                break;
            }
        }
        fclose(file);
        printf("%d  %s       %s         %d\n", pid, entry->d_name, link,inode); //print it all
    }

    closedir(dir);
}

int countFD(int pid){
    char path[256];
    int count =0;
    sprintf(path, "/proc/%d/fdinfo", pid); //Get Directory path from PID
    DIR* dir = opendir(path);
    struct dirent *entry; 

    if(dir == NULL){
        printf("Failed to open /proc/%d/fdinfo directory\n", pid);
        return -1;
    }
    while ((entry = readdir(dir)) != NULL) { 
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        count++; //we just count how many FDs there are in this dir
    }
    
    closedir(dir);
    return count;
}

void compositeToText(int pid) {
    
    char path[256];
    sprintf(path, "/proc/%d/fd", pid);
    DIR* dir = opendir(path);
    struct dirent *entry;
    if (dir == NULL) {
        printf("Failed to open /proc/%d/fd directory\n", pid);
        return;
    }

    FILE* output = fopen("compositeTable.txt", "a"); // We will write the output to this file. Note we append to the end of file.
    if(output == NULL){
        printf("ERROR COULD NOT OPEN compositeTable.txt");
        return;
    }
    fprintf(output,"\n\nPID     FD       Filename       Inode\n===============================================\n");
    while ((entry = readdir(dir)) != NULL) {
        int inode = -1;
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char fd_path[500]; //Use these variables for finding file name
        char link[1024];
        ssize_t len;

        sprintf(fd_path, "/proc/%d/fd/%s", pid,entry->d_name);
        len = readlink(fd_path, link, sizeof(link)-1); //GET file name

        if(len !=-1){
            link[len] = '\0';
        }
        
        char inode_path[500];
        sprintf(inode_path, "/proc/%d/fdinfo/%s", pid, entry->d_name);
        FILE* file = fopen(inode_path, "r");

        char line[256];
        while(fgets(line, 255, file) ){
            if (strncmp(line, "ino:", 4) == 0) {
                sscanf(line, "ino:\t%d", &inode); //GET Inode 
                break;
            }
        }
        fclose(file);
        fprintf(output,"%d  %s       %s         %d\n", pid, entry->d_name, link,inode); //print it all to the our open file
    }
    fclose(output);
    closedir(dir);
}

void compositeToBinary(int pid) {
    
    char path[256];
    sprintf(path, "/proc/%d/fd", pid);
    DIR* dir = opendir(path);
    struct dirent *entry;
    if (dir == NULL) {
        printf("Failed to open /proc/%d/fd directory\n", pid);
        return;
    }

    FILE* output = fopen("compositeTable.bin", "wb"); // We will write the output to this file. Note we append to the end of file.
    if(output == NULL){
        printf("ERROR COULD NOT OPEN compositeTable.txt");
        return;
    }
    fprintf(output,"\n\nPID     FD       Filename       Inode\n===============================================\n");
    while ((entry = readdir(dir)) != NULL) {
        int inode = -1;
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char fd_path[500]; //Use these variables for finding file name
        char link[1024];
        ssize_t len;

        sprintf(fd_path, "/proc/%d/fd/%s", pid,entry->d_name);
        len = readlink(fd_path, link, sizeof(link)-1); //GET file name

        if(len !=-1){
            link[len] = '\0';
        }
        
        char inode_path[500];
        sprintf(inode_path, "/proc/%d/fdinfo/%s", pid, entry->d_name);
        FILE* file = fopen(inode_path, "r");

        char line[256];
        while(fgets(line, 255, file) ){
            if (strncmp(line, "ino:", 4) == 0) {
                sscanf(line, "ino:\t%d", &inode); //GET Inode 
                break;
            }
        }
        fclose(file);
        // Write the pid
        fwrite(&pid, sizeof(pid), 1, output);

        // Write the name of FD
        int name_length = strlen(entry->d_name) + 1; // +1 for \0
        fwrite(&name_length, sizeof(name_length), 1, output); // Write the length of the string
        fwrite(entry->d_name, sizeof(char), name_length, output); // Write the string itself

        // Write the link string
        int link_length = strlen(link) + 1; // +1 for null terminator
        fwrite(&link_length, sizeof(link_length), 1, output); // Write the length of the string
        fwrite(link, sizeof(char), link_length, output); // Write the string itself
        // Write the inode
        fwrite(&inode, sizeof(inode), 1, output);
    }
    fclose(output);
    closedir(dir);
}


// Function to process command line arguments
void processArguments(int argc, char *argv[], int* processes, int size) {
    bool perProcessFlag = false, systemWideFlag = false, vnodesFlag = false, compositeFlag = false, outputFlag = false, binaryFlag;
    int threshold = -1; // Default value if not specified
    int pid = -1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--per-process") == 0) {
            perProcessFlag = true;
        } else if (strcmp(argv[i], "--systemWide") == 0) {
            systemWideFlag = true;
        } else if (strcmp(argv[i], "--Vnodes") == 0) {
            vnodesFlag = true;
        } else if (strcmp(argv[i], "--composite") == 0) { 
            compositeFlag = true;
        } else if (strcmp(argv[i],"--output_TXT") == 0){
            outputFlag = true;
        } else if (strcmp(argv[i],"--output_binary") == 0){
            binaryFlag = true;
        } else if (strncmp(argv[i], "--threshold=", 12) == 0) {
            threshold = atoi(argv[i] + 12); //add 12 since --threshold= is 12 long
        } else {

            // Handle positional argument for specific PID
            pid = atoi(argv[i]);
        
        }
    }

    //we use this to hold then print the offending processes
    int offending_proc[500];
    int offending_fd[500];
    int offending_size = 0;
    int num_fd = 0; // Use num_fd to store number of fd

    if(pid == -1){ // we loop through all processes we have access to if not given a pid
        for(int i = 0; i<size; i++){
            pid = processes[i];
            //check if it is above threshold
            num_fd = countFD(pid);

            if(threshold!=-1 && num_fd > threshold){
                offending_proc[offending_size] = pid;
                offending_fd[offending_size] = num_fd;
                offending_size++;
            } else{
                //Call based on flag
                if(perProcessFlag) {
                    printf("\n\nPID     FD\n===============================================\n");
                    displayProcess(pid);
                }
                if(systemWideFlag) {
                    // Call display function for system-wide FD table
                    printf("\n\nPID     FD      Filename\n===============================================\n");
                    displaySystemWide(pid);
                }
                if(vnodesFlag) {
                    printf("\n\nFD            Inode\n===============================================\n");
                    displayVnodes(pid);
                }
                if(compositeFlag) {
                    printf("\n\nPID     FD       Filename       Inode\n===============================================\n");
                    displayComposite(pid);
                }

                if(outputFlag){
                    compositeToText(pid);
                }
                if(binaryFlag){
                    compositeToBinary(pid);
                }
                // If no arguments passed, show composite
                if(argc == 1) {
                    printf("\n\nPID     FD       Filename       Inode\n===============================================\n");
                    displayComposite(pid);
                }

            }
        }
    } else{ 
        num_fd = countFD(pid);
        if(threshold!= -1 && num_fd > threshold){
            offending_proc[offending_size] = pid;
            offending_fd[offending_size] = num_fd;
            offending_size++;
        } else{
            //Call based on flag
            if(perProcessFlag) {
                printf("\n\nPID     FD\n===============================================\n");
                displayProcess(pid);
            }
            if(systemWideFlag) {
                // Call display function for system-wide FD table
                printf("\n\nPID     FD      Filename\n===============================================\n");
                displaySystemWide(pid);
            }
            if(vnodesFlag) {
                printf("\n\nFD            Inode\n===============================================\n");
                displayVnodes(pid);
            }
            if(compositeFlag) {
                printf("\n\nPID     FD       Filename       Inode\n===============================================\n");
                displayComposite(pid);
            }

            if(outputFlag){
                compositeToText(pid);
            }
            if(binaryFlag){
                compositeToBinary(pid);
            }
            // If no arguments passed, show composite
            if(argc == 1) {
                printf("\n\nPID     FD       Filename       Inode\n===============================================\n");
                displayComposite(pid);
            }

        }

}
    //print out offending proccesses in a table
    if(offending_size>0 && threshold!=-1){
        printf("\n\nOFFENDING PROCCESES\n===============================================\n");
        for(int i =0; i<offending_size; i++){
            printf("PID: %d, Num_FD: %d \n", offending_proc[i],offending_fd[i]);
        }
    }

}