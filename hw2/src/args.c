#include "../include/debug.h"
#include "../include/utf.h"
#include "../include/wrappers.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int opterr;
int optopt;
int optind;
char *optarg;

state_t *program_state;

void
parse_args(int argc, char *argv[])
{
    int i;
    char option;
    bool is_e_given = false;

    program_state = Calloc(1, sizeof(state_t));
    for (i = 0; optind < argc; ++i) {
        debug("%d opterr: %d", i, opterr);
        debug("%d optind: %d", i, optind);
        debug("%d optopt: %d", i, optopt);
        debug("%d argv[optind]: %s", i, argv[optind]);
        if ((option = (char) getopt(argc, argv, "+e:hi:")) != -1) {
            switch (option) {
                case 'e': {
                    info("Encoding Argument: %s", optarg);
                    if ((program_state->encoding_to = determine_format(optarg)) == 0) {
                        goto errorcase;
                    } else {
                        is_e_given = true;
                    }
                    break;
                }
                case 'h': {
                    USAGE(argv[0]);
                    exit(EXIT_SUCCESS);
                }
                case '?': {
                    errorcase:
                        USAGE(argv[0]);
                        exit(EXIT_FAILURE);

                }
                default: {
                    break;
                }
            }
        }
        elsif(argv[optind] != NULL)
        {
            if (program_state->in_file == NULL) {
                program_state->in_file = argv[optind];
            }
            elsif(program_state->out_file == NULL)
            {
                program_state->out_file = argv[optind];
            }
            optind++;
        }
    }
    if(is_e_given == false || program_state->out_file ==NULL || program_state->in_file ==NULL){
        goto errorcase;
    }
}

format_t
determine_format(char *argument)
{
    if (strcmp(argument, STR_UTF16LE) == 0)
        return UTF16LE;
    if (strcmp(argument, STR_UTF16BE) == 0)
        return UTF16BE;
    if (strcmp(argument, STR_UTF8) == 0)
        return UTF8;
    return (format_t) 0;
}

char*
bom_to_string(format_t bom){
    switch(bom){
        case UTF8: return STR_UTF8;
        case UTF16BE: return STR_UTF16BE;
        case UTF16LE: return STR_UTF16LE;
    }
    return "UNKNOWN";
}

char*
join_string_array(int count, char *array[])
{
    int i=0,j=0;
    int len = 0;
    size_t cur_str_len, str_len=0;

    for (j=0;j<count;j++){
        str_len=str_len+strlen(array[j])+1;
    }
    char *ret = malloc(str_len+2);

    for (i = 0; i < count; i++) {
        cur_str_len = strlen(array[i]);
        memecpy(ret + len, array[i], cur_str_len);
        len += cur_str_len;
        memecpy(ret + len, " ", 1);
        len += 1;
    }
    return ret;
}

int
array_size(int count, char *array[])
{
    int i, sum = 1; /* NULL terminator */
    for (i = 0; i < count; ++i) {
        sum += strlen(array[i]);
        ++sum; /* For the spaces */
    }
    return sum+1;
}

void
print_state()
{
    if (program_state == NULL) {
        error("program_state is %p", (void*)program_state);
        exit(EXIT_FAILURE);
    }
    info("program_state {\n"
                 "  format_t encoding_to = 0x%X;\n"
                 "  format_t encoding_from = 0x%X;\n"
                 "  char *in_file = '%s';\n"
                 "  char *out_file = '%s';\n"
                 "};\n",
         program_state->encoding_to, program_state->encoding_from,
         program_state->in_file, program_state->out_file);
}
