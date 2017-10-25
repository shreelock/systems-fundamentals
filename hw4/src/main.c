#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>

#include "sfish.h"
#include "debug.h"

struct state {
    char* curr_dir;
    char* prev_dir;
};


void init(struct state *s1);

char* get_shell_prompt(struct state *s1);

void process_input(char* input, struct state* currstate) ;

char* sget_cwd();

char* sget_home();

void print_help();

int main(int argc, char *argv[], char* envp[]) {
    char* input;
    struct state curr_state;
    init(&curr_state);
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
        char* prompt = get_shell_prompt(&curr_state);
        input = readline(prompt);
        process_input(input, &curr_state);

        //write(1, "\e[s", strlen("\e[s"));
        //write(1, "\e[20;10H", strlen("\e[20;10H"));
        //write(1, "SomeText", strlen("SomeText"));
        //write(1, "\e[u", strlen("\e[u"));

        // If EOF is read (aka ^D) readline returns NULL
        if(input == NULL) {
            continue;
        }


        // Currently nothing is implemented


        // You should change exit to a "builtin" for your hw.
        exited = strcmp(input, "exit") == 0;

        // Readline mallocs the space for input. You must free it.
//        rl_free(input);

    } while(!exited);

    debug("%s", "user entered 'exit'");

    return EXIT_SUCCESS;
}

void init(struct state *s1){
    s1->curr_dir = sget_cwd();
    s1->prev_dir = s1->curr_dir;
}

void update(struct state *s1){
    s1->prev_dir = s1->curr_dir;
    s1->curr_dir = sget_cwd();
}

void process_input(char* input, struct state* currstate) {
    //-------------------------------------------------
    int nargs = 1;
    char *tmp = strdup(input);
    char* tempchr = strtok(tmp, " ");
    while ((tempchr = strtok(NULL, " ")) != NULL)
        nargs++;
    free(tmp);
    free(tempchr);
    //debug("number of arguments = %d", nargs);
    //-------------------------------------------------

    char* first_word = strtok(input, " ");
    if (strcmp(first_word,"exit") ==0) {
        exit(0);
    }

    else if (strcmp (first_word, "pwd") == 0) {
        printf("%s\n", currstate->curr_dir);
    }

    else if ( strcmp(first_word, "help") == 0 ) {
        printf("%s\n", "This is the help menu!!");
        print_help();
    }

    else if (strcmp(first_word, "cd") == 0 ) {
        //Note : Quoted arguemnts are NOT reqd. to be supported.
        switch (nargs) {
            case 1:
                if (chdir(sget_home())==0) {
                    update(currstate);
                } else { printf(HOME_NOT_INIT); }
                break;
            case 2:
                first_word = strtok(NULL, " ");
                //printf("Path : %s\n", first_word);
                if (strcmp(first_word, "-") == 0)
                    first_word = currstate->prev_dir;
                if (chdir(first_word) == 0) {
                    update(currstate);
                } else { printf(CD_PATH_NEXIST, first_word); }
                break;
            default:
                printf(CD_WRONG_NARGS);
        }
    }


    else { printf(EXEC_NOT_FOUND, input); }
}

char* get_formatted_pwd(struct state* currstate){
    char *homedir = NULL, *pwd = NULL;
    homedir = sget_home();
    pwd = currstate->curr_dir;
    if (homedir !=NULL){
        char *ptr = strstr(pwd, homedir);
        if(ptr !=NULL) {
            size_t prompt_size = sizeof(char) * (strlen(pwd) - strlen(homedir) + 2);
            char *prompt = malloc(prompt_size);
            prompt[0] = '~';
            strcpy(prompt + 1, ptr + strlen(homedir));
            return prompt;
        }
    }
    return pwd;
}

char* get_shell_prompt(struct state* s1){
    char* pwd = get_formatted_pwd(s1);
    //debug("%s", pwd);
    char* suffix = " :: shreesharma >>";
    char* finalstring = malloc(sizeof(char)*(strlen(pwd) + strlen(suffix) + 1));
    strcpy(finalstring, pwd);
    strcat(finalstring, suffix);
    return finalstring;
}

char* sget_cwd(){
    char* path = NULL;
    path = getcwd(path, 0);
    return path;
}

char* sget_home(){
    char* path = NULL;
    path = getenv("HOME");
    if( path == NULL)
        printf(HOME_NOT_INIT);
    return path;
}

void print_help(){
    char* string = "Following are the builtin commands -\n1. help: Print a list of all builtins and their basic usage in a single column.\n2. exit: Exits the shell.\n3. cd: Changes the current working directory of the shell.\n4. pwd: Prints the absolute path of the current working directory.";
    printf("%s\n",string);
}
