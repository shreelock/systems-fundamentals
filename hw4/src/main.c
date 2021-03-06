#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <wait.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>

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
#define sBackTick '`'

//#define CYELLOW "\001\e[0;31m\002"
#define RESET "\001\e[0m\002"
#define RED "\033[1;31m"
#define GRN "\033[1;32m"
#define YEL "\033[1;33m"
#define BLU "\033[1;34m"
#define MAG "\033[1;35m"
#define CYN "\033[1;36m"
#define WHT "\033[1;37m"
#define BWN "\033[0;33m"

struct state {
    char* curr_dir;
    char* prev_dir;
};

#define max_stopped_pids 100
int stopped_pids[max_stopped_pids][2];
char* cmds[max_stopped_pids];
int jobcount = 0;
char* last_command = NULL;
char* color = RESET;

//TODO Part IV
//TODO Piping
//TODO change pgid for the newly spawned group, put that into foreground using tcprepgroup then that will be taken care.
//TODO fix the numbering in jobs
//TODO in pipe, set the pgid of next in pipe process to the pg of the prev one.
//TODO SIGSUSPEND the parent as well after pausing

void init(struct state *s1);

char* get_shell_prompt(struct state *s1);

void process_input(char *mainarg, char *inarg, char *outarg, struct state *currstate, int in, int out);

char* sget_cwd();

char* sget_home();

char * get_help();

void process_io_redirect(char *input, struct state *currentstate, int in, int out);

void print_credits();

void process_pipes(char *input, struct state *currentstate);

char* get_trimmed(char* sentence) ;

char* getColorString(char* colorip) ;

void process_backticks(char* input, struct state *currentstate) ;

char * get_git_status(struct state *currs);

void printjobq(){
    for (int i=0;i<max_stopped_pids && stopped_pids[i][0]!=-1;i++){
        printf("\n%d-%d-%d-%s",i, stopped_pids[i][0], stopped_pids[i][1], cmds[i]);
    }
}

void handler(int sign){
    int status, pid;
    switch(sign){
        case SIGINT:
            break;
        case SIGCHLD:
            //printf("Got SIGCHLD\n");
            waitpid(-1, &status, WNOHANG);
            break;
        case SIGTSTP:
            //printf("\nGot SIGTSTP\n");
            pid = waitpid(-1, &status, WSTOPPED);
            int flag=0;
            if(WIFSTOPPED(status)) {
                for (int i =0 ;i<max_stopped_pids && stopped_pids[i][0] != -1; i ++) {
                    if (stopped_pids[i][1]==pid){
                        stopped_pids[i][0]=0;
                        flag=1;
                        //printf("Flagged\n");
                        //printjobq();
                        break;
                    }
                }
                if(flag==0) {
                    //printf("Stopped ID = %d\n", pid);
                    stopped_pids[jobcount][0] = 0;
                    stopped_pids[jobcount][1] = pid;
                    //printf("lc--->%s\n", last_command);
                    cmds[jobcount] = strdup(last_command);
                    //printf("in pid %d ----->%s\n",getpid(),cmds[jobcount]);
                    //printf("created new entry\n");
                    //printjobq();
                    jobcount++;
                }
            }
            break;
        default:
            break;
    }
}

void right_prompt(){
    time_t rawtime;
    struct tm *info;
    char buffer[20];
    time( &rawtime );
    info = localtime( &rawtime );
    strftime(buffer,20,"%a %b %e, %I:%M%p", info);

    //char* prevline = "\e[F";
    //char* curpos = "\e[E";

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    //printf ("lines %d\n", w.ws_row);
    //printf ("columns %d\n", w.ws_col);
    char* demo_pos = "\e[150G";
    char* prevline = "\e[0G";

    char* curpos = malloc((strlen(demo_pos)+1)*sizeof(char));
    int finalpos = (int) (w.ws_col - strlen(buffer) + 1);
    sprintf(curpos, "%s%d%s", "\e[", finalpos,"G");

    char* strin = malloc(sizeof(char)*(strlen(buffer)+strlen(prevline)+strlen(curpos)+2));
    sprintf(strin,"%s%s%s",curpos, buffer, prevline);
    write(STDOUT_FILENO, strin, strlen(strin));
    free(strin);
    free(curpos);
}

