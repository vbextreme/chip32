#include <sos_string.h>

char* str_dup(const char* src, size_t optlen){
	if( optlen == 0 ) optlen = strlen(src);
	char* ret = MANY(char, optlen+1);
	if( ret == NULL ) return NULL;		
	memcpy(ret, src, optlen);
	ret[optlen] = 0;
	return ret;
}

char* str_dup_ch(const char* src, const char ch){
	const char* end = strchr(src, ch);
	if( !end ) return NULL;
	if( end == src ) return NULL;
	return str_dup(src, end-src);
}

int str_equal(char const* a, size_t lena, char const* b, size_t lenb){
	if( lena == 0 ) lena = strlen(a);
	if( lenb == 0 ) lenb = strlen(b);
	if( lena != lenb ) return lena - lenb;
	return memcmp(a,b,lena);
}

const char* str_skip_h(const char* str) {
	while( *str && (*str == ' ' || *str == '\t') ) ++str;
	return str;
}

const char* str_skip_hn(const char* str){
	while( *str && (*str == ' ' || *str == '\t' || *str == '\n') ) ++str;
	return str;
}

const char* str_next_line(const char* str){
	while( *str && *str != '\n' ) ++str;
	if( *str ) ++str;
	return str;
}

char* str_ncpy(char* dst, size_t lend, const char* src, size_t lens){
	--lend;
	if( lens == 0 ) lens = strlen(src);

	while( lens-->0 && lend-->0 && *src){
		*dst++=*src++;
	}
	*dst = 0;
	return dst;
}

char* str_cpy(char* dst, size_t lend, const char* src){
	--lend;
	while( lend-->0 && *src ){
		*dst++=*src++;
	}
	*dst = 0;
	return dst;
}

const char* str_chr(const char* str, const char ch){
	const char* ret = strchr(str, ch);
	if( ret == NULL ){
		ret = &str[strlen(str)];
	}
	return ret;
}

char* str_vprintf(const char* format, va_list va1, va_list va2){
	size_t len = vsnprintf(NULL, 0, format, va1);
	if( len == 0 ) return NULL;

	char* ret = MANY(char, len+1);
	if( !ret ) return NULL;
	
	vsprintf(ret, format, va2);
	return ret;
}

char* str_printf(const char* format, ...){
	va_list va1,va2;
	va_start(va1, format);
	va_start(va2, format);
	char* ret = str_vprintf(format, va1, va2);
	va_end(va1);
	va_end(va2);
	return ret;
}

void str_swap(char* a, char* b){
	for(; *a && *b; ++a, ++b ){
		SWAP(*a, *b);
	}
	if( *a == *b ) return;
	if( *a == 0 ){
		strcpy(a,b);
		*b = 0;
	}
	else{
		strcpy(b,a);
		*a = 0;
	}
}

long str_chomp(char* str){
	const long len = strlen(str);
	if( len > 1 && str[len-1] == '\n' ){
		str[len-1] = 0;
		return len-1;
	}
	return -1;
}

char* quote_printable_decode(size_t *len, const char* str){
	size_t strsize = strlen(str);
	char* ret = MANY(char, strsize + 1);
	if( !ret ) return NULL;

	char* next = ret;
	while( *str ){
		if( *str != '=' ){
			*next++ = *str++;
		}
		else{
			++str;
			if( *str == '\r' ) ++str;
			if( *str == '\n' ){
				++str;
				continue;
			}	
			char val[3];
			val[0] = *str++;
			val[1] = *str++;
			val[2] = 0;
			*next++ = strtoul(val, NULL, 16);
		}
	}
	*next = 0;
	if( len ) *len = next - ret;
	return ret;
}

void str_toupper(char* dst, const char* src){
	while( (*dst++=toupper((unsigned)*src++)) );
}

void str_tolower(char* dst, const char* src){
	while( (*dst++=toupper((unsigned)*src++)) );
}

void str_tr(char* str, const char* find, const char replace){
	while( (str=strpbrk(str,find)) ) *str++ = replace;
}

