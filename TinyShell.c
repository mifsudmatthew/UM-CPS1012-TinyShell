#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#define MAX_INPUT 100

pid_t fork_exec(char **args){ // Defining a new function to perform fork-exec

    pid_t forkPID = fork(); // Creating a copy of the process

    if(forkPID==-1){ // If an error is encountered

        perror("Unable to create new process!\n"); // Outputting error message
        return -1;

    }else if(forkPID==0){ // Child process

        if(execvp(args[0],args)==-1){ // Replacing the current process image with a new process image, according to the inputted arguments
        // If '-1' is returned from 'execvp' then the following is executed:
            perror("Unable to execute program!"); // Outputting error message
            return -2;
        }
    }

    int status;
    wait(&status);

    return forkPID; // Returning the PID
}

pid_t fork_exec_pipe(char **args, int pipe_fd_in[2], int pipe_fd_out[2]){

    pid_t forkPID = fork(); // Creating a copy of the process

    if(forkPID==-1){ // If an error is encountered

        perror("Unable to create new process!"); // Outputting error message
        return -1;

    }else if(forkPID==0){ // Child process

        // Input pipe
        if(pipe_fd_in!=NULL && pipe_fd_in[0]>=0 && pipe_fd_in[1]>=0){ // Valid pipe descriptors
            close(pipe_fd_in[1]); // Closing write end of the input pipe
            dup2(pipe_fd_in[0], STDIN_FILENO); // Making STDIN FD point to input pipe read end
            close(pipe_fd_in[0]); // Closing the read end of the input pipe
        }

        // Output pipe
        if(pipe_fd_out!=NULL&& pipe_fd_out[0]>=0 && pipe_fd_out[1]>=0){ // Valid pipe descriptors
            close(pipe_fd_out[0]); // Closing the read end of the output pipe
            dup2(pipe_fd_out[1], STDOUT_FILENO); // Making STDOUT FD point to output pipe write end
            close(pipe_fd_out[1]); // Closing write end of the output pipe
        }

        if(execvpe(args[0],args,NULL)==-1){ // Replacing the current process image with a new process image, according to the inputted arguments
        // If '-1' is returned from 'execvp' then the following is executed:
            perror("Unable to execute program!"); // Outputting error message
            return -2;
        }
    }

    int status;
    wait(&status);

    return forkPID; // Returning the PID
}

int execute_pipeline(char **pipeline[]){

    int pipelineStage = 0; // Creating a variable to keep track of the current pipeline stage

    while(pipeline[pipelineStage]!=NULL){ // Looping through all pipeline stages
        pipelineStage++; // Incrementing to the next stage
    }

    int fds[pipelineStage-1][2]; // Creating an array to store, each pipe object and the pipe file descriptors

    for(int i=0; i<pipelineStage-1; i++){ // Looping for as many times, as there are pipe objects
        if(pipe(fds[i])==-1){ // Creating a pipe
            // If '-1' is returned from 'pipe' then the following is executed:
            perror("Cannot create pipe!"); // Outputting error message
            return -3;  
        }
    }

    pid_t forkPID[pipelineStage]; // Creating an array to store the PID of each program

    for(int i=0; i<pipelineStage; i++){ // Looping through every stage

        char **args = pipeline[i]; // Obtaining current argument

        int *pipe_fd_in; // Creating a variable to represent an input pipe
        int *pipe_fd_out; // Creating a variable to represent an output pipe

        if(i==0){ // In the first stage (setting arguments for fork_exec_pipe)
            pipe_fd_in = NULL;
        }else{
            pipe_fd_in = fds[i-1];
        }

        if(i==pipelineStage-1){ // In the last stage (setting arguments for fork_exec_pipe)
            pipe_fd_out = NULL;
        }else{
            pipe_fd_out = fds[i];
        }

        forkPID[i] = fork_exec_pipe(args, pipe_fd_in, pipe_fd_out); // Running the 'fork_exec_pipe' function previously created
        close(fds[i][1]);

        if(forkPID[i]==-1){ // If an error is encountered
            printf("Program was unable to create a new process!\n"); // Outputting error message
            break;
        }else if(forkPID[i]==-2){
            printf("Program was unable to perform 'execvpe'\n"); // Outputting error message
            break;
        }else if(forkPID[i]==-3){
            printf("Program was unable to create pipe!\n"); // Outputting error message
            break;
        }
    }
    return forkPID[pipelineStage-1]; // Returning the PID
}

