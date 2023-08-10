/*********************************************************************
   Program  : miniShell                   Version    : 1.3
 --------------------------------------------------------------------
   skeleton code for linix/unix/minix command line interpreter
 --------------------------------------------------------------------
   File			: minishell.c
   Compiler/System	: gcc/linux

********************************************************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define NV 20       /* max number of command tokens */
#define NL 100      /* input buffer size */
char line[NL];      /* command input buffer */

typedef struct bgProcess {
    int id;
    pid_t pid;
    char command[NL];
} bgProcess;

bgProcess bg_processes[NV];
int bg_count = 0;
int bgID = 1;

void check_bg_processes() {
    int status;
    pid_t pid;
    for (int i = 0; i < bg_count; i++) {
        pid = waitpid(bg_processes[i].pid, &status, WNOHANG);
        if (pid > 0) {
            printf("[%d]+ Done %s\n", bg_processes[i].id, bg_processes[i].command);
            // Shift remaining processes
            for (int j = i; j < bg_count - 1; j++) {
                bg_processes[j] = bg_processes[j + 1];
            }
            i--; // Adjust index
            bg_count--; // Reduce the count of active bg processes
        }
    }
}

int main(int argk, char *argv[], char *envp[]) {
    int frkRtnVal;
    char *v[NV];
    char *sep = " \t\n";
    int i;
    int background = 0;

    while (1) {
        check_bg_processes(); // Check status of background processes

        fgets(line, NL, stdin);
        fflush(stdin);

        if (feof(stdin)) {
            exit(0);
        }

        if (line[strlen(line) - 2] == '&') {
            background = 1;
            line[strlen(line) - 2] = '\0';
        }

        v[0] = strtok(line, sep);
        for (i = 1; i < NV; i++) {
            v[i] = strtok(NULL, sep);
            if (v[i] == NULL) {
                break;
            }
        }

        if (strcmp(v[0], "cd") == 0) {
            if (v[1] == NULL) {
                perror("cd: Argument required");
            } else {
                if (chdir(v[1]) != 0) {
                    perror("cd");
                }
            }
            continue;
        }

        frkRtnVal = fork();
        if (frkRtnVal == -1) {
            perror("Failed to fork process");
            continue;
        }

        if (frkRtnVal == 0) {
            execvp(v[0], v);
            perror("Command execution failed");
            exit(EXIT_FAILURE);
        } else {
            if (background) {
                char full_command[NL] = "";  // A buffer to hold the full command

                // Concatenate command and its arguments 
                for (int j = 0; v[j] != NULL; j++) {
                    strcat(full_command, v[j]);
                    strcat(full_command, " ");  // Separate arguments with space
                }

                bg_processes[bg_count].id = bgID++;
                bg_processes[bg_count].pid = frkRtnVal;
                strcpy(bg_processes[bg_count].command, full_command);  // Copy the full command
                printf("[%d] %d\n", bg_processes[bg_count].id, frkRtnVal);
                bg_count++;
            } else {
                waitpid(frkRtnVal, NULL, 0);
                check_bg_processes();
            }
        }
        background = 0;
    }

    return 0;
}
