# unix-shell
A unix style shell built in C++ using system calls such as fork(), execvp() and dup2(). 

### tsh.cpp
This file contains most of my edits to the provided code base. We were tasked with implementing run(), sanitize(), parse_input(), isQuit(), run_commands(), and add_token(). The most work was done when implementing parse_input() and run_commands() both of which do most of the heavy lifting.

### void parse_input(char *cmd, list<Process *> &process_list)
This function will parse through the users input char by char breaking it up by command. It will store each command and its associated flags as tokens in a process class. Parse input works with both files and keyboard input. It returns nothing as all of its extracted information is stored in the process list.

### bool run_commands(list<Process *> &command_list)
This function will take a process list and execute each command in the process list with its associated flags. We were tasked with ensuring that commands operated sequentially for example when the user inputs "ls -l | grep hello ; echo hi" every command to the left of the semi colon will execute before the commands to the right of the semi colon. 
