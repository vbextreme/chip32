#ifndef __SOS_STRING_H__
#define __SOS_STRING_H__

#include <sos_type.h>

__extern_begin

char* str_dup(const char* src, size_t optlen);
char* str_dup_ch(const char* src, const char ch);
int str_equal(char const* a, size_t lena, char const* b, size_t lenb);
const char* str_skip_h(const char* str);
const char* str_skip_hn(const char* str);
const char* str_next_line(const char* str);
char* str_ncpy(char* dst, size_t lend, const char* src, size_t lens);
char* str_cpy(char* dst, size_t lend, const char* src);
const char* str_chr(const char* str, const char ch);
char* str_vprintf(const char* format, va_list va1, va_list va2);
char* str_printf(const char* format, ...);
void str_swap(char* a, char* b);
long str_chomp(char* str);
char* quote_printable_decode(size_t *len, const char* str);
void str_toupper(char* dst, const char* src);
void str_tolower(char* dst, const char* src);
void str_tr(char* str, const char* find, const char replace);
char* base64_encode(const void* src, const size_t size);
void* base64_decode(size_t* size, const char* b64);

__extern_end
#endif
