#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include "pcb.h"
#include "scheduler.h"
#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 7;

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}


int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int source(char *script);
int echo(char *string);
int my_ls();
int my_mkdir(char *dirName);
int my_touch(char *fileName);
int my_cd(char *dirName);
int run(char *command_args[], int args_size);
int exec(char *filename1, char *filename2, char *filename3, char *policy);
int badcommandFileDoesNotExist();
int badcommandFolderDoesNotExist();

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
    int i;

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand();
    }

    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0) {
        //help
        if (args_size != 1)
            return badcommand();
        return help();

    } else if (strcmp(command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1)
            return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) {
        //set
        if (args_size != 3)
            return badcommand();
        return set(command_args[1], command_args[2]);

    } else if (strcmp(command_args[0], "print") == 0) {
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);

    } else if (strcmp(command_args[0], "source") == 0) {
        if (args_size != 2)
            return badcommand();
        return source(command_args[1]);

    } else if (strcmp(command_args[0], "echo") == 0) {
	    if (args_size != 2) {
	        return badcommand();
	    }
	    return echo(command_args[1]);
    } else if (strcmp(command_args[0], "my_ls") == 0) {
    	if (args_size != 1) {
	        return badcommand();
	    }
	    return my_ls();
    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
	    if (args_size != 2) {
	        return badcommand();
	    }
	    return my_mkdir(command_args[1]);
    } else if (strcmp(command_args[0], "my_touch") == 0) {
	    if (args_size != 2) {
	        return badcommand();
	    }
	    return my_touch(command_args[1]);
    } else if (strcmp(command_args[0], "my_cd") == 0) {
        if (args_size != 2) {
            return badcommand();
        }
        return my_cd(command_args[1]);
    } else if (strcmp(command_args[0], "run") == 0) {
        if (args_size <= 1) {
            return badcommand();
        }
        return run(command_args + 1, args_size - 1);
    } else if (strcmp(command_args[0], "exec") == 0) {
        if (args_size <= 1 || args_size > 5) {
            return badcommand();
        }
        if (args_size == 2) {
            return exec(command_args[1], NULL, NULL, "FCFS");
        }

        char *policy = command_args[args_size - 1];
        if (strcmp(policy, "FCFS") != 0 &&
            strcmp(policy, "SJF") != 0 &&
            strcmp(policy, "RR") != 0 &&
            strcmp(policy, "AGING") != 0 &&
            strcmp(policy, "RR30") != 0) {

            return badcommand();
        }

        if (args_size == 3) {
            return exec(command_args[1], NULL, NULL, policy);
        } else if (args_size == 4) {
            return exec(command_args[1], command_args[2], NULL, policy);
        } else {
            return exec(command_args[1], command_args[2], command_args[3], policy);
        }
    } else {
        printf("here\n");
        return badcommand();
    }
}

int help() {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n ";
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value) {
    // Challenge: allow setting VAR to the rest of the input line,
    // possibly including spaces.

    // Hint: Since "value" might contain multiple tokens, you'll need to loop
    // through them, concatenate each token to the buffer, and handle spacing
    // appropriately. Investigate how `strcat` works and how you can use it
    // effectively here.

    mem_set_value(var, value);
    return 0;
}


int print(char *var) {
    struct memory_return *result = mem_get_value(var);
    printf("%s\n", result->res);
    free(result->res);
    free(result);
    return 0;
}


int source(char *script) {
    int errCode = 0;

    FILE *p = fopen(script, "rt");      // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }

    struct pcb *pcb = init_pcb(p);
    fclose(p);

    run_scheduler(&pcb, 1, "FCFS");

    return errCode;
}

int echo(char *string) {
    int errCode = 0;
    if (string[0] == '$' && string[1] != '\0') {
        struct memory_return *result = mem_get_value(string + 1);
        if (result->status == -1) {
            printf("\n");
        } else {
            printf("%s\n", result->res);
        }
        free(result->res);
        free(result);
    } else {
	    printf("%s\n", string);
    }
    return errCode;
}

int badcommandFolderDoesNotExist() {
    printf("Bad command: Folder not found\n");
    return 3;
}

