#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <wait.h>
#include <fcntl.h>

#include "sfish.h"
#include "debug.h"

#define CD_PATH_NEXIST "cd: path not exist"
#define HOME_NOT_INIT "cd: home not init"
#define REDIRECTION_SYNTAX_ERROR "Syntax error with redirection"
#define FILE_NO_EXIST_ERROR "File doesnt exist"

#define sLEFT_ARROW '<'
#define sRIGHT_ARROW '>'
#define sPIPE '|'

struct state {
    char* curr_dir;
    char* prev_dir;
};
//TODO Compare _exit and exit
//TODO SIGCHILD Handling
//TODO Part IV
//TODO Piping
//TODO cat with empty arg not working

void init(struct state *s1);

char* get_shell_prompt(struct state *s1);

void process_input(char *mainarg, char *inarg, char *outarg, struct state *currstate);

char* sget_cwd();

char* sget_home();

void print_help();

void process_io_redirect(char* input, struct state* currentstate);

void print_credits();

void process_pipes(char *input, struct state *currentstate);

char* command_to_run = NULL;

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
        if(input == NULL || strcmp(input,"")==0) {
            printf("\n");
            continue;
        }
//        process_pipes(input, &curr_state);
        process_io_redirect(input, &curr_state);

        //write(1, "\e[s", strlen("\e[s"));
        //write(1, "\e[20;10H", strlen("\e[20;10H"));
        //write(1, "SomeText", strlen("SomeText"));
        //write(1, "\e[u", strlen("\e[u"));

        // If EOF is read (aka ^D) readline returns NULL


        // Currently nothing is implemented


        // You should change exit to a "builtin" for your hw.
        exited = strcmp(input, "exit") == 0;

        // Readline mallocs the space for input. You must free it.
        rl_free(input);

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

void process_input(char *mainarg, char *inarg, char *outarg, struct state *currstate) {
    //-------------------------------------------------
    int nargs = 1;
    char *tmp = strdup(mainarg);
    char* tempchr = strtok(tmp, " ");
    while ((tempchr = strtok(NULL, " ")) != NULL)
        nargs++;


    char* first_word = strtok(mainarg, " ");
    if (strcmp(first_word,"exit") ==0) {
        _exit(0);
    }

    else if (strcmp (first_word, "pwd") == 0) {
        printf("%s\n", currstate->curr_dir);
    }

    else if ( strcmp(first_word, "help") == 0 ) {
        printf("%s\n", "This is the help menu!!");
        print_help();
    }

    else if (strcmp(first_word, "credits") == 0){
        print_credits();
    }

    else if (strcmp(first_word, "cd") == 0 ) {
        //Note : Quoted arguemnts are NOT reqd. to be supported.
        switch (nargs) {
            case 1:
                if (chdir(sget_home())==0) {
                    update(currstate);
                } else { printf(BUILTIN_ERROR, HOME_NOT_INIT); }
                break;
            default:
                first_word = strtok(NULL, " ");
                //printf("Path : %s\n", first_word);
                if (strcmp(first_word, "-") == 0)
                    first_word = currstate->prev_dir;
                if (chdir(first_word) == 0) {
                    update(currstate);
                } else { printf(BUILTIN_ERROR, CD_PATH_NEXIST); }
        }
    }
    /*
    else if (strcmp(first_word, "ls") == 0 ){
        int child_status;
        if (fork() == 0) {
            execlp("ls","ls", (char*)0);
            exit(0);
        } else {
            wait(&child_status);
        }
    }*/
    else {
        int child_status;
        //Since fist_word pointer is iterator.
        char* command = strdup(first_word);
        if (fork() == 0) {
            char** argsarray = NULL;
            int nvars = 0;
            while(first_word){
                argsarray = realloc(argsarray, sizeof(char*)*++nvars);
                argsarray[nvars-1] = first_word;
                first_word = strtok(NULL, " ");
            }
            argsarray = realloc(argsarray, sizeof(char*)*(nvars+1));
            argsarray[nvars]=0;
            //-----------------------------------------------------
            if (inarg!=NULL) {
                //printf("Got redirection : input = _%s_\n", inarg);
                if (access(inarg, F_OK)==-1){
                    printf(SYNTAX_ERROR, FILE_NO_EXIST_ERROR);
                    return;
                }
                int in = open(inarg, O_RDONLY);
                dup2(in, STDIN_FILENO);
                close(in);
            }
            if (outarg!=NULL) {
                //printf("Got redirection : output = _%s_\n", outarg);
                int out = creat(outarg, 0664);
                dup2(out, STDOUT_FILENO);
                close(out);
            }
            //-----------------------------------------------------
            if( execvp(command,argsarray)==-1 )
                printf(EXEC_NOT_FOUND, command);
            free(argsarray);
            _exit(0);
        } else {
            wait(&child_status);
        }
    }
}

