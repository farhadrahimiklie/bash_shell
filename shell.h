#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_ARGS 64
#define MAX_JOBS 128

typedef struct job {
    pid_t pid;
    char command[256];
    int running; // 1 = running, 0 = stopped
} job;

/* global job list */
extern job jobs[MAX_JOBS];
extern int job_count;

/* Function declarations */
void parse_command(char *cmd, char **args);
void handle_sigint(int sig);
void handle_sigchld(int sig);
void add_job(pid_t pid, char *command, int running);
void remove_job(pid_t pid);
void list_jobs();
void fg_job(int job_number);
void bg_job(int job_number);
int is_builtin(char **args);
void execute_builtin(char **args);
void execute_command(char **args, int background);

#endif
