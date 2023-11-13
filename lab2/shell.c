#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>

//+
// File:	shell.c
//
// Pupose:	This program implements a simple shell program. It does not start
//		processes at this point in time. However, it will change directory
//		and list the contents of the current directory.
//
//		The commands are:
//		   cd name -> change to directory name, print an error if the directory doesn't exist.
//		              If there is no parameter, then change to the home directory.
//		   ls -> list the entries in the current directory.
//			      If no arguments, then ignores entries starting with .
//			      If -a then all entries
//		   pwd -> print the current directory.
//		   exit -> exit the shell (default exit value 0)
//				any argument must be numeric and is the exit value
//
//		if the command is not recognized an error is printed.
//-

#define CMD_BUFFSIZE 1024
#define MAXARGS 10

int splitCommandLine(char * commandBuffer, char* args[], int maxargs);
int doInternalCommand(char * args[], int nargs);
int doProgram(char * args[], int nargs);

//+
// Function:	main
//
// Purpose:	The main function. Contains the read
//		eval print loop for the shell.
//
// Parameters:	(none)
//
// Returns:	integer (exit status of shell)
//-

int main() {

    char commandBuffer[CMD_BUFFSIZE];
    // note the plus one, allows for an extra null
    char *args[MAXARGS+1];

    // print prompt.. fflush is needed because
    // stdout is line buffered, and won't
    // write to terminal until newline
    printf("%%> ");
    fflush(stdout);

    while(fgets(commandBuffer,CMD_BUFFSIZE,stdin) != NULL){
        //printf("%s",commandBuffer);

	// remove newline at end of buffer
	int cmdLen = strlen(commandBuffer);
	if (commandBuffer[cmdLen-1] == '\n'){
	    commandBuffer[cmdLen-1] = '\0';
	    cmdLen--;
            //printf("<%s>\n",commandBuffer);
	}

	// split command line into words.(Step 2)
	// Num of words stored in nargs
    int nargs = splitCommandLine(commandBuffer, args, MAXARGS);

	// add a null to end of array (Step 2)
	//End of list indaced with a null.
    args[nargs] = NULL;

    /*
	//debugging
	printf("%d\n", nargs);
	int i;
	for (i = 0; i < nargs; i++){
	   printf("%d: %s\n",i,args[i]);
	}
	// element just past nargs
	printf("%d: %s\n",i, args[i]);
    //End of debugging
    */

        //Check if 1 or more args (Step 3)
        //If one or more args, call doInternalCommand  (Step 3)
        //If doInternalCommand returns 0, call doProgram  (Step 4)
        //If doProgram returns 0, print error message (Step 3 & 4)
        //that the command was not found.
        if (nargs >= 1){
            if (doInternalCommand(args, nargs) == 0){
                if (doProgram(args, nargs) == 0){
                    printf("Error: Command not found.\n");
                }
            }
        } 
        
	// print prompt
	printf("%%> ");
	fflush(stdout);
    }
    return 0;
}

////////////////////////////// String Handling (Step 1) ///////////////////////////////////

//+
// Function:	skipChar
//
// Purpose:	This function skips over a given char in a string
//		For security, will not skip null chars.
//
// Parameters:
//    charPtr	Pointer to string
//    skip	character to skip
//
// Returns:	Pointer to first character after skipped chars
//		ID function if the string doesn't start with skip,
//		or skip is the null character
//-

char * skipChar(char * charPtr, char skip){
    //Iterate through and only skip if pointing to a space character and if the character is not a null.
    while (*charPtr == skip && *charPtr != '\0'){
        charPtr++;
    }
    //Return the pointer to the immediate character after the skip.
    return charPtr;
}

//+
// Funtion:	splitCommandLine
//
// Purpose:	
//      Takes the user command input and splits it up into individual words. 
//      Pointers to the beggining of each word are then stored, and the total number of words is returned.
//
// Parameters: 
//      commandBuffer: Input string by the user signifying a command needing to be split up.
//      args[]: Array used to store the pointer to the beggining of each word.
//      maxargs: The maximum number of arguments allowed for the input.
//
// Returns:	Number of arguments (< maxargs).
//
//-

int splitCommandLine(char * commandBuffer, char* args[], int maxargs){

    char *curr = commandBuffer;
    int nargs = 0;

    //Iterate the command buffer for all characters that aren't null.
    while (*curr != '\0'){
        //Skip spaces to get beggining of word. 
        curr = skipChar(curr, ' ');

        //Break out if a null character has been reached, indicating the end of word.
        if (*curr == '\0')
            break;
        //Otherwise, store in the args array a pointer to the start of the word, and incrament to the next argument.
        args[nargs] = curr;
        nargs++;

        //Iterate until the next space or null, indicating the next word.
        while (*curr != ' ' && *curr != '\0'){
            curr++;
        }

        //Add a null to indicate the end of word, if no null present yet.
        if (*curr != '\0') {
            *curr = '\0';
            curr++;
        }

        //Return error if too many arguments.
        if (maxargs < nargs) {
            fprintf(stderr, "More arguments in string than the max number of arguments.\n");
            return 0;
        }
    }
    //Return the number of words.
    return nargs;
}

