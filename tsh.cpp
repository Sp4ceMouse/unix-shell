/*--------------------------------------------------*/
//NAME:           Joseph Schneider
//FILE:           tsh.cpp
//CLASS:          CICS-377 Operating Systems
//ASSIGNMENT:     Project_01 UNIX Shell
//PROFESSOR:      Nikko
//DATE COMPLETED: 09/29/2025
//DATE DUE:       10/02/2025
/*--------------------------------------------------*/

#include <tsh.h>

using namespace std;

//----- done -----
/**
 * (Do not modify)
 * @brief
 * Helper function to print the PS1 pormpt.
 *
 * Linux has multiple promt levels
 * (https://wiki.archlinux.org/title/Bash/Prompt_customization): PS0 is
 * displayed after each command, before any output. PS1 is the primary prompt
 * which is displayed before each command. PS2 is the secondary prompt displayed
 * when a command needs more input (e.g. a multi-line command). PS3 is not very
 * commonly used
 */
void display_prompt() { cout << "$ " << flush; }

//----- done -----
/**
 * (Do not modify)
 * @brief Cleans up allocated resources to prevent memory leaks.
 *
 * This function deletes all elements in the provided list of Process objects,
 * clears the list, and frees the memory allocated for the input line.
 *
 * @param process_list A pointer to a list of Process pointers to be cleaned up.
 * @param input_line A pointer to the dynamically allocated memory for user
 * input. This memory is freed to avoid memory leaks.
 */
void cleanup(list<Process *> &process_list, char *input_line) {
  for (Process *p : process_list) {
    delete p;
  }
  process_list.clear();
  free(input_line);
  input_line = nullptr;
}

//----- done -----
/**
 * @brief Main loop for the shell, facilitating user interaction and command
 * execution.
 *
 * This function initializes a list to store Process objects, reads user input
 * in a loop, and executes the entered commands. The loop continues until the
 * user decides to quit.
 *
 * @note The function integrates several essential components:
 *   1. Displaying the shell prompt to the user.
 *   2. Reading user input using the read_input function.
 *   3. Parsing the input into a list of Process objects using parse_input.
 *   4. Executing the commands using run_commands.
 *   5. Cleaning up allocated resources to prevent memory leaks with the cleanup function.
 *   6. Breaking out of the loop if the user enters the quit command.
 *   7. Continuously prompting the user for new commands until an exit condition is met.
 *
 * @warning
 *  Initialize Necessary Variables:
 *      Declare variables for storing the list of Process objects, the input
 * line, and a flag for quitting. Read User Input in a Loop: Utilize a while
 * loop to continuously read user input until a termination condition is met.
 * Check for a valid input line and skip empty lines. Clean Up Resources: After
 * executing the commands, clean up allocated resources using the cleanup
 *      function to avoid memory leaks.
 *  Check for Quit Command:
 *      Determine if the user entered the quit command. If so, set the exit flag
 * to true and break out of the loop. Repeat the Process: If the user did not
 * quit, continue prompting for input by displaying the prompt again.
 *  Considerations:
 *      Handle edge cases such as empty input lines or unexpected errors
 * gracefully. Ensure proper error handling and informative messages for the
 * user.
 */
void run() {
  list<Process *> process_list;
  char *input_line;
  bool is_quit = false;

  //keep asking for input and running commands until user specifies quit
  while(!is_quit){
    display_prompt();

    //read input
    input_line = read_input();

    //making sure input is not empty
    if(input_line == nullptr){
      break;
    }

    //clean newline
    sanitize(input_line);

    //parse input, split input into separate process objects
    parse_input(input_line, process_list);

    //run all of the processes in the list, will return true if one of the processes is quit.
    is_quit = run_commands(process_list);

    //free memory
    cleanup(process_list, input_line);
  }

}

//----- done -----
/**
 * (Do not modify)
 * @brief Reads input from the standard input (stdin) in chunks and dynamically
 *        allocates memory to store the entire input.
 *
 * This function reads input from the standard input or file in chunks of size
 * MAX_LINE. It dynamically allocates memory to store the entire input, resizing
 * the memory as needed. The input is stored as a null-terminated string. The
 * function continues reading until a newline character is encountered or an
 * error occurs.
 *
 * @return A pointer to the dynamically allocated memory containing the input
 * string. The caller is responsible for freeing this memory when it is no
 * longer needed. If an error occurs or EOF is reached during input, the
 * function returns NULL.
 *
 * @warning Ensure that the memory allocated by this function is freed using
 * free() to avoid memory leaks.
 */
