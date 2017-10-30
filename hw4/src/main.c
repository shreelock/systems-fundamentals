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
#define REDIRECTION_SYNTAX_ERROR_2 "statement length zero"
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

void process_input(char *mainarg, char *inarg, char *outarg, struct state *currstate, int* pd);

char* sget_cwd();

char* sget_home();

void print_help();

void process_io_redirect(char *input, struct state *currentstate, int* pd);

void print_credits();

void process_pipes(char *input, struct state *currentstate);

char* get_trimmed(char* sentence) ;

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
        char* inputcopy = strdup(input);
        if(input == NULL || strcmp(input,"")==0) {
            printf("\n");
            continue;
        }
        process_pipes(inputcopy, &curr_state);
        //process_io_redirect(input, &curr_state);

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

void process_input(char *mainarg, char *inarg, char *outarg, struct state *currstate, int* pd) {
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
            if(pd !=NULL) {
                dup2(pd[1], STDOUT_FILENO); //Magic
            }
            //-----------------------------------------------------
            int execvp_ret = execvp(command,argsarray);
            if( execvp_ret ==-1 ) {
                printf(EXEC_NOT_FOUND, command);
            }
            free(argsarray);
            _exit(0);
        } else {
            wait(&child_status);
        }
    }
}

void process_io_redirect(char *input, struct state *currentstate, int* pd) {
    char* inputcopy = strdup(input);
    char* ptr = inputcopy;
    int outarrcnt=0, inarrcnt=0, len = (int) strlen(ptr);
    int inarrow = -1, outarrow = -1, i=0;
    char* inarg = NULL, *outarg=NULL, *mainarg=NULL;

    while(i < len) {
        if (*ptr == sRIGHT_ARROW) {
            outarrcnt++;
            if (outarrcnt > 1) {
                printf(SYNTAX_ERROR, "LINE215");
                return;
            }
            outarrow = i;
        }
        if (*ptr == sLEFT_ARROW) {
            inarrcnt++;
            if (inarrcnt++ > 1) {
                printf(SYNTAX_ERROR, "LINE223");
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
            printf(SYNTAX_ERROR, "LINE244");
            return;
        }
        mainarg = calloc((size_t)mainarglen, sizeof(char));
        strncpy(mainarg, inputcopy, (size_t) mainarglen);
        //printf("mainarg-------->_%s_\n", mainarg);
        if (strcmp(mainarg, " ")==0){
            printf(SYNTAX_ERROR, "LINE251");
            return;
        }
        mainarg = get_trimmed(mainarg);
        if(strlen(mainarg)==0) {
            printf(SYNTAX_ERROR, "LINE272");
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
            printf(SYNTAX_ERROR, "LINE287");
            return;
        }

        inarg = calloc((size_t) inarglen, sizeof(char));
        strncpy(inarg, inputcopy + inargstart, (size_t) inarglen);
        //printf("inarg-------->_%s_\n", inarg);

        inarg = get_trimmed(inarg);
        if(strlen(inarg)==0) {
            printf(SYNTAX_ERROR, "LIN309");
            return;
        }

    }

    if(outarrow!=-1){
        //printf("outarrow-->%d",outarrow);
        int outargstart = outarrow + 1;
        int outargend = inarrow > outarrow ? inarrow-1 : len-1;
        int outarglen = outargend - outargstart + 1;

        if(outarglen == 0){
            printf(SYNTAX_ERROR, "LINE322");
            return;
        }

        outarg = calloc((size_t) outarglen, sizeof(char));
        strncpy(outarg, inputcopy + outargstart, (size_t) outarglen);
        //printf("outarg-------->_%s_\n", outarg);

        outarg = get_trimmed(outarg);
        if (strlen(outarg)==0) {
            printf(SYNTAX_ERROR, "LINE344");
            return;
        }
    }
    process_input(mainarg, inarg, outarg, currentstate, pd);
}

void process_pipes(char *input, struct state *currentstate) {

    char* inputcopy = strdup(input);
    inputcopy = get_trimmed(inputcopy);
    if(*inputcopy==sPIPE || *(inputcopy+strlen(inputcopy)-1)==sPIPE){
        printf(SYNTAX_ERROR, "LINE366");
        return;
    }

    char* word = strtok(inputcopy, "|");
    char** exprsarray = NULL;
    int nvars = 0;
    while(word){
        exprsarray = realloc(exprsarray, sizeof(char*)*++nvars);
        word = get_trimmed(word);
        if(strlen(word)==0){
            printf(SYNTAX_ERROR, "line376");
            return;
        }
        exprsarray[nvars-1] = word;
        //printf("word: .%s.\n", word);
        word = strtok(NULL, "|");
    }

    for (int k=0; k < nvars-1 ; k++){
        int pd[2];
        pipe(pd);
        char* expr = exprsarray[k];
        process_io_redirect(expr, currentstate, pd);
        dup2(pd[0], STDIN_FILENO);
        close(pd[1]);
    }
    process_io_redirect(exprsarray[nvars-1], currentstate, NULL);
}


char* get_trimmed(char* sentence) {
    char* tempstmt = strdup(sentence);
    while(*tempstmt==' ')
        tempstmt++;

    char* tempstmte = tempstmt + strlen(tempstmt) -1;
    while(*tempstmte ==' ') {
        *tempstmte = '\0';
        tempstmte --;
    }
    return tempstmt;
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
