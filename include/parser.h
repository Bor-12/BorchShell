#ifndef PARSER_H
#define PARSER_H

#define MAX_LINE 256

typedef struct {
    char **args;   // Argumentos de un solo comando
    int argc;
} SimpleCommand;

typedef struct {
    SimpleCommand *cmds; // Lista de comandos separados por pipes
    int n_cmds;          // Número de comandos en la tubería
    char *outfile;       // Archivo de salida (>)
    char *infile;        // Archivo de entrada (<)
    int has_redirect;
    int exit_flag;
} CommandInfo;

// Funciones principales
CommandInfo get_command();
CommandInfo parse_command(char *line);
void free_command(CommandInfo *cmd);

#endif
