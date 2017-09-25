#include "../include/debug.h"
#include "../include/utf.h"
#include "../include/wrappers.h"

int
main(int argc, char *argv[])
{
  int infile, outfile, in_flags, out_flags;
  parse_args(argc, argv);
  check_bom();
  print_state();
  in_flags = O_RDONLY;
  out_flags = O_WRONLY | O_CREAT;
  infile = Open(program_state->in_file, in_flags);
  outfile = Open(program_state->out_file, out_flags);
  lseek(SEEK_SET, program_state->bom_length, infile); /* Discard BOM */
  get_encoding_function()(infile, outfile);
  if(program_state != NULL) {
    close(infile);
    close(outfile);
  }
  //I think this is how this works
//  free((int*)outfile);
//  free((int*)infile);
  return EXIT_SUCCESS;
}