int compareNames(const void *a, const void *b) {
    const char *strA = *(const char **)a;
    const char *strB = *(const char **)b;

    return strcmp(strA, strB);
}


int my_ls() {
    int errCode = 0;
    DIR *dir = opendir(".");
    if (dir == NULL) {
        errCode = badcommandFolderDoesNotExist();
        return errCode;
    }

    char **result = NULL;
    struct dirent *entry;
    int count = 0;

    entry = readdir(dir);
    while (entry != NULL) {
        result = realloc(result, (count + 1) * sizeof(char *));
        result[count] = strdup(entry->d_name);
        count++;
        entry = readdir(dir);
    }

    closedir(dir);

    qsort(result, count, sizeof(char *), compareNames); 

    for (int i = 0; i < count; i++) { 
        printf("%s\n", result[i]);
        free(result[i]);
    }

    free(result);

    return errCode;
}

int alphaNumCheck(char *string) {
    int isAlphaNum = 1;
    for (int i = 0; i < strlen(string); i++) {
        if (!isalnum(string[i])) {
            isAlphaNum = 0;
            break;
        }
    }
    return isAlphaNum;
}

int badMyMkdirCommand() {
    printf("Bad command: my_mkdir\n");
    return 3;
}

void createDir(char *dirName) {
    mode_t perms = 0755;
    mkdir(dirName, perms);
}

int my_mkdir(char *dirName) {
    int errCode = 0;
    
    if (dirName[0] == '$') {
        struct memory_return *result = mem_get_value(dirName + 1);
        if (result->status != -1 && alphaNumCheck(result->res) == 1) {
            createDir(result->res);
        } else {
            errCode = badMyMkdirCommand();
        }
        free(result->res);
        free(result);
    } else {
	    createDir(dirName);
    }

    return errCode;
}

int my_touch(char *fileName) {
    int errCode = 0;
    if (alphaNumCheck(fileName) == 1) {
        FILE *f = fopen(fileName, "w");
        fclose(f);
        chmod(fileName, 0755);
    }
    return errCode;
}

int specialDirCheck(char *dirName) {
    return strcmp(dirName, ".") == 0 || strcmp(dirName, "..") == 0;
}

int badMyCdCommand() {
    printf("Bad command: my_cd\n");
    return 3;
}

int my_cd(char *dirName) {
    int errCode = 0;
    if ((alphaNumCheck(dirName) != 1 && specialDirCheck(dirName) != 1) || chdir(dirName) != 0) {
	    errCode = badMyCdCommand();
    } 

    return errCode;
}

int run(char *command_args[], int args_size) {
   int errCode = 0;
   int pid;

   command_args[args_size] = (char *) NULL;
   if ((pid = fork())) {
       waitpid(pid, NULL, 0);
   } else {
       execvp(command_args[0], command_args);
   }

   return errCode;
}

int compare_filenames(char *f1, char *f2, char *f3) {
    if (f1 && f2 && strcmp(f1, f2) == 0) return 0;
    if (f1 && f3 && strcmp(f1, f3) == 0) return 0;
    if (f2 && f3 && strcmp(f2, f3) == 0) return 0;
    return 1;
}

int exec(char *filename1, char *filename2, char *filename3, char *policy) {
    int errCode = 0;
    if (compare_filenames(filename1, filename2, filename3) == 0) {
        printf("Filenames must be unique!\n");
        return errCode;
    }

    char *files[3] = {filename1, filename2, filename3};
    struct pcb **pcbs = (struct pcb **)malloc(3 * sizeof(struct pcb *));
    if (pcbs == NULL) {
        printf("Something went wrong!!\n");
        return errCode;
    }

    int i = 0;
    while (files[i] != NULL) {
        FILE *p = fopen(files[i], "rt");      // the program is in a file

        if (p == NULL) {
            return badcommandFileDoesNotExist();
        }

        struct pcb *pcb = init_pcb(p);
        if (pcb == NULL) {
            printf("Something went wrong!!\n");
            return errCode;
        }
        pcbs[i] = pcb;
        fclose(p);

        i++;
    }

    run_scheduler(pcbs, i, policy);

    while (i >= 0) {
        free(pcbs[i]);
        i--;
    }

    free(pcbs);

    return errCode;
}