int execute_pipeline_async(char **pipeline[], bool async){

    int pipelineStage = 0; // Creating a variable to keep track of the current pipeline stage

    while(pipeline[pipelineStage]!=NULL){ // Looping through all pipeline stages
        pipelineStage++; // Incrementing to the next stage
    }

    int fds[pipelineStage-1][2]; // Creating an array to store, each pipe object and the pipe file descriptors

    for(int i=0; i<pipelineStage-1; i++){ // Looping for as many times, as there are pipe objects
        if(pipe(fds[i])==-1){ // Creating a pipe
            // If '-1' is returned from 'pipe' then the following is executed:
            perror("Cannot create pipe!"); // Outputting error message
            return -3;  
        }
    }

    pid_t forkPID[pipelineStage]; // Creating an array to store the PID of each program

    for(int i=0; i<pipelineStage; i++){ // Looping through every stage

        char **args = pipeline[i]; // Obtaining current argument

        int *pipe_fd_in = NULL; // Creating a variable to represent an input pipe
        int *pipe_fd_out = NULL; // Creating a variable to represent an output pipe

        if(i==0){ // In the first stage (setting arguments for fork_exec_pipe)
            pipe_fd_in = NULL;
        }else{
            pipe_fd_in = fds[i-1];
        }

        if(i==pipelineStage-1){ // In the last stage (setting arguments for fork_exec_pipe)
            pipe_fd_out = NULL;
        }else{
            pipe_fd_out = fds[i];
        }

        forkPID[i] = fork_exec_pipe(args, pipe_fd_in, pipe_fd_out); // Running the 'fork_exec_pipe' function previously created
        close(fds[i][1]);

        if(forkPID[i]==-1){ // If an error is encountered
            printf("Program was unable to create a new process!\n"); // Outputting error message
            break;
        }else if(forkPID[i]==-2){
            printf("Program was unable to perform 'execvpe'\n"); // Outputting error message
            break;
        }else if(forkPID[i]==-3){
            printf("Program was unable to create pipe!\n"); // Outputting error message
            break;
        }
    }
    if(!async){
        int status;
        waitpid(forkPID[pipelineStage-1], &status, 0); // Waiting for the last stage of the pipeline to complete
    }
    return forkPID[pipelineStage-1]; // Returning the PID
}

int fork_exec_pipe_ex(char **pipeline[], bool async, char *file_in, char *file_out, bool append_out){

    int pipelineStage = 0; // Creating a variable to keep track of the current pipeline stage
    
    while(pipeline[pipelineStage]!=NULL){ // Looping through all pipeline stages
        pipelineStage++; // Incrementing to the next stage
    }

    int fds[pipelineStage-1][2]; // Creating an array to store, each pipe object and the pipe file descriptors

    for(int i=0; i<pipelineStage-1; i++){ // Looping for as many times, as there are pipe objects
        if(pipe(fds[i])==-1){ // Creating a pipe
            // If '-1' is returned from 'pipe' then the following is executed:
            perror("Cannot create pipe!"); // Outputting error message
            return -3;  
        }
    }

    pid_t forkPID[pipelineStage]; // Creating an array to store the PID of each program

    int *pipe_fd_in = NULL; // Creating a variable to represent an input pipe
    int *pipe_fd_out = NULL; // Creating a variable to represent an output pipe

    for(int i=0; i<pipelineStage; i++){ // Looping through every stage

        char **args = pipeline[i]; // Obtaining current argument

    if(i==0){ // In the first stage (setting arguments for fork_exec_pipe)
            if(file_in!=NULL){ // If input file is specified
                FILE* fileToRead = freopen(file_in, "r", stdin);
                if(fileToRead==NULL){ // Checking for error
                    perror("Unable to redirect input to file!"); // Outputting error message
                    return -4;
                }
            }

        }else{
            pipe_fd_in=fds[i-1];
        }

        if(i==pipelineStage-1){ // In the last stage (setting arguments for fork_exec_pipe)
            if(file_out!=NULL){ // If output file is specified
                FILE* fileToWrite;
                if(append_out){ // Checking the boolean value of 'append_out'
                    fileToWrite = freopen(file_out, "a", stdout); // If 'append_out' is true, then output is appended to file.
                }else{
                    fileToWrite = freopen(file_out, "w", stdout); // If 'append_out' is true, then output to file is written afresh.
                }
                if(fileToWrite==NULL){ // Checking for error
                    perror("Unable to redirect output to file!"); // Outputting error message
                    return -5;
                }
            }
        }else{
            pipe_fd_out=fds[i];
        }

        forkPID[i] = fork_exec_pipe(args, pipe_fd_in, pipe_fd_out); // Running the 'fork_exec_pipe' function previously created
        close(fds[i][1]);

        if(forkPID[i]==-1){ // If an error is encountered
            printf("Program was unable to create a new process!\n"); // Outputting error message
            break;
        }else if(forkPID[i]==-2){
            printf("Program was unable to perform 'execvpe'\n"); // Outputting error message
            break;
        }else if(forkPID[i]==-3){
            printf("Program was unable to create pipe!\n"); // Outputting error message
            break;
        }
    }
    if(!async){
        int status;
        waitpid(forkPID[pipelineStage-1], &status, 0); // Waiting for the last stage of the pipeline to complete
    }
    return forkPID[pipelineStage-1]; // Returning the PID
}

