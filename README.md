# prctree
The prctree C program searches for a specified process in the process tree rooted at a given process and outputs information based on the input parameters.

## Usage
### prctree [root_process] [process_id] [OPTION]

root_process is the PID of a process that is a descendant of a current BASH process.
process_id is the PID of a process that is a descendant of a current BASH process.

### OPTION can be one of the following:
1. -c lists the PIDs of all the child processes (immediate descendants) of process_id
2. -s lists the PID and PPID of all the sibling processes of process_id
3. -gp lists the PID of the grandparent of process_id
4. -gc lists the PIDs and PPIDs of all the grandchildren of process_id
5. -z prints if process_id is DEFUNCT/NOT DEFUNCT
6. -zl lists the PIDs of all the child processes of process_id that are currently in the defunct state

# ztree
The ztree C program searches for defunct processes in the process tree rooted at a specified process and forcefully terminates the parent processes based on the input parameters.

## Usage
### ztree [root_process] [OPTION1] [OPTION2]

root_process is the PID of a process that is a descendant of a current BASH process.

### OPTION1 can be one of the following:
1. -t forcefully terminates parent processes (whose elapsed time is greater than PROC_ELTIME) of all the defunct processes in the process tree rooted at root_process
2. -b forcefully terminates all the processes in the process tree rooted at root_process that have NO_OF_DFCS defunct child processes.
### OPTION2 can be used with -t and -b options:
1. PROC_ELTIME is the elapsed time of the process in minutes since it was created (>=1)
2. NO_OF_DFCS is the number of defunct child processes (>=1)