////////////////////////////// External Program  (Note this is step 4, complete doeInternalCommand first!!) ///////////////////////////////////

// list of directorys to check for command.
// terminated by null value
char * path[] = {
    ".",
    "/usr/bin",
    NULL
};

//+
// Funtion:	doProgram
//
// Purpose:
//      Iterate through path to search for inputed argument, and construct a path to this executable, if possible.
//      If all conditions are met (regular and executable file), fork and execute the program.
//
// Parameters:
//      args[]: The array that stores the user inputted arguments.
//      nargs: Number of arguments stored within the array.
//
// Returns	int
//		1 = found and executed the file
//		0 = could not find and execute the file
//-

int doProgram(char * args[], int nargs){
  // find the executable
  // Note this is step 4, complete doInternalCommand first!!!

    //Iterate through path[].
    for (int i = 0; path[i] != NULL; i++) {
        //Declare pointer to the path, and allocate enough memory depending on the length of the path, accounting for '/' and null.
        char *cmd_path = (char*)malloc(strlen(path[i]) + strlen(args[0]) + 2);
        
        //Check for error while allocating memory.
        if (cmd_path == NULL) {
            fprintf(stderr, "Error: Unable to allocate memory.\n");
            exit(1);
        }

        //Filling in the cmd_path string with appropriate arguments.
        sprintf(cmd_path, "%s/%s", path[i], args[0]);

        struct stat st;
        //Checking if a regular file, and checking if the file exists (0 returned by stat()).
        if (stat(cmd_path, &st) == 0 && S_ISREG(st.st_mode)){
            //Executable file if st.st_mode bit-and S_IXUSR is non-zero.
            if (st.st_mode & S_IXUSR) {
                //Fork into child process.
                pid_t child = fork();
                //Error case for not being ble to create the child proccess. 
                if (child < 0){
                    fprintf(stderr, "Error: Child proccess could not be created.\n");
                    free(cmd_path);
                    return 0;
                }
                //Proccess forked is the child.
                else if (child == 0){
                    //Execute the proccess.
                    execv(cmd_path, args);
                    //Execv should not return, but if it does, error has occurred. 
                    fprintf(stderr, "Error: Cannot execute child process.\n");
                    exit(1);
                } 
                //Parent proccess of child.
                else if (child > 0){
                    //Let the child process execute by waiting.
                    int status;
                    wait(&status);
                    //Free memory.
                    free(cmd_path);
                    //Executed successfully, return 1.
                    return 1;
                }
            }
        }
        //Free memory even if not successful.
        free(cmd_path);
    }
}

////////////////////////////// Internal Command Handling (Step 3) ///////////////////////////////////

// define command handling function pointer type
typedef void(*commandFunc)(char * args[], int nargs);

// associate a command name with a command handling function
struct cmdStruct{
   char 	* cmdName;
   commandFunc 	cmdFunc;
};

// prototypes for command handling functions
// add prototype for each comamand function
void cdFunc(char *args[], int nargs);
void lsFunc(char *args[], int nargs);
void pwdFunc(char *args[], int nargs);
void exitFunc(char *args[], int nargs);


// list commands and functions
// must be terminated by {NULL, NULL} 
// in a real shell, this would be a hashtable.
struct cmdStruct commands[] = {
   //add entry for each command
    {"cd", cdFunc},
    {"ls", lsFunc},
    {"pwd", pwdFunc},
    {"exit", exitFunc},
    { NULL, NULL}		// terminator
};

//+
// Function:	doInternalCommand
//
// Purpose:
//      Function to check the validity of the user inputted arguments. 
//      If the argument is a valid internal command as created earlier, the function to execute this command will be called.
//
// Parameters:
//      args[]: The array that stores the user inputted arguments.
//      nargs: Number of arguments stored within the array.
// Returns	int
//		1 = args[0] is an internal command
//		0 = args[0] is not an internal command
//-

int doInternalCommand(char * args[], int nargs){
    //Return if no arguments inputted.
    if (nargs < 1) {
        return 0;
    }
    //Iterate through list of created commands until the final null terminator is reached, indicating no more commands available.
    for (int i = 0; commands[i].cmdName != NULL; i++) {
        //Compare the inputted string, with one of the four created commands.
        //strcmp returns 0 if the two strings are equal.
        int validity = strcmp(args[0], commands[i].cmdName);
        if (validity == 0) {
            //If matching, run the associated command and return 1.
            commands[i].cmdFunc(args, nargs);
            return 1;
        }
    }
    //If not matching, return 0.
    return 0;
}

