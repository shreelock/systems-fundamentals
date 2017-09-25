#include "../include/utf.h"
#include "../include/wrappers.h"

int
from_utf16be_to_utf16le(int infile, int outfile)
{
    int bom;
    utf16_glyph_t buf;
    ssize_t bytes_read;
    ssize_t bytes_to_write;
    int ret = -1;

    bom = UTF16LE;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    reverse_bytes(&bom, 3);
#endif
    while ((bytes_read = read_to_bigendian(infile, &(buf.upper_bytes), 2)) > 0) {
        //Right now, irrespective of that first two bytes are a surrogate pair or not, we'll be writing them.
        bytes_to_write = 2;
        printf("\nbuf upper bytes : %x ",buf.upper_bytes);
        reverse_bytes(&buf.upper_bytes, (size_t) bytes_to_write);

        if(is_upper_surrogate_pair(buf)) {
            printf("\nIs surrogate : Upper pair : %x", buf.upper_bytes);

            if(read_to_bigendian(infile, &buf.lower_bytes, 2) > 0) {
                printf("\nbuf lower bytes : %x ", buf.lower_bytes);
                reverse_bytes(&buf.lower_bytes, 2);
                bytes_to_write += 2;
            }
        }
        write_to_bigendian(outfile, &buf, (size_t) bytes_to_write);
    }
    ret = (int) bytes_read;
    return ret;
}



int
from_utf16be_to_utf8(int infile, int outfile)
{
    utf16_glyph_t buf;
    utf8_glyph_t utf8_glyph1;
    code_point_t code_point;
    ssize_t bytes_read;
    int ret = -1;


    while ((bytes_read = read_to_bigendian(infile, &(buf.upper_bytes), 2)) > 0) {
        reverse_bytes(&buf.upper_bytes, sizeof(buf.upper_bytes));
        if(is_upper_surrogate_pair(buf)) {
            if(read_to_bigendian(infile, &buf.lower_bytes, 2) > 0) {
                printf("\nbuf lower bytes : %x ", buf.lower_bytes);
                reverse_bytes(&buf.lower_bytes,sizeof(buf.lower_bytes));
            }
        } else {
            buf.lower_bytes = 0x0000;
        }
        code_point = utf16_glyph_to_code_point(&buf);
        size_t size_of_glyph;
        utf8_glyph1 = code_point_to_utf8_glyph(code_point, &size_of_glyph);
        write_to_bigendian(outfile, &utf8_glyph1, size_of_glyph);
    }
    ret = (int) bytes_read;
    return ret;

}

utf16_glyph_t
code_point_to_utf16be_glyph(code_point_t code_point, size_t *size_of_glyph)
{
    utf16_glyph_t ret;

    memeset(&ret, 0, sizeof ret);
    if(is_code_point_surrogate(code_point)) {
        code_point -= 0x10000;
        ret.upper_bytes = (code_point >> 10) + 0xD800;
        ret.lower_bytes = (code_point & 0x3FF) + 0xDC00;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        reverse_bytes(&ret.upper_bytes, 2);
        reverse_bytes(&ret.lower_bytes, 2);
#endif
        *size_of_glyph = 4;
    }
    else {
        ret.upper_bytes |= code_point;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        reverse_bytes(&ret.upper_bytes, 2);
#endif
        *size_of_glyph = 2;
    }
    return ret;
}
