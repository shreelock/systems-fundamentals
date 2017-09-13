#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "wrappers.h"

#define TOP_TWO_BYTES_FROM_THREE(b) ((b & 0xFFFF00) >> 8)
#define LOWER_TWO_BYTES(b) ((b & 0xFFFF))
#define SURROGATE_PAIR 0x10000

#define AS_BYTE(x) ((char*)x)
#define AS_GLYF(x) ((utf8_glyph_t*)x)

const char *STR_UTF16BE  = "UTF16BE";
char *const STR_UTF16LE = "UTF16LE";
char const *STR_UTF8  = "UTF8";

typedef enum { UTF16LE = 0xFFFE, UTF16BE = 0xFEFF, UTF8 = 0xBFBBEF } format_t;

typedef enum {
  utf8_to_utf16le = (UTF8 - UTF16LE),
  utf8_to_utf16be = (UTF8 - UTF16BE),
  utf16le_to_utf16be = (UTF16LE - UTF16BE),
  utf16be_to_utf16le = (UTF16BE - UTF16LE),
  utf16be_to_utf8 = (UTF16BE - UTF8),
  utf16le_to_utf8 = (UTF16LE - UTF8),
  transcribe_file = 0
} encoding_from_to_t;

typedef struct
{
  format_t encoding_to;
  format_t encoding_from;
  int bom_length;
  char *in_file;
  char *out_file;
} state_t;

extern state_t *program_state;

typedef union
{
  struct
  {
    uint8_t remaining : 7;
    uint8_t bit : 1;
  } top_one;

  struct
  {
    uint8_t remaining : 6;
    uint8_t bits : 2;
  } top_two;

  struct
  {
    uint8_t remaining : 5;
    uint8_t bits : 3;
  } top_three;

  struct
  {
    uint8_t remaining : 4;
    uint8_t bits : 4;
  } top_four;

  struct
  {
    uint8_t remaining : 3;
    uint8_t bits : 5;
  } top_five;

  uint8_t byte;

}  utf8_byte_t;

typedef struct {
  utf8_byte_t bytes[4];
} utf8_glyph_t;

typedef struct
{
  uint16_t upper_bytes;
  uint16_t lower_bytes;
} utf16_glyph_t;

typedef uint32_t code_point_t;

/* Conversion Function type, conversion functions and getter */
typedef int (*convertion_func_t)(int infile, int outfile);

int from_utf8_to_utf16le(int infile, int outfile);
int from_utf8_to_utf16be(int infile, int outfile);
int from_utf16be_to_utf16le(int infile, int outfile);
int from_utf16le_to_utf16be(int infile, int outfile);
int from_utf16le_to_utf8(int infile, int outfile);
int from_utf16be_to_utf8(int infile, int outfile);
int transcribe(int infile, int outfile);

convertion_func_t get_encoding_function();

/* UTF8 Encoding Function Type, encoding functions, and getter */
typedef utf8_glyph_t (*utf8_encoding_func_t)(code_point_t code_point);

utf8_glyph_t utf8_one_byte_encode(code_point_t code_point);
utf8_glyph_t utf8_two_byte_encode(code_point_t code_point);
utf8_glyph_t utf8_three_byte_encode(code_point_t code_point);
utf8_glyph_t utf8_four_byte_encode(code_point_t code_point);

utf8_encoding_func_t get_utf8_encoding_function(size_t size);

/* UTF8 Decoding Function Type, decoding functions and getter */
typedef code_point_t (*utf8_decoding_func_t)(utf8_glyph_t);

code_point_t utf8_one_byte_decode(utf8_glyph_t glyph);
code_point_t utf8_two_byte_decode(utf8_glyph_t glyph);
code_point_t utf8_three_byte_decode(utf8_glyph_t glyph);
code_point_t utf8_four_byte_decode(utf8_glyph_t glyph);

utf8_decoding_func_t get_utf8_decoding_function(size_t size);

bool is_upper_surrogate_pair(utf16_glyph_t glyph);
bool is_lower_surrogate_pair(utf16_glyph_t glyph);

bool is_code_point_surrogate(code_point_t code_point);

void check_bom();

void parse_args(int argc, char *argv[]);

char *join_string_array(int argc, char *argv[]);

int array_size(int count, char *array[]);

format_t determine_format(char *argument);

code_point_t utf16_glyph_to_code_point(utf16_glyph_t *glyph);

utf8_glyph_t code_point_to_utf8_glyph(code_point_t code_point, size_t *size_of_glyph);

size_t utf8_glyph_size_of_code_point(code_point_t code_point);

void print_state();

char *bom_to_string(format_t bom);

size_t remaining_utf8_bytes(utf8_byte_t first_byte);

utf16_glyph_t code_point_to_utf16le_glyph(code_point_t code_point, size_t *size_of_glyph);
utf16_glyph_t code_point_to_utf16be_glyph(code_point_t code_point, size_t *size_of_glyph);

#define elsif else if

#define USAGE(prog_name)                                                       \
  do {                                                                         \
    fprintf(stderr,                                                            \
            "\n%s [-h] -e ENCODING INPUT_FILE OUTPUT_FILE \n"                  \
            "\n"                                                               \
            "Translates unicode files between utf-8, utf-16le, and utf-16be\n" \
            "\n"                                                               \
            "Option arguments:\n\n"                                            \
            "-h          Displays this usage menu.\n"                          \
            "-e ENCODING MANDATORY FLAG: Choose output format.\n"              \
            "            Accepted values:\n"                                   \
            "             - UTF16LE\n"                                         \
            "             - UTF16BE\n"                                         \
            "             - UTF8\n"                                            \
            "\nPositional arguments:\n\n"                                      \
            "INPUT_FILE  File to convert.\n"                                   \
            "            Must contain a Byte Order Marking (BOM)\n"            \
            "\n"                                                               \
            "OUTPUT_FILE Output file\n"                                        \
            "            Will contain a Byte Order Marking (BOM)\n",           \
            (prog_name));                                                      \
  } while (0)