typedef int(*builtin_t)(char**); // Defining the type of builtin commands.

struct builtin_command{ // Defining the structure for builtin commands.
    char *name;
    builtin_t method;
};

int builtin_exit(char **args){ // Implementing a builtin command 'exit'.
    exit(0);
}

int builtin_cd(char **args){ // Implementing a builtin command 'cd'

    if(args[1]==NULL){ // If there is no argument following 'cd'
        fprintf(stderr,"Error: Missing operand\n"); // Output error message
        return -7;
    }else{
        if(chdir(args[1])!=0){ // If changing directory has failed
            fprintf(stderr,"Error: Failed to change directory\n"); // Output error message
            return -8;
        }
    }
    return 0;
}

int builtin_cwd(){ // Implementing a builtin command 'cwd'

    char *CurrentWorkingDirectory = getenv("PWD"); // Obtaining current working directory and printing it to stdout
    if(CurrentWorkingDirectory!=NULL){ // If the directory is obtained
        printf("%s\n",CurrentWorkingDirectory); // The directory is outputted
    }else{ // Otherwise
        fprintf(stderr,"Error: Failed to 'getenv'"); // Output error message
        return -9;
    }
    return 0;
}

int builtin_ver(){ // Implementing a builtin command 'ver'
    printf("Tiny Shell v1.0\nAuthor: Matthew Mifsud\nAvailable Functions: exit, cd, cwd, ver\n");
    return 0;
}

struct builtin_command builtin_list[] = { // Defining a list of builtin commands.
    {"exit",&builtin_exit},{"cd",&builtin_cd},{"cwd",&builtin_cwd},{"ver",&builtin_ver}
};

int execute_builtin_command(char **args){

    char *input_string = args[0]; // Obtaining the opcode
    int builtinNum = sizeof(builtin_list) / sizeof(struct builtin_command); // Calculating the number of builtin commands available
    int input_length = strlen(input_string); // Obtaining the opcode length
    int name_length; // Creating a variable to store the length of a command accessed
    int min_length; // Creating a variable to store the length of the smallest between 'input_length' and 'name_length'
    int result; // Creating variable to store the return value of a function

    for(int n=0; n<builtinNum; n++){

       name_length = strlen(builtin_list[n].name); // Obtaining the length of the command currently being accessed

        // Setting the value of 'min_length' as the minimum of the lengths of the two input strings
       if(input_length<name_length){ // Checking if 'input_length' smaller than 'name_length'
        min_length = input_length; // Updating the value of 'min_length' to 'input_length'
       }else{ // If 'input_length' is larger than 'name_length'
        min_length = name_length; // Updating the value of 'min_length' to 'name_length'
       }
        if(strncmp(input_string, builtin_list[n].name, min_length)==0){ // If the inputted string matches the command in the list
            result = builtin_list[n].method(args); // Executing the associated method
            return result;
        }
    }

    return -6; // Builtin command not found

}