void process_io_redirect(char* input, struct state* currentstate){
    char* inputcopy = strdup(input);
    char* ptr = inputcopy;
    int outarrcnt=0, inarrcnt=0, len = (int) strlen(ptr);
    int inarrow = -1, outarrow = -1, i=0;
    char* inarg = NULL, *outarg=NULL, *mainarg=NULL;

    while(i < len) {
        if (*ptr == sRIGHT_ARROW) {
            outarrcnt++;
            if (outarrcnt > 1) {
                printf(SYNTAX_ERROR, REDIRECTION_SYNTAX_ERROR);
                return;
            }
            outarrow = i;
        }
        if (*ptr == sLEFT_ARROW) {
            inarrcnt++;
            if (inarrcnt++ > 1) {
                printf(SYNTAX_ERROR, REDIRECTION_SYNTAX_ERROR);
                return;
            }
            inarrow = i;
        }
        ptr++;
        i++;
    }

    //printf("Found arrows at : inarr=%d, outarr=%d, lenght=%d\n", inarrow, outarrow, len);
    if(inarrow!=-1 || outarrow!=-1){
        int mainarglen = 0;
        if(inarrow == -1)
            mainarglen = outarrow;
        else if (outarrow == -1)
            mainarglen = inarrow;
        else
            mainarglen = inarrow < outarrow ? inarrow : outarrow;

        //printf("-->%d\n", mainarglen);
        if(mainarglen == 0) {
            printf(SYNTAX_ERROR, REDIRECTION_SYNTAX_ERROR);
            return;
        }
        mainarg = calloc((size_t)mainarglen, sizeof(char));
        strncpy(mainarg, inputcopy, (size_t) mainarglen);
        //printf("mainarg-------->_%s_\n", mainarg);
        if (strcmp(mainarg, " ")==0){
            printf(SYNTAX_ERROR, REDIRECTION_SYNTAX_ERROR);
            return;
        }

        char* tempmainarg = mainarg;
        while(*tempmainarg==' ')
            tempmainarg++;
        //printf("tempmainarg-------->_%s_\n", tempmainarg);


        char* tempmainargend = mainarg + mainarglen - 1;
        //printf("tempmainargend-------->_%s_\n", tempmainargend);

        while(*tempmainargend==' ') {
            *tempmainargend = '\0';
            tempmainargend--;
        }
        //printf("mainarg-------->_%s_\n", tempmainarg);

        mainarg = tempmainarg;
        if(strlen(mainarg)==0) {
            printf(SYNTAX_ERROR, REDIRECTION_SYNTAX_ERROR);
            return;
        }

    } else {
        mainarg = inputcopy;
    }


    if(inarrow!=-1){
        int inargstart = inarrow + 1;
        int inargend = outarrow > inarrow ? outarrow-1 : len-1;
        int inarglen = inargend - inargstart + 1;

        if(inarglen == 0){
            printf(SYNTAX_ERROR, REDIRECTION_SYNTAX_ERROR);
            return;
        }

        inarg = calloc((size_t) inarglen, sizeof(char));
        strncpy(inarg, inputcopy + inargstart, (size_t) inarglen);
        //printf("inarg-------->_%s_\n", inarg);

        char* tempinarg = inarg;
        while(*tempinarg==' ')
            tempinarg++;
        //printf("teminarg-------->_%s_\n", tempinarg);

        char* tempinargend = inarg + inarglen -1;
        while(*tempinargend==' ') {
            *tempinargend = '\0';
            tempinargend --;
        }
        //printf("teminarg-------->_%s_\n", tempinarg);

        inarg = tempinarg;
        if(strlen(inarg)==0) {
            printf(SYNTAX_ERROR, REDIRECTION_SYNTAX_ERROR);
            return;
        }

    }

    if(outarrow!=-1){
        //printf("outarrow-->%d",outarrow);
        int outargstart = outarrow + 1;
        int outargend = inarrow > outarrow ? inarrow-1 : len-1;
        int outarglen = outargend - outargstart + 1;

        if(outarglen == 0){
            printf(SYNTAX_ERROR, REDIRECTION_SYNTAX_ERROR);
            return;
        }

        outarg = calloc((size_t) outarglen, sizeof(char));
        strncpy(outarg, inputcopy + outargstart, (size_t) outarglen);
        //printf("outarg-------->_%s_\n", outarg);

        char* tempoutarg = outarg;
        while(*tempoutarg==' ')
            tempoutarg++;
        //printf("temoutarg-------->_%s_\n", tempoutarg);

        char* tempoutargend = outarg + outarglen -1;
        while(*tempoutargend ==' ') {
            *tempoutargend = '\0';
            tempoutargend --;
        }
        //printf("temoutarg-------->_%s_\n", tempoutarg);

        outarg = tempoutarg;
        if (strlen(outarg)==0) {
            printf(SYNTAX_ERROR, REDIRECTION_SYNTAX_ERROR);
            return;
        }
    }
    process_input(mainarg, inarg, outarg, currentstate);
}