__private const unsigned char base64et[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
__private const unsigned char base64dt[80] = {
	['A' - '+'] = 0,  ['B' - '+'] = 1,  ['C' - '+'] = 2,  ['D' - '+'] = 3,	['E' - '+'] = 4,  ['F' - '+'] = 5,  ['G' - '+'] = 6,
	['H' - '+'] = 7,  ['I' - '+'] = 8,  ['J' - '+'] = 9,  ['K' - '+'] = 10, ['L' - '+'] = 11, ['M' - '+'] = 12, ['N' - '+'] = 13,
	['O' - '+'] = 14, ['P' - '+'] = 15, ['Q' - '+'] = 16, ['R' - '+'] = 17, ['S' - '+'] = 18, ['T' - '+'] = 19,	['U' - '+'] = 20, 
	['V' - '+'] = 21, ['W' - '+'] = 22, ['X' - '+'] = 23, ['Y' - '+'] = 24, ['Z' - '+'] = 25, 
	['a' - '+'] = 26, ['b' - '+'] = 27, ['c' - '+'] = 28, ['d' - '+'] = 29, ['e' - '+'] = 30, ['f' - '+'] = 31, ['g' - '+'] = 32, 
	['h' - '+'] = 33, ['i' - '+'] = 34, ['j' - '+'] = 35, ['k' - '+'] = 36, ['l' - '+'] = 37, ['m' - '+'] = 38, ['n' - '+'] = 39,
	['o' - '+'] = 40, ['p' - '+'] = 41, ['q' - '+'] = 42, ['r' - '+'] = 43,	['s' - '+'] = 44, ['t' - '+'] = 45,	['u' - '+'] = 46, 
	['v' - '+'] = 47, ['w' - '+'] = 48, ['x' - '+'] = 49, ['y' - '+'] = 50, ['z' - '+'] = 51, 
	['0' - '+'] = 52, ['1' - '+'] = 53, ['2' - '+'] = 54, ['3' - '+'] = 55, ['4' - '+'] = 56, ['5' - '+'] = 57, ['6' - '+'] = 58, 
	['7' - '+'] = 59, ['8' - '+'] = 60, ['9' - '+'] = 61,
	['+' - '+'] = 62, ['/' - '+'] = 63
};

char* base64_encode(const void* src, const size_t size){
	size_t una = size % 3;
	size_t ali = (size - una) / 3;
	una = 3 - una;
	const unsigned char* data = src;
	const size_t outsize = (size * 4) / 3 + 4;
	char* ret = MANY(char, outsize+1);
	if( !ret ) return NULL;

	char* next = ret;

	while( ali --> 0 ){
		*next++ = base64et[data[0] >> 2];
		*next++ = base64et[((data[0] & 0x03) << 4) | (data[1] >> 4)];
		*next++ = base64et[((data[1] & 0x0F) << 2) | (data[2] >> 6)];
		*next++ = base64et[data[2] & 0x3F];
		data += 3;
	}
	
	switch( una ){
		case 1:
			*next++ = base64et[data[0] >> 2];
			*next++ = base64et[((data[0] & 0x03) << 4) | (data[1] >> 4)];
			*next++ = base64et[((data[1] & 0x0F) << 2)];
			*next++ = '=';
		break;

		case 2:
			*next++ = base64et[data[0] >> 2];
			*next++ = base64et[((data[0] & 0x03) << 4)];
			*next++ = '=';
			*next++ = '=';
		break;
	}

	*next = 0;

	return ret;
}

void* base64_decode(size_t* size, const char* b64){
	const unsigned char* str = (const unsigned char*)b64;
	const size_t len = strlen(b64);
	size_t una = 0;
	if( str[len - 1] == '=' ) ++una;
	if( str[len - 2] == '=' ) ++una;
	
	const size_t countali = (len / 4) * 3 - una;
	if( size ) *size = countali;

	void* data = MANY(char, countali);
	if( !data )	return NULL;
	char* next = data;

	size_t count = (len / 4) - 1;

	while( count --> 0 ){
		*next++ = (base64dt[str[0] - '+'] << 2) | (base64dt[str[1] - '+'] >> 4);
		*next++ = (base64dt[str[1] - '+'] << 4) | (base64dt[str[2] - '+'] >> 2);
		*next++ = (base64dt[str[2] - '+'] << 6) |  base64dt[str[3] - '+'];
		str += 4;
	}

	switch( una ){
		case 0:
			*next++ = (base64dt[str[0] - '+'] << 2) | (base64dt[str[1] - '+'] >> 4);
			*next++ = (base64dt[str[1] - '+'] << 4) | (base64dt[str[2] - '+'] >> 2);
			*next++ = (base64dt[str[2] - '+'] << 6) |  base64dt[str[3] - '+'];
		break;
		case 1:
			*next++ = (base64dt[str[0] - '+'] << 2) | (base64dt[str[1] - '+'] >> 4);
			*next++ = (base64dt[str[1] - '+'] << 4) | (base64dt[str[2] - '+'] >> 2);
		break;
		case 2:
			*next++ = (base64dt[str[0] - '+'] << 2) | (base64dt[str[1] - '+'] >> 4);
		break;
	}
	
	return data;
}