///////////////////////////////
// comand Handling Functions //
///////////////////////////////



//+
// Function:	cdFunc
//
// Purpose:
//      Function used to run the cd (change directory) command. See line-by-line comments for specifics. 
//
// Parameters:
//      args[]: The array that stores the user inputted arguments.
//      nargs: Number of arguments stored within the array.
// Returns	void
//-
void cdFunc(char *args[], int nargs){
    //Error case to account for too many arguments for the cd command (max 2).
    if (nargs > 2){
        fprintf(stderr, "Error: Too many args for cd command.\n");
        return;
    }
    //Obtain the home directory of the user.
    struct passwd *pw = getpwuid(getuid());
    //Print error if unable to get the home directory.
    if (pw == NULL){
        fprintf(stderr, "Error: Cannot retrieve home directory.\n");
        return;
    }
    int status;
    //If 1 argument inputted, change to home directory, and store the status of the chdir return.
    if (nargs == 1){
        status = chdir(pw->pw_dir);
    }
    //Otherwise, cd to the user inputted directory, and store the status of the chdir return.
    else{
        status = chdir(args[1]);
    }
    //Print error if cd unsuccessful. 
    if (status != 0) {
        fprintf(stderr, "Error: Unsuccesful.\n");
    }
}


//+
// Function:	filterFunc
//
// Purpose:
//      Helper function called by lsFunc used to filter out files to be printed when calling the ls command. 
//
// Parameters:
//      d: Pointer to the file within the directory.
// Returns	int
//		1 = The file should be included in the list.
//		0 = The file should be ommitted from the list.
//-
int filterFunc(const struct dirent * d){
    //Check if the name of the file does not begin with a dot, and return 1 if true, 0 otherwise.
    if (d->d_name[0] != '.'){
        return 1;
    }
    else{
        return 0;
    }
}

//+
// Function:	lsFunc
//
// Purpose:
//      Function used to run the ls (list files) command. See line-by-line comments for specifics. 
//
// Parameters:
//      args[]: The array that stores the user inputted arguments.
//      nargs: Number of arguments stored within the array.
// Returns	void
//-
void lsFunc(char *args[], int nargs){
    //Error case to account for too many arguments for the ls command (max 2).
    if (nargs > 2){
        fprintf(stderr, "Error: Too many args for ls command.\n");
        return;
    }
    //Declare array of pointers to struct dirent and integer for number of elements.
    struct dirent** namelist;
    int numEnts;

    //If command called with no supporting arguments, call filterFunc to exclude files starting with '.'
    if (nargs == 1){
        numEnts = scandir(".", &namelist, filterFunc, NULL);
    } 
    //If ls accompanied with another argument that is "-a", do not exclude files. 
    else if (nargs == 2 && strcmp(args[1], "-a") == 0){
        numEnts = scandir(".", &namelist, NULL, NULL);
    } 
    //Otherwise, print error due to incorrect usage.
    else{
        fprintf(stderr, "Error: Incorrect usage of ls command.\n");
        return;
    }

    //Iterate through and print out file names.
    for (int i = 0; i < numEnts; i++){
        printf("%s\n", namelist[i]->d_name);
    }
    //Free memory.
    free(namelist);
}


//+
// Function:	pwdFunc
//
// Purpose:
//      Function used to run the pwd (print working directory) command. See line-by-line comments for specifics. 
//
// Parameters:
//      args[]: The array that stores the user inputted arguments.
//      nargs: Number of arguments stored within the array.
// Returns	void
//-
void pwdFunc(char *args[], int nargs){
    //Error case to account for too many arguments for the pwd command (max 1).
    if (nargs > 1){
        fprintf(stderr, "Error: Too many args for pwd command.\n");
        return;
    }
    //Store a pointer to the buffer (current working directory).
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        fprintf(stderr, "Error: Cannot retrieve current working directory.\n");
    }
    else{
        //Print the working directory and free memory.
        printf("%s\n", cwd);
        free(cwd);
    }
}

//+
// Function:	exitFunc
//
// Purpose:
//      Function used to run the exit command. See line-by-line comments for specifics. 
//
// Parameters:
//      args[]: The array that stores the user inputted arguments.
//      nargs: Number of arguments stored within the array.
// Returns	void
//-
void exitFunc(char *args[], int nargs){
    //Error case to account for too many arguments for the exit command (max 2).
    if (nargs > 2){
        fprintf(stderr, "Error: Too many args for exit command.\n");
    }
    //If 2 arguments inputted, use the second argument as the exit code.
    else if (nargs == 2){
        //Exit with the user provided exit code. Covert the argument to an integer.
        exit(atoi(args[1]));
    }
    //Otherwise exit with code zero.
    else{
        exit(0);
    }
}
