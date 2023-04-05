/* 	
	Author: Rishabh Pahwa (110091645)
	Title: COMP8567: Advanced Systems Programming - Assignment 2 (Part A)
	Submitted to: Dr. Prashanth Ranga
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>

#define DEBUG_MODE 0 // will output log messages for each step when set 1

int is_process_descendant_of(int pid, int root_pid) {
    char buf[256];
    int ppid;
	FILE * stat;
	
	if(DEBUG_MODE){
			printf("\nChecking if process is descendant of root PID %s\n", buf);
		}
	
    // Check if the process with ID `pid` is a descendant of the process with ID `root_pid`.
    while (pid != root_pid) {
        sprintf(buf, "/proc/%d/stat", pid);
		
		if(DEBUG_MODE){
			printf("Opening %s\n", buf);
		}
        // Open the process's /proc directory entry
        stat = fopen(buf, "r");
        if (!stat) {
				printf("\nTarget Process %d is not running\n", pid);
            return 0;
        }

        // Read the process's PPID from its /proc directory entry
        fscanf(stat, "%*d %*s %*c %d", &ppid);
        fclose(stat);
		
		if(DEBUG_MODE){
			printf("Parent process id : %d\n",ppid);
		}
		
        // If the PPID is the same as the root PID, the process is a direct child of the root process
        if (ppid == root_pid) {
            return 1;
        }

        // If the PPID is 1, the process is a descendant of the init process and not the root process
        if (ppid == 1) {
            return 0;
        }

        // Check the next process in the chain
        pid = ppid;
    }

    // The process with ID `pid` is the same as the root process ID
    return 1;
}

// function to check if a process is in defunct state
bool is_defunct(int pid) {
    char status_file_path[50];
    FILE *status_file;
    char status_line[100];
    char *status;
    bool result = false;
    
    sprintf(status_file_path, "/proc/%d/status", pid);
    status_file = fopen(status_file_path, "r");
    if (status_file == NULL) {
        perror("Error opening status file");
        exit(EXIT_FAILURE);
    }
    
    while (fgets(status_line, sizeof(status_line), status_file) != NULL) {
		if(DEBUG_MODE){
			printf("\n Status Line: %s",status_line);
		}
        if (strncmp(status_line, "State:", 6) == 0) {
            status = status_line + 7;
			if(DEBUG_MODE){
				printf("\n Status Line: %s",status);
			}
            if (strstr(status, "Z (zombie)") != NULL) {
                result = true;
            }
            break;
        }
    }
	
    fclose(status_file);
    return result;
}


// function to return the parent process id
int get_ppid(int pid) {
    char stat_path[50];
    FILE *stat;
    int ppid;
    
    sprintf(stat_path, "/proc/%d/stat", pid);
    stat = fopen(stat_path, "r");
    if (stat == NULL) {
        perror("Error opening stat file");
        exit(EXIT_FAILURE);
    }
    
	fscanf(stat, "%*d %*s %*c %d", &ppid);
    
    fclose(stat);
	
	return ppid;
	
}

// function to return the parent process id
int get_gpid(int pid) {
	
    int ppid, gpid;
    
	ppid = get_ppid(pid);
	gpid = get_ppid(ppid);
	
	return gpid;
	
}

void list_siblings(int pid, int ppid) {
    char proc_path[100];
    DIR *dp;
	FILE *fp;
    struct dirent *dirp;
    int sibling_pid, sibling_ppid;
	sprintf(proc_path, "/proc/");
	if(DEBUG_MODE){
		printf("start proc_path %s\n",proc_path);
	}
	
    if ((dp = opendir(proc_path)) == NULL) {
        printf("Unable to open directory: %s\n", proc_path);
        return;
    }

    printf("\nSibling Processes:\n");
    while ((dirp = readdir(dp)) != NULL) {
        if (dirp->d_type == DT_DIR) {
			if (!isdigit(dirp->d_name[0])) continue; // skip non-digit folders
            sibling_pid = atoi(dirp->d_name);
            if (sibling_pid == 1) continue; // skip init
            if (sibling_pid == pid) continue; // skip the process itself
			
			// get sibling ppid 
			sibling_ppid = get_ppid(sibling_pid);
            if (sibling_ppid == ppid) { // found a sibling process
                printf("PID: %d\tPPID: %d\n", sibling_pid, sibling_ppid);
            }
        }
    }

    closedir(dp);
}

void list_children(int pid) {
    char proc_path[100];
    DIR *dp;
    struct dirent *dirp;
    int child_pid, child_ppid;
	
	sprintf(proc_path, "/proc/");
	
	if(DEBUG_MODE){
		printf("start proc_path %s\n",proc_path);
	}
	
    if ((dp = opendir(proc_path)) == NULL) {
        printf("Unable to open directory: %s\n", proc_path);
        return;
    }

    printf("Child Processes:\n");
    while ((dirp = readdir(dp)) != NULL) {
        if (dirp->d_type == DT_DIR) {
			if (!isdigit(dirp->d_name[0])) continue; // skip non-digit folders
            child_pid = atoi(dirp->d_name);
            if (child_pid == 1) continue; // skip init
			if (child_pid == pid) continue;	// skip the process itself
            
			// get child ppid 
			child_ppid = get_ppid(child_pid);
			
			if(child_ppid == pid){
				printf("PID: %d PPID: %d\n", child_pid, child_ppid);
			}
        }
    }

    closedir(dp);
}

void list_children_defunct(int pid) {
    char proc_path[100];
    DIR *dp;
    struct dirent *dirp;
    int child_pid, child_ppid;
	
	sprintf(proc_path, "/proc/");
	
	if(DEBUG_MODE){
		printf("start proc_path %s\n",proc_path);
	}
	
    if ((dp = opendir(proc_path)) == NULL) {
        printf("Unable to open directory: %s\n", proc_path);
        return;
    }

    printf("Defunct Child Processes:\n");
    while ((dirp = readdir(dp)) != NULL) {
        if (dirp->d_type == DT_DIR) {
			if (!isdigit(dirp->d_name[0])) continue; // skip non-digit folders
            child_pid = atoi(dirp->d_name);
            if (child_pid == 1) continue; // skip init
			if (child_pid == pid) continue;	// skip the process itself
			
            // get child ppid 
			child_ppid = get_ppid(child_pid);
			
			if(child_ppid == pid && is_defunct(child_pid)){
				printf("PID: %d PPID: %d\n", child_pid, child_ppid);
			}
        }
    }

    closedir(dp);
}

void list_grandchildren(int pid) {
    char proc_path[100];
    DIR *dp;
    struct dirent *dirp;
    int child_pid, child_ppid;
	
	sprintf(proc_path, "/proc/");
	
	if(DEBUG_MODE){
		printf("start proc_path %s\n",proc_path);
	}
	
    if ((dp = opendir(proc_path)) == NULL) {
        printf("Unable to open directory: %s\n", proc_path);
        return;
    }

    printf("Grandchild Processes:\n");
    while ((dirp = readdir(dp)) != NULL) {
        if (dirp->d_type == DT_DIR) {
			if (!isdigit(dirp->d_name[0])) continue; // skip non-digit folders
            child_pid = atoi(dirp->d_name);
            if (child_pid == 1) continue; // skip init
			if (child_pid == pid) continue;	// skip the process itself
            
			// get child ppid 
			child_ppid = get_ppid(child_pid);
			
			if(get_ppid(child_ppid) == pid){
				printf("PID: %d PPID: %d\n", child_pid, child_ppid);
			}
        }
    }
    closedir(dp);
}

int main(int argc, char *argv[]) {
	
    char *root_pid_str, *target_pid_str;    
    int root_pid = 0, target_pid = 0;    
    char *option;    
    
	
	
	// validate input arguments
	if (argc == 4) {    
        option = argv[3];  
	}
	// no option selected
	else if (argc == 3){
		option = NULL;
	}
	// wrong number of arguments
    else {    
        printf("\nUsage: %s [root_process] [process_id] [OPTION]\n", argv[0]);
        return 1;
    }
	
    // read root and target process id
    root_pid_str = argv[1];    
    target_pid_str = argv[2];  
    
	// convert PID string to integer
	root_pid = atoi(root_pid_str);    
    target_pid = atoi(target_pid_str);    
	
	if(DEBUG_MODE){
		printf("Root Process (int) : %d\n", root_pid);
		printf("Target Process (int) : %d\n", target_pid);
		printf("Option : %s\n", option);
    }
	
	// check if the target process PID is valid
    if (target_pid <= 0) {    
        printf("\nInvalid process ID: %s\n", target_pid_str);
        return 1;
    }
	
	
	// if the root process is the current process
	// use the init process as the root process
    if (strcmp(root_pid_str, "self") == 0) {    
        root_pid_str = "1";    
    }
	
	// check if the root process is a descendant of the bash process
    if (!is_process_descendant_of(root_pid, getppid())) {    
        printf("\nRoot process %s is not a descendant of the current bash process\n", root_pid_str);
        return 1;
    }
    
	// check if the target process is a descendant of the root process
    if (!is_process_descendant_of(target_pid, root_pid)) {    
        printf("\nTarget process %s is not a descendant of the root process %s\n", target_pid_str, root_pid_str);
        return 1;
    }
	
	// always print the PID and PPID of the target process
	printf("\nPID: %d\tPPID: %d\n", target_pid, get_ppid(target_pid));
	
    
    if (option != NULL) {    
        if (strcmp(option, "-c") == 0) {   
			list_children(target_pid);
        }
        else if (strcmp(option, "-s") == 0) {  
			list_siblings(target_pid , get_ppid(target_pid) );
        }
        else if (strcmp(option, "-gp") == 0) {   
			printf("\n Grandparent PID: %d\n", get_gpid(target_pid));
        }
        else if (strcmp(option, "-gc") == 0) {    
			list_grandchildren(target_pid);
        }
        else if (strcmp(option, "-z") == 0) {   
			if(is_defunct(target_pid)){
				printf("\nDEFUNCT\n");
			}
        }
        else if (strcmp(option, "-zl") == 0) {    
			list_children_defunct(target_pid);
        }
        else {    
			// the option is invalid
            printf("Invalid option: %s\n", option);
            return 1;
        }
    }
    return 0;  
}