bool validityCheck(char *string){ // Creating a function to check for invalid syntax
    char *illegalChars = "#%&{}<>?/$!\':@+`|=*"; // Storing all illegal characters in a list
    int numOfIllegals = strlen(illegalChars); // Obtaining the number of illegal characters
    for(int i = 0; i<numOfIllegals; i++){ // Looping through all illegal characters
        if(strchr(string,illegalChars[i])!=NULL){ // Checking if a string contains the current illegal character being accessed
        // If there is a match
            return false;
        }
    }
    // Otherwise
    return true;
}


int tokenize(char *inputString){ // Creating a function to tokenize a line of text

    char *userInput = malloc(strlen(inputString)+1);
    strcpy(userInput,inputString);

    if(strlen(userInput)<1 || userInput[0]=='\n'){ // Checking if user inputted nothing
        printf("Error encountered: Nothing was inputted\n"); // Output error message
        exit(EXIT_FAILURE); // Stopping execution
    }
    
    char *tokenizedInput = strtok(userInput, " "); // Tokenizing input using " " as a delimiter
    char *previousToken = NULL; // Creating a variable to store the previous token
    
    while(tokenizedInput!=NULL){ // Looping until the last token
        
        if(strcmp(tokenizedInput,"|")==0){ // Checking if the token is a '|'
            if(previousToken == NULL) { // If the first character is the '|'
                fprintf(stderr,"Error: Pipeline operator cannot appear as the first or last token in a sequence.\n"); // Output error message
                free(userInput);
                return 1; // Stopping execution
            }else if(!validityCheck(strtok(NULL," "))){ // If the next token is invalid
                fprintf(stderr,"Error: Pipeline operator should always be followed by a valid filename.\n"); // Output error message
                free(userInput);
                return 1; // Stopping execution
            }

        }else if(strcmp(tokenizedInput,">")==0) { // Checking if the token is a '>'
            if(previousToken==NULL) { // If the first character is the '>'
                fprintf(stderr,"Error: Output redirection operator cannot appear as the first or last token in a sequence.\n"); // Output error message
                free(userInput); 
                return 1; // Stopping execution               

            }else if(!validityCheck(strtok(NULL," "))){ // If the next token is invalid
                fprintf(stderr,"Error: Output redirection operator should always be followed by a valid filename.\n"); // Output error message
                free(userInput);
                return 1; // Stopping execution
            }

        }else if(strcmp(tokenizedInput,">>")==0){ // Checking if the token is a '>>'
            if(previousToken==NULL) { // If the first character is the '>>'
                fprintf(stderr,"Error: Append output redirection operator cannot appear as the first or last token in a sequence.\n"); // Output error message
                free(userInput); 
                return 1; // Stopping execution               

            }else if(!validityCheck(strtok(NULL," "))){ // If the next token is invalid
                fprintf(stderr,"Error: Append output redirection operator should always be followed by a valid filename.\n"); // Output error message
                free(userInput);
                return 1; // Stopping execution
            }

        }else if(strcmp(tokenizedInput,"<") == 0){ // Checking if the token is a '<'
            if(previousToken==NULL) { // If the first character is the '<'
                fprintf(stderr,"Error: Input redirection operator cannot appear as the first or last token in a sequence.\n"); // Output error message
                free(userInput);

            }else if(!validityCheck(strtok(NULL," "))){ // If the next token is invalid
                fprintf(stderr,"Error: Input redirection operator should always be followed by a valid filename.\n"); // Output error message
                free(userInput);
                return 1; // Stopping execution
            }

        }else if(strchr(tokenizedInput, '|') || strchr(tokenizedInput, '>') || strstr(tokenizedInput, ">>") || strchr(tokenizedInput, '<')){
            fprintf(stderr,"Error: Metacharacters should always be followed by a valid argument.\n");
            free(userInput);
            return 1; // Stopping execution

        }else if(!validityCheck(tokenizedInput)){ // Checking if filename is not valid
            fprintf(stderr,"Error: Filename is not valid.\n");
            free(userInput);
            return 1; // Stopping execution
        }
        previousToken = tokenizedInput; // Saving the current token before accessing the next token
        tokenizedInput = strtok(NULL, " "); // Accessing the next token
    }

    free(userInput);

    return 0;

}