char *read_input() {
  char *input = NULL;
  char tempbuf[MAX_LINE];
  size_t inputlen = 0, templen = 0;

  do {
    char *a = fgets(tempbuf, MAX_LINE, stdin);
    if (a == nullptr) {
      return input;
    }
    templen = strlen(tempbuf);
    input = (char *)realloc(input, inputlen + templen + 1);
    strcpy(input + inputlen, tempbuf);
    inputlen += templen;
  } while (templen == MAX_LINE - 1 && tempbuf[MAX_LINE - 2] != '\n');

  return input;
}

//----- done -----
/**
 * @brief
 * removes the new line char of the end in cmd.
 */
void sanitize(char *cmd) {
  //if passed string is null do nothing
  if (cmd == nullptr){
     return;
  }

  size_t len = strlen(cmd);

  //empty string do nothing
  if (len == 0){
    return;
  }

  //removing carriage returns from the end of the string making sure there isn't more than one
  while (len > 0 && (cmd[len-1] == '\n' || cmd[len-1] == '\r')) {
      cmd[len-1] = '\0';
      len--;
  }
}

//----- done -----
/**
 * Parses the given command string and populates a list of Process objects.
 *
 * This function takes a command string and a reference to a list of Process
 * pointers. It tokenizes the command based on the delimiters "|; " and creates
 * a new Process object for each token. The created Process objects are added to
 * the provided process_list. Additionally, it sets pipe flags for each Process
 * based on the presence of pipe delimiters '|' in the original command string.
 *
 * @param cmd The command string to be parsed.
 * @param process_list A reference to a list of Process pointers where the
 * created Process objects will be stored.
 *
 * Hints for students:
 * - The 'delimiters' we need to handle are '|', ';', and '[space]'
 * - 'pipe_in_val' is a flag indicating whether the current Process should take
 *      input from a previous Process (1 if true, 0 if false).
 * - 'currProcess' is a pointer to the current Process being created and added
 *      to the list.
 * - When a delim is a space it calls currProcess->add_token()
 * - When the delimiter is a ";" or "|", a new Process object is created with
 *       relevant information, and the pipe flags are set based on the presence
 *       of '|' in the original command.
 * - The created Process objects are added to the process_list.
 */
void parse_input(char *cmd, list<Process *> &process_list) {

  Process *currProcess = nullptr;

  char tempToken[100];

  int idx = 0;

  //used to track if the current process we are examining has a pipe in or pipe out
  bool pipe_in_flag = false;
  bool pipe_out_flag = false;

  //used to distinguish if we are currently looking for a command or flags for a command. This will
  //affect how we handle creating new processes.
  bool srch_for_cmd = true;
  bool srch_for_flg = false;

  //used to check if we have found a usable character and not just leading spaces.
  bool tkn_found = false;

  //going through each char in the input line passed.
  for (char *p = cmd;; p++) {
    //end of file or end of input string
    if(*p == '\0'){
      //making sure we have usable input and not just spaces
      if(tkn_found){
        //make sure to terminate our current command or flag
        tempToken[idx] = '\0';
        //if we do not already have a process created
        if (!currProcess) {
          currProcess = new Process(pipe_in_flag, pipe_out_flag);
        }
        //adding our token to the current process
        currProcess->add_token(tempToken);
        //resetting index and status flags
        idx = 0;
        tkn_found = false;
      }
      //pushing process to command list. If we had no usable input while we encountered /0 then we 
      //jump here and push the process without adding a token. Similary if we had no new process and
      //no usable input then we will just break out of the loop.
      if(currProcess){
        process_list.push_back(currProcess);
        currProcess = nullptr;
      }
      break;
    }
    else if(*p == ' ' || *p == '\n' || *p == '\r'){
      if(tkn_found){
        tempToken[idx] = '\0';
        if (!currProcess) {
          currProcess = new Process(pipe_in_flag, pipe_out_flag);
        }
        currProcess->add_token(tempToken);
        idx = 0;
        tkn_found = false;
        //if we hit a space while looking for command we want to make sure that we pick up any flags
        //that the user added, so we have to set our search flags accordingly.
        if(srch_for_cmd){
          currProcess = new Process(pipe_in_flag, pipe_out_flag);
          srch_for_cmd = false;
          srch_for_flg = true;
        }
      }
    }
    else if(*p == ';'){
      if(tkn_found){
        tempToken[idx] = '\0';
        if (!currProcess) {
          currProcess = new Process(pipe_in_flag, pipe_out_flag);
        }
        currProcess->add_token(tempToken);
        idx = 0;
        tkn_found = false;
      }
      if(currProcess){
        process_list.push_back(currProcess);
        currProcess = nullptr;
      }
      srch_for_cmd = true;
      srch_for_flg = false;
      pipe_in_flag = false;
    }
    // we have to worry about our pipe flags here, making sure to set pipe out to true and pipe in to true
    else if(*p == '|'){
      if(tkn_found){
        tempToken[idx] = '\0';
        if (!currProcess) {
          currProcess = new Process(pipe_in_flag, pipe_out_flag);
        }
        currProcess->add_token(tempToken);
        idx = 0;
        tkn_found = false;
      }
      if(currProcess){
        currProcess->pipe_out = true;
        process_list.push_back(currProcess);
        currProcess = nullptr;
      }
      srch_for_cmd = true;
      srch_for_flg = false;
      pipe_in_flag = true;
    }
    //here we found a usable ascii character that we want to record as part of a command or flag.
    else{
      tempToken[idx++] = *p;
      tkn_found = true;
      if(srch_for_cmd){
        if (!currProcess) {
          currProcess = new Process(pipe_in_flag, pipe_out_flag);
        }
        srch_for_cmd = false;
        srch_for_flg = true;
      }
    }
  }
}


