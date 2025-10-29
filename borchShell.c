#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_LINE 256

int main() {
    char line[MAX_LINE];
    char **args = NULL;
    int argc;

    while (1) {
        printf("mini-shell> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL)
            break;

        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, "exit") == 0)
            break;

        // Copia de seguridad de la línea
        char line_copy[MAX_LINE];
        strcpy(line_copy, line);

        int background = 0;
        int has_pipe = 0;
        int has_redirect = 0;
        char *outfile = NULL;

        argc = 0;
        args = NULL;
        int error = 0;


        char *p = line;
        int in_quotes = 0;
        char *start = NULL;

        while (*p != '\0') {
            if (*p == '"') {
                in_quotes = !in_quotes;
                *p = '\0'; 
                if (!in_quotes && start != NULL) {
                    args = realloc(args, (argc + 1) * sizeof(char *));
                    args[argc++] = strdup(start);
                    start = NULL;
                } else if (in_quotes) {
                    start = p + 1;
                }
            } else if (*p == ' ' && !in_quotes) {
                *p = '\0';
                if (start != NULL) {
                    args = realloc(args, (argc + 1) * sizeof(char *));
                    args[argc++] = strdup(start);
                    start = NULL;
                }
            } else if (start == NULL && *p != ' ') {
                start = p;
            }
            p++;
        }

        if (in_quotes) {
            fprintf(stderr, "Error: comillas sin cerrar\n");
            error = 1;
        }

        if (start != NULL && !error) {
            args = realloc(args, (argc + 1) * sizeof(char *));
            args[argc++] = strdup(start);
        }


        if (strchr(line_copy, '|') != NULL)
            has_pipe = 1;

        if (!error && argc > 0) {
            args = realloc(args, (argc + 1) * sizeof(char *));
            args[argc] = NULL;

            if (has_pipe) {
                char *cmd1_str = strtok(line_copy, "|");
                char *cmd2_str = strtok(NULL, "|");
                char *token;

                if (cmd2_str == NULL) {
                    fprintf(stderr, "Error: uso incorrecto del pipe\n");
                } else {
                    while (*cmd1_str == ' ') cmd1_str++;
                    while (*cmd2_str == ' ') cmd2_str++;

                 
                    char **cmd1 = NULL;
                    int c1_argc = 0;
                    token = strtok(cmd1_str, " ");
                    while (token != NULL) {
                        cmd1 = realloc(cmd1, (c1_argc + 1) * sizeof(char *));
                        cmd1[c1_argc++] = strdup(token);
                        token = strtok(NULL, " ");
                    }
                    cmd1 = realloc(cmd1, (c1_argc + 1) * sizeof(char *));
                    cmd1[c1_argc] = NULL;

              
                    char **cmd2 = NULL;
                    int c2_argc = 0;
                    token = strtok(cmd2_str, " ");
                    while (token != NULL) {
                        cmd2 = realloc(cmd2, (c2_argc + 1) * sizeof(char *));
                        cmd2[c2_argc++] = strdup(token);
                        token = strtok(NULL, " ");
                    }
                    cmd2 = realloc(cmd2, (c2_argc + 1) * sizeof(char *));
                    cmd2[c2_argc] = NULL;

                    
                    int fd[2];
                    pipe(fd);

                    pid_t pid1 = fork();
                    if (pid1 == 0) {
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[0]);
                        close(fd[1]);
                        execvp(cmd1[0], cmd1);
                        perror("execvp cmd1");
                        exit(1);
                    }

                    pid_t pid2 = fork();
                    if (pid2 == 0) {
                        dup2(fd[0], STDIN_FILENO);
                        close(fd[1]);
                        close(fd[0]);
                        execvp(cmd2[0], cmd2);
                        perror("execvp cmd2");
                        exit(1);
                    }

                    close(fd[0]);
                    close(fd[1]);
                    waitpid(pid1, NULL, 0);
                    waitpid(pid2, NULL, 0);

                    for (int i = 0; i < c1_argc; i++) free(cmd1[i]);
                    for (int i = 0; i < c2_argc; i++) free(cmd2[i]);
                    free(cmd1);
                    free(cmd2);
                }
            } else {
                pid_t pid = fork();
                if (pid < 0) {
                    perror("fork");
                    exit(1);
                } else if (pid == 0) {
                    if (has_redirect && outfile != NULL) {
                        int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd < 0) {
                            perror("open");
                            exit(1);
                        }
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                    }
                    execvp(args[0], args);
                    perror("execvp");
                    exit(1);
                } else {
                    waitpid(pid, NULL, 0);
                }
            }
        }

        for (int i = 0; i < argc; i++)
            free(args[i]);
        free(args);
        free(outfile);
    }

    printf("Saliendo de la mini-shell.\n");
    return 0;
}