int main(int argc, char *argv[], char* envp[]) {
    char* input;
    struct state curr_state;
    init(&curr_state);
    bool exited = false;
    signal(SIGINT, handler);
    signal(SIGCHLD, handler);
    signal(SIGTSTP, handler);

    for (int i=0;i<max_stopped_pids;i++)
        stopped_pids[i][0]=-1;

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
        char* gitstatus = get_git_status(&curr_state);
        char* colored_prompt=malloc(sizeof(char)*(strlen(prompt)+strlen(gitstatus)+strlen(color)+strlen(RESET)+1));
        sprintf(colored_prompt, "%s%s%s%s", color,gitstatus, prompt, RESET);
        //get_git_status(&curr_state);
        right_prompt();
        input = readline(colored_prompt);
        //printf("\nGot input as  : %s",input);
        if(input == NULL || strcmp(input,"")==0) {
            printf("\n");
            continue;
        }
        char* inputcopy = strdup(input);
        process_backticks(inputcopy, &curr_state);
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

void process_input(char *mainarg, char *inarg, char *outarg, struct state *currstate, int in, int out) {
    //-------------------------------------------------
    int nargs = 1;
    char *tmp = strdup(mainarg);
    char *tmpcopy = strdup(mainarg);
    char *tmpcopy2 = strdup(mainarg);
    char* tempchr = strtok(tmp, " ");
    while ((tempchr = strtok(NULL, " ")) != NULL) {
        nargs++;
    }

    char* first_word = strtok(tmpcopy2, " ");
    if (strcmp(first_word,"exit") ==0) {
        exit(0);
    }
/*
    else if (strcmp (first_word, "pwd") == 0) {
        puts(currstate->curr_dir);
    }

    else if ( strcmp(first_word, "help") == 0 ) {
        puts(get_help());
    }
*/
    else if ( strcmp(first_word, "jobs") == 0) {
        for (int i = 0; i<max_stopped_pids && stopped_pids[i][0]!=-1; i++) {
            if(stopped_pids[i][0]==0)//Show Stopped processes only
                printf(JOBS_LIST_ITEM, i+1, cmds[i]);
        }
    }

    else if (strcmp(first_word, "credits") == 0){
        print_credits();
    }

    else if (strcmp(first_word, "color") == 0){
        first_word = strtok(NULL, " ");
        color = strcmp(getColorString(first_word),RESET)!=0 ? getColorString(first_word):color;
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

    else if (strcmp(first_word, "fg") == 0 ) {
        switch (nargs) {
            case 1://Only fg
                printf(BUILTIN_ERROR, "Usage fg [jobid6]");
                break;
            case 2:
                first_word = strtok(NULL, " ");
                if (*first_word=='%'){
                    //printf("--%c",*(first_word+1));
                    int jid = atoi(first_word+1)-1;
                    //printf("JID = %d\n", jid);
                    //printjobq();
                    if(stopped_pids[jid][0]==0){
                        kill(stopped_pids[jid][1], SIGCONT);
                        stopped_pids[jid][0]=1;
                        //printjobq();
                    } else {
                        printf(BUILTIN_ERROR, "Usage fg [jobid5]");
                        return;
                    }
                    break;
                }
            default:
                printf(BUILTIN_ERROR, "Usage fg [jobid4]");
                break;
        }
    }

    else if (strcmp(first_word, "kill") == 0 ) {
        switch (nargs) {
            case 1://Only fg
                printf(BUILTIN_ERROR, "Usage kill [jobid3]");
                return;
            case 2:
                first_word = strtok(NULL, " ");
                if (*first_word=='%'){
                    //printf("--%c",*(first_word+1));
                    int jid = atoi(first_word+1)-1;
                    //printf("\njid=%d", jid);
                    //printjobq();
                    if(stopped_pids[jid][0]!=-1){
                        kill(stopped_pids[jid][1], SIGKILL);
                        stopped_pids[jid][0]=2;
                        //printjobq();
                    } else {
                        printf(BUILTIN_ERROR, "Usage kill [jobid2]");
                        return;
                    }
                    break;
                } else {
                    int pid = atoi(first_word);
                    for (int i=0;i<max_stopped_pids && stopped_pids[i][0]!=-1;i++){
                        if(pid == stopped_pids[i][1] && stopped_pids[i][0]!=2){
                            kill(stopped_pids[i][1],SIGKILL);
                            stopped_pids[i][0]=2;
                            return;
                        }
                    }
                }
            default:
                printf(BUILTIN_ERROR, "Usage kill [jobid1]");
                break;
        }
    }


    else {
        //int child_status;
        //Since fist_word pointer is iterator.
        char* command = strdup(first_word);
        last_command = strdup(tmpcopy);
        int pid = fork();
        if (pid==-1){
            printf(EXEC_ERROR, "Fork failed");
            return;
        }
        if (pid == 0) {
            //signal(SIGINT, handler_kids);
            //setpgid(0,0);
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

            //-----------------------------------------------------
            if (inarg!=NULL) {
                //printf("Got redirection : input = _%s_\n", inarg);
                if (access(inarg, F_OK)==-1){
                    printf(SYNTAX_ERROR, FILE_NO_EXIST_ERROR);
                    return;
                }
                int in1 = open(inarg, O_RDONLY);
                dup2(in1, STDIN_FILENO);
                close(in1);
            }
            if (outarg!=NULL) {
                //printf("Got redirection : output = _%s_\n", outarg);
                int out1 = creat(outarg, 0664);
                dup2(out1, STDOUT_FILENO);
                close(out1);
            }

            //-----------------------------------------------------
            if(in != STDIN_FILENO ){
                dup2(in, STDIN_FILENO);
                close(in);
            }
            if(out != STDOUT_FILENO ){
                dup2(out, STDOUT_FILENO);
                close(out);
            }
            //-----------------------------------------------------


            if (strcmp (command, "pwd") == 0) {
                //char *ret = malloc((strlen(currstate->curr_dir) + 2)*sizeof(char));
                //sprintf(ret, "%s", currstate->curr_dir);
                puts(currstate->curr_dir);
            }

            else if ( strcmp(command, "help") == 0 ) {
                //printf("%s\n", "This is the help menu!!");
                //get_help();
                puts(get_help());
            }

            else {
                int execvp_ret = execvp(command, argsarray);
                if (execvp_ret == -1) {
                    printf(EXEC_NOT_FOUND, command);
                    kill(getpid(), SIGTERM);
                }
            }
            free(argsarray);
            exit(0);
        } else {
            pause();
        }
    }
}

void process_io_redirect(char *input, struct state *currentstate, int in, int out) {
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
    process_input(mainarg, inarg, outarg, currentstate, in, out);
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
    //printf("\nnvars = %d\n", nvars);
    int in = STDIN_FILENO, pd[2];
    for (int k=0; k < nvars-1 ; k++){
        pipe(pd);
        char* expr = exprsarray[k];
        process_io_redirect(expr, currentstate, in, pd[1]);
          close(pd[1]);
          in = pd[0];
    }
    process_io_redirect(exprsarray[nvars - 1], currentstate, in, 1);
}

void process_backticks(char* input, struct state *currentstate) {
    char* inputcp = strdup(input);
    inputcp = get_trimmed(inputcp);
    int hasbacktick = 0;

    for(int i=0;i<strlen(inputcp);i++) {
        if(inputcp[i]==sBackTick)
            hasbacktick++;
    }

    if(hasbacktick==2) {
        char *word = strtok(inputcp, "`");
        char **exprsarray = NULL;
        int nvars = 0;
        while (word) {
            exprsarray = realloc(exprsarray, sizeof(char *) * ++nvars);
            word = get_trimmed(word);
            if (strlen(word) == 0) {
                printf(SYNTAX_ERROR, "line554");
                return;
            }
            exprsarray[nvars - 1] = word;
            //printf("word: .%s.\n", word);
            word = strtok(NULL, "`");
        }
        /*for (int k = 0; k < nvars; k++) {
            printf("%s\n", exprsarray[k]);
        }*/

        char* backticked_expr = get_trimmed(exprsarray[1]);
                                                //printf("-->%s\n", backticked_expr);
        char* pre_backtick_expr = get_trimmed(exprsarray[0]);
                                                //printf("-->%s\n", pre_backtick_expr);
        int in = STDIN_FILENO, fd[2];
        pipe(fd);
                                                //printf("done this\n");
        process_io_redirect(backticked_expr, currentstate, in, fd[1]);
                                                //printf("done this\n");
        close(fd[1]);
                                                //printf("done this\n");
        process_io_redirect(pre_backtick_expr, currentstate, fd[0], STDOUT_FILENO);
                                                //printf("done this\n");


    } else if (hasbacktick==0) {
        process_pipes(inputcp, currentstate);
    } else {
        printf(SYNTAX_ERROR, "Wrong number of backticks");
    }


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

char* get_git_status(struct state *currs){
    char* propmt = malloc(20);
    freopen("/dev/null", "w", stderr);
    char* first_word = "git rev-parse --is-inside-work-tree > tf.sfish";
    process_pipes(first_word, currs);
    freopen("/dev/tty", "w", stderr);
    FILE* pr = fopen("tf.sfish", "r");
    char ch;
    fscanf(pr , "%c", &ch);
    fclose(pr);
    remove("tf.sfish");
    if(ch=='t') {
        char* sec_word = "git status -s | wc -l > gitop.txt";
        process_pipes(sec_word, currs);
        FILE* r = fopen("gitop.txt", "r");
        int num;
        fscanf(r , "%d", &num);
        fclose(r);
        remove("gitop.txt");
        //printf("%d ",num);
        sprintf(propmt,"%d ", num);
        return propmt;
    }
    return "";
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

char * get_help(){
    char* string = "Following are the builtin commands -\n1. help: Print a list of all builtins and their basic usage in a single column.\n2. exit: Exits the shell.\n3. cd: Changes the current working directory of the shell.\n4. pwd: Prints the absolute path of the current working directory.";
    //printf("%s\n",string);
    return string;
}

void print_credits(){
    char*  string = "Thanks to -"\
                    "\n1. https://stackoverflow.com/questions/11515399/implementing-shell-in-c-and-need-help-handling-input-output-redirection"\
                    "\n2. https://stackoverflow.com/questions/11198604/c-split-string-into-an-array-of-strings#11198630"\
                    "\n3. http://developerweb.net/viewtopic.php?id=4881"\
                    "\n4. https://stackoverflow.com/questions/7292642/grabbing-output-from-exec#7292659"\
                    "\n5. https://stackoverflow.com/a/43614414"\
                    "";
    printf("%s\n",string);
}

char* getColorString(char* colorip) {
           if (strcmp(colorip, "RED")==0){
        return RED;
    } else if (strcmp(colorip, "GRN")==0){
        return GRN;
    } else if (strcmp(colorip, "YEL")==0){
        return YEL;
    } else if (strcmp(colorip, "BLU")==0){
        return BLU;
    } else if (strcmp(colorip, "MAG")==0){
        return MAG;
    } else if (strcmp(colorip, "CYN")==0){
        return CYN;
    } else if (strcmp(colorip, "WHT")==0){
        return WHT;
    } else if (strcmp(colorip, "BWN")==0){
        return BWN;
    } else
        return RESET;
}