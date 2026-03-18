#include "shell.h"

job jobs[MAX_JOBS];
int job_count = 0;

void parse_command(char *cmd, char **args)
{
    int i = 0;
    char *token = strtok(cmd, " ");
    while (token != NULL)
    {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

void handle_sigint(int sig)
{
    printf("\nmyshell> ");
    fflush(stdout);
}

void handle_sigchld(int sig)
{
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        remove_job(pid);
    }
}

void add_job(pid_t pid, char *command, int running)
{
    if (job_count < MAX_JOBS)
    {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].command, command, 255);
        jobs[job_count].running = running;
        job_count++;
    }
}

void remove_job(pid_t pid)
{
    for (int i = 0; i < job_count; i++)
    {
        if (jobs[i].pid == pid)
        {
            for (int j = i; j < job_count - 1; j++)
                jobs[j] = jobs[j + 1];
            job_count--;
            break;
        }
    }
}

void list_jobs()
{
    for (int i = 0; i < job_count; i++)
    {
        printf("[%d] %s %s\n", i + 1,
                jobs[i].running ? "Running" : "Stopped",
                jobs[i].command);
    }
}

void fg_job(int job_number)
{
    if (job_number < 1 || job_number > job_count)
    {
        printf("fg: invalid job number\n");
        return;
    }

    pid_t pid = jobs[job_number - 1].pid;
    kill(pid, SIGCONT);
    jobs[job_number - 1].running = 1;
    waitpid(pid, NULL, 0);
    remove_job(pid);
}

void bg_job(int job_number)
{
    if (job_number < 1 || job_number > job_count)
    {
        printf("bg: invalid job number\n");
        return;
    }

    pid_t pid = jobs[job_number - 1].pid;
    kill(pid, SIGCONT);
    jobs[job_number - 1].running = 1;
}

int is_builtin(char **args)
{
    if (args[0] == NULL)
        return 0;
    return (strcmp(args[0], "cd") == 0 ||
            strcmp(args[0], "exit") == 0 ||
            strcmp(args[0], "jobs") == 0 ||
            strcmp(args[0], "fg") == 0 ||
            strcmp(args[0], "bg") == 0);
}

void execute_builtin(char **args)
{
    if (strcmp(args[0], "cd") == 0)
    {
        if (args[1] == NULL)
            fprintf(stderr, "cd: missing argument\n");
        else if (chdir(args[1]) != 0)
            perror("cd failed");
    }
    else if (strcmp(args[0], "exit") == 0)
    {
        exit(0);
    }
    else if (strcmp(args[0], "jobs") == 0)
    {
        list_jobs();
    }
    else if (strcmp(args[0], "fg") == 0)
    {
        if (args[1] != NULL)
            fg_job(atoi(args[1]));
        else
            printf("fg: job number required\n");
    }
    else if (strcmp(args[0], "bg") == 0)
    {
        if (args[1] != NULL)
            bg_job(atoi(args[1]));
        else
            printf("bg: job number required\n");
    }
}

void execute_command(char **args, int background)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        /* Check for I/O redirection */
        int input_fd = -1, output_fd = -1;
        for (int i = 0; args[i] != NULL; i++)
        {
            if (strcmp(args[i], ">") == 0)
            {
                output_fd = open(args[i + 1],
                        O_CREAT | O_WRONLY | O_TRUNC,
                        0644);
                args[i] = NULL;
            }
            else if (strcmp(args[i], ">>") == 0)
            {
                output_fd = open(args[i + 1],
                        O_CREAT | O_WRONLY | O_APPEND,
                        0644);
                args[i] = NULL;
            }
            else if (strcmp(args[i], "<") == 0)
            {
                input_fd = open(args[i + 1], O_RDONLY);
                args[i] = NULL;
            }
        }

        if (input_fd != -1)
        {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != -1)
        {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        execvp(args[0], args);
        perror("exec failed");
        exit(1);
    }
    else
    {
        if (!background)
        {
            waitpid(pid, NULL, 0);
        }
        else
        {
            add_job(pid, args[0], 1);
            printf("[background pid %d]\n", pid);
        }
    }
}

/* main function */
int main()
{
    signal(SIGINT, handle_sigint);
    signal(SIGCHLD, handle_sigchld);

    char *line = NULL;
    size_t len = 0;

    while (1)
    {
        printf("myshell> ");
        fflush(stdout);

        if (getline(&line, &len, stdin) == -1)
            break;

        line[strcspn(line, "\n")] = 0;

        char *args[MAX_ARGS];
        parse_command(line, args);

        if (args[0] == NULL)
            continue;

        if (is_builtin(args))
        {
            execute_builtin(args);
            continue;
        }

        int background = 0;
        for (int i = 0; args[i] != NULL; i++)
        {
            if (strcmp(args[i], "&") == 0)
            {
                background = 1;
                args[i] = NULL;
                break;
            }
        }

        execute_command(args, background);
    }

    free(line);
    return 0;
}