char * arguments[MAX_INPUT];

void obtainArgs(char *inputString){ // Creating a function to split each argument in an array of strings

    char *command = malloc(strlen(inputString)+1); // Allocating memory for the command(s) inputted
    strcpy(command, inputString); // Storing the command(s) inputted in the allocated memory

    int i = 0;
    int length = 0;
    bool insideQuote = false; // Creating a variable to keep track if we are currently inside quoted string
    bool isFirstTokenInQuote = false; // Creating a variable to keep track if the current token is the first one in a quoted string

    char *tokenizedInput = strtok(command, " "); // Tokenizing input string
    
    while (tokenizedInput!=NULL){ // Looping through all tokens

        if(tokenizedInput[0]=='\"'){ // Checking if token starts with double quote
            insideQuote = true; // Updating 'insideQuote' variable
            isFirstTokenInQuote = true;
            tokenizedInput++; // Moving to the next character after the quote
            
            // Check if the first character after the opening quote is a backslash
            if (tokenizedInput[0] == '\\') {
                isFirstTokenInQuote = false;
                tokenizedInput++; // Move past the backslash
                
                // Check if the next character is also a backslash
                if (tokenizedInput[0] == '\\') {
                    isFirstTokenInQuote = true;
                    tokenizedInput++; // Move past the second backslash
                }
            }
        }

        if(insideQuote){ // If a token was encountered

            length = strlen(tokenizedInput); // Obtaining the length of the tokenized input

            if (length > 0 && tokenizedInput[length-1]=='\"' && tokenizedInput[length-2]!='\\') { // If the final token ends with a double quote (that is not escaped)
                tokenizedInput[length-1]='\0'; // The double quote is replaced with a null terminator
                insideQuote = false; // Updating the 'insideQuote' variable
            }
            
            // If this is the first token in a quoted string and it starts with a backslash, add the backslash to the token
            if (isFirstTokenInQuote && tokenizedInput[0] == '\\') {
                arguments[i++] = "\\"; // Add the backslash as a separate token
                tokenizedInput++; // Move past the backslash
                
                // Check if the next character is also a backslash
                if (tokenizedInput[0] == '\\') {
                    arguments[i++] = "\\"; // Add the second backslash as a separate token
                    tokenizedInput++; // Move past the second backslash
                }
            }

            arguments[i++] = tokenizedInput; // Storing quoted string in the variable 'arguments'

        }else{ // If not inside a quoted string
            arguments[i++] = tokenizedInput; // Storing quoted string in the variable 'arguments'
        }
        
        if(tokenizedInput[strlen(tokenizedInput)-1]=='\"' && tokenizedInput[strlen(tokenizedInput)-2]!='\\' && 
        tokenizedInput[strlen(tokenizedInput)-3]!='\\' && !insideQuote){ // Checking for end double quote 
            fprintf(stderr,"Error: Missing opening double quote\n"); // Output error message
            exit(EXIT_FAILURE);
        }

        tokenizedInput = strtok(NULL, " "); // Obtaining the next token, using " " as a delimiter
    }

    arguments[i] = NULL; // Terminating the 'arguments' list with NULL

    if (insideQuote){ // If no double quote was found at
        fprintf(stderr,"Error: Missing closing double quote\n"); // Output error message
        exit(EXIT_FAILURE);
    }
}

