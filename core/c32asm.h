#ifndef __C32ASM_H__
#define __C32ASM_H__

#include <chip32.h>
#include <sos_string.h>
#include <rbhash.h>

/*
 * 1234 literal
 * 'a' literal
 * "abc" literal
 * ? auto ram address
 * & get adddress
 * * pointer for address
 * @ data address
 * 
 * declare n0 $1234
 * declare r1 $1235
 * declare r2 $1236
 * declare r3 $
 *
 * move r0, 0
 * move r1, r0
 * move r2, 1235
 * move *r2, r0
 * inc r1
 * int *r2
 * inc *r2
 * move r2, &r3
 * move *r2, 3
 */ 

#define A32_START_CMD_SIZE 1024
#define A32_START_MC_SIZE  1024

#define A32_START_ADDR CHIP32_REG_LAST
#define A32_MAX_TOKEN  64

#define A32_RBH_SIZE 4096
#define A32_RBH_MIN  10
#define A32_RBH_KEY  64


#define A32_ARG1F_SET(F, V) do{ (F) |= ((V) & 3) << 2; } while(0)
#define A32_ARG0F_SET(F, V) do{ (F) |= (V) & 3; } while(0)
#define A32_ARG1F_GET(F) (((F) >> 2) & 3)
#define A32_ARG0F_GET(F) ((F) & 3)


typedef enum {
	TK_ERROR, 
	TK_LITERAL_NUM, 
	TK_LITERAL_CHAR,
	TK_LITERAL_STR,
	TK_AUTO,
	TK_RAM,
	TK_ADDR,
	TK_PTR,
	TK_DATA,
	TK_NAME,
	TK_LABEL
}tktype_e;

typedef struct lextoken{
	size_t line;
	tktype_e type;
	const char* start;
	const char* end;
}lextoken_s;

typedef struct lexcmd{
	lextoken_s tokens[A32_MAX_TOKEN];
	size_t ntoken;
}lexcmd_s;

typedef int(*action_f)(asmchip32_s* ac, lexcmd_s* cmd);

typedef enum { DEC_TYPE_LABEL, DEC_TYPE_LITERAL_NUM } declared_e;

typedef struct declared{
	declared_e type;
	uint32_t addr;
	const char* name;
	size_t size;
}declared_s;

typedef struct asmchip32{
	lexcmd_s* vcmd;
	size_t cmdsize;
	size_t cmdcount;

	chip32Command_s* mc;
	size_t mcsize;
	size_t mccount;
	size_t avaddr;

	rbhash_s* actions;
	rbhash_s* declare;
	
	const char* code;
	const char* parse;

	size_t line;
	char* error;
}asmchip32_s;

int a32_lexer(asmchip32_s* c);
int a32_parser_init(asmchip32_s* c);
int a32_parse(asmchip32_s* c);

#endif
