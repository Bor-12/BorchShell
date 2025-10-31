#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

static void silence_stderr(void (*test_func)(void)) {
    fflush(stderr);
    FILE *old_stderr = stderr;
    stderr = fopen("/dev/null", "w");
    if (!stderr) {
        perror("stderr");
        exit(1);
    }
    test_func();
    fflush(stderr);
    fclose(stderr);
    stderr = old_stderr;
}

static void test_simple_command(void) {
    CommandInfo cmd = parse_command("ls -l /home");
    assert(cmd.n_cmds == 1);
    assert(cmd.cmds[0].argc == 3);
    assert(strcmp(cmd.cmds[0].args[0], "ls") == 0);
    assert(strcmp(cmd.cmds[0].args[1], "-l") == 0);
    assert(strcmp(cmd.cmds[0].args[2], "/home") == 0);
    free_command(&cmd);
    printf("test_simple_command OK\n");
}

static void test_redirection(void) {
    CommandInfo cmd = parse_command("cat < input.txt");
    assert(cmd.n_cmds == 1);
    assert(cmd.infile != NULL);
    assert(strcmp(cmd.infile, "input.txt") == 0);
    free_command(&cmd);
    printf("test_redirection OK\n");
}

static void test_multiple_pipes(void) {
    CommandInfo cmd = parse_command("ls -l | grep .c | sort");
    assert(cmd.n_cmds == 3);
    assert(strcmp(cmd.cmds[0].args[0], "ls") == 0);
    assert(strcmp(cmd.cmds[1].args[0], "grep") == 0);
    assert(strcmp(cmd.cmds[2].args[0], "sort") == 0);
    free_command(&cmd);
    printf("test_multiple_pipes OK\n");
}

static void test_invalid_redirect(void) {
    CommandInfo cmd = parse_command("ls > salida.txt extra");
    assert(cmd.n_cmds == 0);
    free_command(&cmd);
    printf("test_invalid_redirect OK\n");
}

/* --- NUEVOS TESTS --- */

static void test_quoted_args(void) {
    CommandInfo cmd = parse_command("echo \"hola mundo\"");
    assert(cmd.n_cmds == 1);
    assert(cmd.cmds[0].argc == 2);
    assert(strcmp(cmd.cmds[0].args[0], "echo") == 0);
    assert(strcmp(cmd.cmds[0].args[1], "hola mundo") == 0);
    free_command(&cmd);
    printf("test_quoted_args OK\n");
}

static void test_extra_spaces(void) {
    CommandInfo cmd = parse_command("   ls     -a    ");
    assert(cmd.n_cmds == 1);
    assert(cmd.cmds[0].argc == 2);
    assert(strcmp(cmd.cmds[0].args[0], "ls") == 0);
    assert(strcmp(cmd.cmds[0].args[1], "-a") == 0);
    free_command(&cmd);
    printf("test_extra_spaces OK\n");
}

static void test_output_redirect(void) {
    CommandInfo cmd = parse_command("grep main > result.txt");
    assert(cmd.n_cmds == 1);
    assert(cmd.outfile != NULL);
    assert(strcmp(cmd.outfile, "result.txt") == 0);
    free_command(&cmd);
    printf("test_output_redirect OK\n");
}

static void test_pipe_with_spaces(void) {
    CommandInfo cmd = parse_command("  ps aux   |   grep bash  ");
    assert(cmd.n_cmds == 2);
    assert(strcmp(cmd.cmds[0].args[0], "ps") == 0);
    assert(strcmp(cmd.cmds[1].args[0], "grep") == 0);
    free_command(&cmd);
    printf("test_pipe_with_spaces OK\n");
}

static void test_empty_command(void) {
    CommandInfo cmd = parse_command("");
    assert(cmd.n_cmds == 0);
    free_command(&cmd);
    printf("test_empty_command OK\n");
}

static void test_invalid_both_redirects(void) {
    CommandInfo cmd = parse_command("cat < in.txt > out.txt extra");
    assert(cmd.n_cmds == 0);
    free_command(&cmd);
    printf("test_invalid_both_redirects OK\n");
}

static void test_only_redirect_token(void) {
    CommandInfo cmd = parse_command(">");
    assert(cmd.n_cmds == 0);
    free_command(&cmd);
    printf("test_only_redirect_token OK\n");
}

int main(void) {
    test_simple_command();
    test_redirection();
    test_multiple_pipes();
    silence_stderr(test_invalid_redirect);
    test_quoted_args();
    test_extra_spaces();
    test_output_redirect();
    test_pipe_with_spaces();
    test_empty_command();
    silence_stderr(test_invalid_both_redirects);
    silence_stderr(test_only_redirect_token);

    printf("\nTodos los tests pasaron correctamente \n");
    return 0;
}
