#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

// Inicializa estructura
static void init_command(CommandInfo *cmd) {
    cmd->cmds = NULL;
    cmd->n_cmds = 0;
    cmd->outfile = NULL;
    cmd->infile = NULL;
    cmd->has_redirect = 0;
    cmd->exit_flag = 0;
}

// Tokeniza una cadena en palabras (respetando comillas)
static char **split_args(const char *str, int *argc_out) {
    char *copy = strdup(str);
    char *p = copy;
    int in_quotes = 0;
    char *start = NULL;
    char **args = NULL;
    int argc = 0;

    while (*p) {
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

    if (start != NULL) {
        args = realloc(args, (argc + 1) * sizeof(char *));
        args[argc++] = strdup(start);
    }

    args = realloc(args, (argc + 1) * sizeof(char *));
    args[argc] = NULL;

    *argc_out = argc;
    free(copy);
    return args;
}

// Procesa redirecciones globales
// Procesa redirecciones globales (solo permite un token tras < o >)
static int process_redirections(CommandInfo *cmd, char *line) {
    char *out = strchr(line, '>');
    char *in = strchr(line, '<');

    if (out && in) {
        fprintf(stderr, "Error: no se permite usar < y > en la misma línea.\n");
        return -1;
    }

    if (out) {
        cmd->has_redirect = 1;
        *out = '\0';
        out++;
        while (*out == ' ') out++;

        // Solo tomar el primer token después de >
        char *end = strpbrk(out, " <>|");
        if (end) {
            *end = '\0';
            // Si hay algo más después → error
            while (*(++end) == ' ') {}
            if (*end != '\0') {
                fprintf(stderr, "Error: texto no permitido después de redirección.\n");
                return -1;
            }
        }

        if (*out == '\0') {
            fprintf(stderr, "Error: falta el nombre del archivo de salida.\n");
            return -1;
        }

        cmd->outfile = strdup(out);
    }

    if (in) {
        cmd->has_redirect = 1;
        *in = '\0';
        in++;
        while (*in == ' ') in++;

        // Solo tomar el primer token después de <
        char *end = strpbrk(in, " <>|");
        if (end) {
            *end = '\0';
            while (*(++end) == ' ') {}
            if (*end != '\0') {
                fprintf(stderr, "Error: texto no permitido después de redirección.\n");
                return -1;
            }
        }

        if (*in == '\0') {
            fprintf(stderr, "Error: falta el nombre del archivo de entrada.\n");
            return -1;
        }

        cmd->infile = strdup(in);
    }

    return 0;
}


// Parse principal
CommandInfo parse_command(char *line) {
    CommandInfo cmd;
    init_command(&cmd);

    
    char *line_copy = strdup(line);

    if (process_redirections(&cmd, line_copy) != 0) {
        free(line_copy);
        return cmd;
    }

    char *saveptr;
    char *token = strtok_r(line_copy, "|", &saveptr);

    while (token) {
        cmd.cmds = realloc(cmd.cmds, (cmd.n_cmds + 1) * sizeof(SimpleCommand));
        SimpleCommand *current = &cmd.cmds[cmd.n_cmds];

        current->args = split_args(token, &current->argc);
        cmd.n_cmds++;

        token = strtok_r(NULL, "|", &saveptr);
    }

    free(line_copy);  
    return cmd;
}



// Pide una línea y la parsea
CommandInfo get_command() {
    char line[MAX_LINE];
    CommandInfo cmd;
    init_command(&cmd);

    printf("mini-shell> ");
    fflush(stdout);

    if (fgets(line, sizeof(line), stdin) == NULL) {
        cmd.exit_flag = 1;
        return cmd;
    }

    line[strcspn(line, "\n")] = '\0';

    if (strcmp(line, "exit") == 0) {
        cmd.exit_flag = 1;
        return cmd;
    }

    cmd = parse_command(line);
    return cmd;
}

// Liberación de memoria
void free_command(CommandInfo *cmd) {
    if (!cmd) return;

    for (int i = 0; i < cmd->n_cmds; i++) {
        for (int j = 0; j < cmd->cmds[i].argc; j++)
            free(cmd->cmds[i].args[j]);
        free(cmd->cmds[i].args);
    }

    free(cmd->cmds);
    free(cmd->outfile);
    free(cmd->infile);
}
