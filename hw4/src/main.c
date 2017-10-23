#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>

#include "sfish.h"
#include "debug.h"

char* get_shell_prompt();

int main(int argc, char *argv[], char* envp[]) {
    char* input;
    bool exited = false;

    if(!isatty(STDIN_FILENO)) {
        // If your shell is reading from a piped file
        // Don't have readline write anything to that file.
        // Such as the prompt or "user input"
        if((rl_outstream = fopen("/dev/null", "w")) == NULL){
            perror("Failed trying to open DEVNULL");
            exit(EXIT_FAILURE);
        }
    }

    do {
        char* prompt = get_shell_prompt();
        //debug("Got prompt as : %s", prompt);
        input = readline(prompt);

        write(1, "\e[s", strlen("\e[s"));
        write(1, "\e[20;10H", strlen("\e[20;10H"));
        write(1, "SomeText", strlen("SomeText"));
        write(1, "\e[u", strlen("\e[u"));

        // If EOF is read (aka ^D) readline returns NULL
        if(input == NULL) {
            continue;
        }


        // Currently nothing is implemented
        printf(EXEC_NOT_FOUND, input);

        // You should change exit to a "builtin" for your hw.
        exited = strcmp(input, "exit") == 0;

        // Readline mallocs the space for input. You must free it.
        rl_free(input);

    } while(!exited);

    debug("%s", "user entered 'exit'");

    return EXIT_SUCCESS;
}

char* get_formatted_pwd(){
    char *homedir = NULL, *pwd = NULL;
    homedir = getenv("HOME");
    pwd = getcwd(pwd,0);
    if (homedir !=NULL){
        char *ptr = strstr(pwd, homedir);
        if(ptr !=NULL) {
            size_t prompt_size = sizeof(char) * (strlen(pwd) - strlen(homedir) + 1);
            char *prompt = malloc(prompt_size);
            prompt[0] = '~';
            strcpy(prompt + 1, ptr + strlen(homedir));
            return prompt;
        }
    }
    return pwd;
}

char* get_shell_prompt(){
    char* pwd = get_formatted_pwd();
    //debug("%s", pwd);
    char* suffix = " :: shreesharma >>";
    char* finalstring = malloc(sizeof(char)*(strlen(pwd)+ strlen(suffix)));
    strcpy(finalstring, pwd);
    strcat(finalstring, suffix);
    return finalstring;
}