void process_pipes(char *input, struct state *currentstate) {

    char* inputcopy = strdup(input);
    char* ptr = inputcopy;
    int len = (int) strlen(ptr);
    int oldpipecmdend = -2, newpipecmdend = 0, newpipecmdstart = 0, i=0;
    char* pipecmd = NULL;

    while(i < len) {
        if (*ptr == sPIPE || i==len-1) {
            newpipecmdstart = oldpipecmdend + 2; // To ignore the pipe completely
            newpipecmdend = (i==len-1) ? i : i-1; // The command ends before the Pipe
            oldpipecmdend = newpipecmdend; //For Next Iteration
            //printf("New pipe command found at %d - %d\n", newpipecmdstart, newpipecmdend);

            int pipecmdlen = newpipecmdend - newpipecmdstart + 1;
            if (pipecmdlen == 1){ // Due to +1
                printf(SYNTAX_ERROR, REDIRECTION_SYNTAX_ERROR);
                return;
            }

            pipecmd = calloc((size_t) pipecmdlen, sizeof(char));
            strncpy(pipecmd, inputcopy + newpipecmdstart, (size_t) pipecmdlen);


            char* tempcmds = pipecmd;
            while(*tempcmds==' ')
                tempcmds++;

            char* tempcmde = pipecmd + pipecmdlen -1;
            while(*tempcmde ==' ') {
                *tempcmde = '\0';
                tempcmde --;
            }

            pipecmd = tempcmds;
            if (strlen(pipecmd)==0) {
                printf(SYNTAX_ERROR, REDIRECTION_SYNTAX_ERROR);
                return;
            }
            //printf("Got Command to run as : _%s_\n", pipecmd);
            process_io_redirect(pipecmd, currentstate);

        }
        ptr++;
        i++;
    }
/*
    if(newpipecmdend!=-1){
        //printf("newpipecmdend-->%d",newpipecmdend);
        int outargstart = newpipecmdend + 1;
        int outargend = oldpipecmdend > newpipecmdend ? oldpipecmdend-1 : len-1;
        int pipecmdlen = outargend - outargstart + 1;

        if(pipecmdlen == 0){
            printf(SYNTAX_ERROR, REDIRECTION_SYNTAX_ERROR);
            return;
        }

        pipecmd = calloc((size_t) pipecmdlen, sizeof(char));
        strncpy(pipecmd, inputcopy + outargstart, (size_t) pipecmdlen);
        //printf("pipecmd-------->_%s_\n", pipecmd);

        char* tempoutarg = pipecmd;
        while(*tempoutarg==' ')
            tempoutarg++;
        //printf("temoutarg-------->_%s_\n", tempoutarg);

        char* tempoutargend = pipecmd + pipecmdlen -1;
        while(*tempoutargend ==' ') {
            *tempoutargend = '\0';
            tempoutargend --;
        }
        //printf("temoutarg-------->_%s_\n", tempoutarg);

        pipecmd = tempoutarg;
        if (strlen(pipecmd)==0) {
            printf(SYNTAX_ERROR, REDIRECTION_SYNTAX_ERROR);
            return;
        }
    }
    process_input(mainarg, inarg, pipecmd, currentstate);
*/
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
        printf(EXEC_ERROR, HOME_NOT_INIT);
    return path;
}

void print_help(){
    char* string = "Following are the builtin commands -\n1. help: Print a list of all builtins and their basic usage in a single column.\n2. exit: Exits the shell.\n3. cd: Changes the current working directory of the shell.\n4. pwd: Prints the absolute path of the current working directory.";
    printf("%s\n",string);
}

void print_credits(){
    char*  string = "Thanks to -"\
                    "\n1. https://stackoverflow.com/questions/11515399/implementing-shell-in-c-and-need-help-handling-input-output-redirection"\
                    "\n2. https://stackoverflow.com/questions/11198604/c-split-string-into-an-array-of-strings#11198630"\
                    "\n3. http://developerweb.net/viewtopic.php?id=4881"\
                    "\n4. https://stackoverflow.com/questions/7292642/grabbing-output-from-exec#7292659"\
                    "";
    printf("%s\n",string);
}