//----- done -----
/**
 * Check if the given command represents a quit request.
 *
 * This function compares the first token of the provided command with the
 * string "quit" to determine if the command is a quit request.
 *
 * Parameters:
 *   - p: A pointer to a Process structure representing the command.
 *
 * Returns:
 *   - true if the command is a quit request (the first token is "quit").
 *   - false otherwise.
 */
bool isQuit(Process *p) {
  string firstCmd = p->cmdTokens[0];
  if (firstCmd == "quit") {
      return true;
  }
  return false;
}

//----- done -----
/**
 * @brief Execute a list of commands using processes and pipes.
 *
 * This function takes a list of processes and executes them sequentially,
 * connecting their input and output through pipes if needed. It handles forking
 * processes, creating pipes, and waiting for child processes to finish.
 *
 * @param command_list A list of Process pointers representing the commands to
 * execute. Each Process object contains information about the command, such as
 *                     command tokens, pipe settings, and file descriptors.
 *
 * @return A boolean indicating whether a quit command was encountered during
 * execution. If true, the execution was terminated prematurely due to a quit
 * command; otherwise, false.
 *
 * @details
 * The function iterates through the provided list of processes and performs the
 * following steps:
 * 1. Check if a quit command is encountered. If yes, terminate execution.
 * 2. Create pipes and fork a child process for each command.
 * 3. In the parent process, close unused pipes, wait for child processes to
 * finish if necessary, and continue to the next command.
 * 4. In the child process, set up pipes for input and output, execute the
 * command using execvp, and handle errors if the command is invalid.
 * 5. Cleanup final process and wait for all child processes to finish.
 *
 * @note
 * - The function uses Process objects, which contain information about the
 * command and pipe settings.
 * - It handles sequential execution of commands, considering pipe connections
 * between them.
 * - The function exits with an error message if execvp fails to execute the
 * command.
 * - Make sure to properly manage file descriptors, close unused pipes, and wait
 * for child processes.
 * - The function returns true if a quit command is encountered during
 * execution; otherwise, false.
 *
 * @warning
 * - Ensure that the Process class is properly implemented and contains
 * necessary information about the command, such as command tokens and pipe
 * settings.
 * - The function relies on proper implementation of the isQuit function for
 * detecting quit commands.
 * - Students should understand the basics of forking, pipes, and process
 * execution in Unix-like systems.
 */
