#include <stdio.h>
#include "parser.h"

int main() {
    while (1) {
        CommandInfo cmd = get_command();
        if (cmd.exit_flag)
            break;

        printf("Número de comandos: %d\n", cmd.n_cmds);

        for (int i = 0; i < cmd.n_cmds; i++) {
            printf("Comando %d:\n", i + 1);
            for (int j = 0; j < cmd.cmds[i].argc; j++)
                printf("  arg[%d]: %s\n", j, cmd.cmds[i].args[j]);
        }

        if (cmd.outfile)
            printf("Redirección de salida: %s\n", cmd.outfile);
        if (cmd.infile)
            printf("Redirección de entrada: %s\n", cmd.infile);

        free_command(&cmd);
    }

    return 0;
}