void execute_shell_command(char* command){ // Creating a function to execute both builtin and external commands

char *command_pipe[MAX_INPUT];

int tokenResult = tokenize(command); // Tokenizing the user's input, to check if it is in a correct format
obtainArgs(command); // Obtaining all tokens with the delimiter being " "

  if(arguments!=NULL && tokenResult==0){ // If the user inputted some arguments in a correct format 

    if(execute_builtin_command(arguments)==-6){  // We first try to execute a builtin command. If the input does not match a builtin:

      bool pipeFound = false; // Creating a variable to check if there are pipes, meaning multiple commands
      bool append = false; // Creating a variable to check if ">>" was used for append output redirection
      int commandIndex = 0; // Creating a variable to keep track of the command index
      char *commandList[MAX_INPUT]; // Creating a variable to store the commands which will be executed
      int count = 0; // Creating a variable to store the number of commands
      char *outFile = NULL; // Creating a variable to store the name of the file use for output redirection.
      char *inFile = NULL; // Creating a variable to store the name of the file use for input redirection.

      for(int i = 0; arguments[i]!=NULL; i++){ // Looping through all arguments

        if(strcmp(arguments[i],"|")==0){ // If a '|' is encountered

          pipeFound = true; // Updating the 'pipeFound' variable

          command_pipe[commandIndex] = strdup(commandList[0]); // Storing the command in the pipe
          count=0; // Resetting 'count' to 0
          commandIndex++; // Increasing the index for the command pipeline

        }else if(strcmp(arguments[i],">")==0){ // If a '>' is encountered

          append = false; // Updating the 'append' variable, such that output is not appended
          i++; // Incrementing, to select the next element in the array 'arguments'

          if(arguments[i]!=NULL){ // Checking if there is a next element
            outFile = arguments[i]; // Storing the name of the file 
          }

          for(int j = i; j>0; j--){ // Removing > and filename from 'arguments' such that the command is only left
            arguments[j] = NULL;
          }

        }else if(strcmp(arguments[i],">>")==0){ // If a '>>' is encountered

          append = true; // Updating the 'append' variable, such that output is appended
          i++; // Incrementing, to select the next element in the array 'arguments'

          if(arguments[i]!=NULL){ // Checking if there is a next element
            outFile = arguments[i]; // Storing the name of the file 
          }
          
          for(int j = i; j>0; j--){ // Removing '>>' and filename from arguments such that the command is only left
            arguments[j] = NULL;
          }

        }else if(strcmp(arguments[i],"<")==0){ // If a '<' is encountered

          i++; // Incrementing, to select the next element in the array 'arguments'

          if(arguments[i]!=NULL){ // Checking if there is a next element
            inFile = arguments[i]; // Storing the name of the file 
            for(int j = i; j>0; j--){ // Cleaning from < and filename
            arguments[j] = NULL; // Removing '<' and filename from arguments such that the command is only left
            }
          }

        }else{ // If any other argument is encountered
          commandList[count] = arguments[i]; // Storing the argument in 'commandList'
          count++; // Incrementing 'count'
        }

      }

      commandList[count] = NULL; // Terminating 'commandList' with NULL
      command_pipe[commandIndex] = strdup(commandList[0]);

      if(!pipeFound){ // If there are no pipes, meaning one command

        char **pipeline[] ={arguments,NULL}; // Filling pipeline
        fork_exec_pipe_ex(pipeline,true,inFile,outFile,append); // Executing command

      }else{ // Otherwise

        int pipelineCount = commandIndex + 1; // Obtaining last index of pipeline
        char **pipeline[pipelineCount+1]; // Creating a pipeline

        for(int i = 0; i < pipelineCount; i++){ // Looping through pipeline
          pipeline[i] = malloc(MAX_INPUT *sizeof(char*)); // Allocating memory for a command
          pipeline[i][0] = command_pipe[i]; // Storing the command obtained from user in pipeline
          pipeline[i][1] = NULL; // Terminating the command with a NULL
        }

        pipeline[pipelineCount] = NULL; // Terminating pipeline with NULL
        fork_exec_pipe_ex(pipeline,true,inFile,outFile,append); // Executing commands

      }
    }   
  }
}

int main(){
    char input[MAX_INPUT]; // Creating a variable to store user input

    printf("TinyShell>$ "); // Prompt to show user that shell is waiting for input

    if(fgets(input, MAX_INPUT, stdin)==NULL){ // Reading input from user
        printf("Error in reading input.\n"); // Output error message
        exit(EXIT_FAILURE);
    }

    input[strcspn(input, "\n")] = '\0'; // Removing the newline character from the inputted string

    execute_shell_command(input); // Executing the command(s) inputted

    exit(EXIT_SUCCESS);
}