bool run_commands(list<Process *> &command_list) {
  bool is_quit = false;

  //not used in our implementation.
  int max_process = 0;
  int min_process = 0;

  int size = command_list.size();

  //array for tracking child process ids
  pid_t pids[size];

  Process *prev = nullptr;

  int idx = 0;
  
  //parsing through the entire process list
  for (const auto& process : command_list) {
    //if the current process is quit then break out of the loop and set is_quit flag
    if(isQuit(process)){
        is_quit = true;
        break; 
    }

    //if the process has a pipe out then we need to make sure the next process has access to that pipe
    //we pipe here before forking so that both child processes will have access to the pipe file descriptors.
    if(process->pipe_out){
        if(pipe(process->pipe_fd) < 0){
            cerr << "tsh: error in piping" << endl;
            is_quit = true;
            break;
        }
    }

    pid_t c_pid = fork();
    
    //error in forking
    if (c_pid == -1){
        cerr << "tsh: error in running command: " << process->cmdTokens[0] << endl;
        is_quit = true;
        break;
    }

    //parent process (our shell)
    else if (c_pid > 0){
        //add the childs process id to the pid list
        pids[idx++] = c_pid;

        //parent closes previous pipe after child is forked
        if (prev != nullptr && prev->pipe_out) {
            close(prev->pipe_fd[0]);
            close(prev->pipe_fd[1]);
        }

        //if there is no pipe out that means our command should execute before other commands so in the parent
        //we want to immediately wait for that child to process.
        if (!process->pipe_out) {
          int status;
          waitpid(c_pid, &status, 0);
          //make sure that the next process id will overwrite this process id in the pid list so we dont double wait
          idx--;
        }

        //update previous process
        prev = process;
    }

    //child process (our command)
    else {
      //if the command gets input from a pipe need to redirect it to STDIN
      if (process->pipe_in && prev != nullptr) {
          dup2(prev->pipe_fd[0], STDIN_FILENO);
      }

      //if the command send input through a pipe to another command need to redirect it to STDOUT
      if (process->pipe_out) {
          dup2(process->pipe_fd[1], STDOUT_FILENO);
      }

      //no longer need access to any inhereted pipes so close all open file descriptors for them
      if (prev != nullptr && prev->pipe_out) {
          close(prev->pipe_fd[0]);
          close(prev->pipe_fd[1]);
      }

      if (process->pipe_out) {
          close(process->pipe_fd[0]);
          close(process->pipe_fd[1]);
      }

      //overwrite current process with code from specified command.
      execvp(process->cmdTokens[0], process->cmdTokens);

      //wont reach this unless error in execvp
      perror(process->cmdTokens[0]);
      exit(1);
  }
}

  //parent closes the last pipe if needed
  if (prev != nullptr && prev->pipe_out) {
      close(prev->pipe_fd[0]);
      close(prev->pipe_fd[1]);
  }

  //wait for all children
  for (int i = 0; i < idx; ++i) {
      int status;
      waitpid(pids[i], &status, 0);
  }

  return is_quit;
}

//----- done -----
/**
 * @brief Constructor for Process class.
 *
 * Initializes a Process object with the provided plags
 *
 * @param _pipe_in 1: The process takes input form prev, 0: if not
 * @param _pipe_out 1: The output of current proches is piped to next, 0: if not
 */
Process::Process(bool _pipe_in_flag, bool _pipe_out_flag) {
  pipe_in = _pipe_in_flag;
  pipe_out = _pipe_out_flag;
  tok_index = 0;
}

//----- done -----
/**
 * @brief Destructor for Process class.
 */
Process::~Process() {}

//----- done -----
/**
 * @brief add a pointer to a command or flags to cmdTokens
 *
 * @param tok
 */
void Process::add_token(char *tok) {
  //buffer
  cmdTokens[tok_index] = new char[strlen(tok) + 1];
  strcpy(cmdTokens[tok_index], tok);
  //increase index for next token
  tok_index++;
  //terminate our list
  cmdTokens[tok_index] = nullptr;
}

/* debug tools

//----- FOR DEBUGGING -----//
for (int i = 0; i < process->tok_index; i++) {
cout << process->cmdTokens[i] << " ";
}
cout << "pipe in: " << process->pipe_in << " " << "pipe out: " << process->pipe_out << " ";
cout << endl;
//----- END FOR DEBUGGING -----//

//----- FOR DEBUG -----/
//print input
for (const auto& process : process_list) {
if(isQuit(process)){
return;
}
for (int i = 0; i < process->tok_index; i++) {
cout << process->cmdTokens[i] << " ";
}
cout << "pipe in: " << process->pipe_in << " " << "pipe out: " << process->pipe_out << " ";
cout << endl;
}
//----- END FOR DEBUG -----/

*/

