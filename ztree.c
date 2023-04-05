/* 	
	Author: Rishabh Pahwa (110091645)
	Title: COMP8567: Advanced Systems Programming - Assignment 2 (Part B)
	Submitted to: Dr. Prashanth Ranga
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>

#define MAX_CHILDREN 1000
#define MAX_DEPTH 1000
#define DEBUG_MODE 1 // will output log messages for each step when set 1

int child[MAX_CHILDREN];
int bash_pid; 

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
        if (strncmp(status_line, "State:", 6) == 0) {
            status = status_line + 7;
            if (strstr(status, "Z (zombie)") != NULL) {
                result = true;
            }
            break;
        }
    }
	
    fclose(status_file);
    return result;
}

// function to get all children of given PID
int *get_children(int pid) {
    char proc_path[100];
    DIR *dp;
    struct dirent *dirp;
    int child_pid, child_ppid;
	
	int count = 0;
	
	sprintf(proc_path, "/proc/");
	
    if ((dp = opendir(proc_path)) == NULL) {
        printf("Process is not running: %s\n", proc_path);
        return 0;
    }
	
    while ((dirp = readdir(dp)) != NULL) {
        if (dirp->d_type == DT_DIR) {
			if (!isdigit(dirp->d_name[0])) continue; // skip non-digit folders
            child_pid = atoi(dirp->d_name);
            if (child_pid == 1) continue; // skip init
			if (child_pid == pid) continue;	// skip the process itself
            
			// get child ppid 
			child_ppid = get_ppid(child_pid);
			
			if(child_ppid == pid){
				child[count] = child_pid;
				count++;
			}
        }
    }
	
	return child;

    closedir(dp);
}


// function to get elapsed time in minutes of input PID
int get_elapsed_time(int pid) {
    char filename[100], line[100], *str;
    FILE *fp;
    int clock_ticks;
    struct timespec ts;

    // Open the /proc/[pid]/stat file
    sprintf(filename, "/proc/%d/stat", pid);
    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Process with given PID %d is not running\n", pid);
        return -1;
    }

    // Read the start time of the process from the stat file
    if (fgets(line, 100, fp) == NULL) {
        perror("fgets");
        fclose(fp);
        return -1;
    }
    str = strtok(line, " ");
    for (int i = 1; i < 22; i++) {
        str = strtok(NULL, " ");
    }
    clock_ticks = atoi(str);

    // Close the stat file
    fclose(fp);

    // Get the current time and calculate the elapsed time
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        perror("clock_gettime");
        return -1;
    }
    return (int)(ts.tv_sec - (clock_ticks / sysconf(_SC_CLK_TCK)))/60;
}

// function to check if children array has any values filled (initial value ie. 0 means empty)
int hasChildren( int child[]){
	
	for(int i = 0; i < MAX_CHILDREN; i++){
		if( child[i] > 0) return 1;
	}
	return 0;
	
}

void terminateProcess(int pid){
	if( pid == bash_pid){
		printf("\nNot killing Bash Process\n");
		return;
	}
	printf("Killing PID: %d\n", pid);
	kill(pid, SIGKILL);
}

void search_process_tree(pid_t pid, int depth, int PROC_ELTIME, int NO_OF_DFCS){
	
	if (depth >= MAX_DEPTH) {
        printf("Reached maximum process tree depth\n");
        return;
    }
	
	int * childArr;
	int localChild[MAX_CHILDREN], count_defunct = 0, elapsed_time = 0;
    
	//get elapsed time of parent	
	elapsed_time = get_elapsed_time(pid);
	
	// get children of parent
	childArr = get_children(pid);
	memcpy(localChild, childArr, MAX_CHILDREN * sizeof(int)); // copy children locally
	memset(child, 0, sizeof(child)); // clear the global array
	
	// check if any children
	if(!hasChildren(localChild)) return;
	
	// count defunct children
	if(DEBUG_MODE){
		printf("\nFound child Processes of %d:\n", pid);
	}
	for(int i = 0; i <128; i++){
		if(localChild[i] == 0) continue; // skip empty values
		if(DEBUG_MODE){
			printf("%d", localChild[i]);
		}
		
		if(is_defunct(localChild[i])){
			count_defunct++;
			if(DEBUG_MODE){
				printf(" DEFUNCT\n");
			}
		}
		else{
			if(DEBUG_MODE){
				printf("\n");
			}
		}
		
		// Recursive call (depth first method)
		search_process_tree(localChild[i], depth + 1, PROC_ELTIME, NO_OF_DFCS);
		
	}
	
	if(DEBUG_MODE){
		printf("\nProcess %d has %d defunct children and %d minutes elapsed time\n", pid, count_defunct, elapsed_time);
	}
	
	// terminate parent process with elapsed time > PROC_ELTIME
	if(PROC_ELTIME > 0 && count_defunct > 0){
		
		if( elapsed_time > PROC_ELTIME){
			
			terminateProcess(pid);
			return;
		}
	}
	
	// terminate parent process with number of defunct child >= NO_OF_DFCS
	else if( NO_OF_DFCS > 0 && count_defunct > 0){
		
		if( count_defunct >= NO_OF_DFCS){
			terminateProcess(pid);
			return;
		}
	}
	
	// terminate all parent process
	else if( PROC_ELTIME == 0 && NO_OF_DFCS == 0 && count_defunct > 0){
			terminateProcess(pid);
			return;
	}
	
	if(DEBUG_MODE){
		printf("\nNot killing %d\n", pid);
	}
	
}

int main(int argc, char *argv[]) {
	
	int PROC_ELTIME = 0, NO_OF_DFCS = 0;
	
	char *option1;
	char *option2;
	
	pid_t root_pid;
	
	
    // parse command line arguments
    if (argc != 2 && argc != 4) {
        printf("Usage: ztree [root_process] [OPTION1] [OPTION2]\n");
        return 1;
    }
    
	//read root pid
	root_pid = atoi(argv[1]);
	
    if (root_pid <= 0) {
        printf("Invalid root process PID\n");
        return 1;
    }
	
    if (argc == 4) {    
		
		option1 = argv[2];
		option2 = argv[3];
		
		if(DEBUG_MODE){
			printf("\nOption 1: %s\n",option1);
			printf("\nOption 2: %s\n",option2);
		}
		
        if (strcmp(option1, "-t") == 0) {   
            PROC_ELTIME = atoi(option2);
            if (PROC_ELTIME < 1) {
                printf("Invalid PROC_ELTIME value\n");
                return 1;
            }
		}
        else if (strcmp(option1, "-b") == 0) { 
            NO_OF_DFCS = atoi(option2);
            if (NO_OF_DFCS < 1) {
                printf("Invalid NO_OF_DFCS value\n");
                return 1;
            }
        } 
		else {
			printf("Invalid option1\n");
            return 1;
		}
	}
	
	// populate bash PID
	int bash_pid = getppid();
	
    // start searching process tree
    search_process_tree(root_pid, 0, PROC_ELTIME, NO_OF_DFCS);
    return 0;
}





