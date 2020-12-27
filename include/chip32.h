#ifndef __CHIP32_H__
#define __CHIP32_H__

#include <sos_type.h>

//TODO pagging ram

/* RAM::
 * 
 *
 * COMMANDS::
 *  ----------------------------
 * | 12 bytes 96 bits           |
 *  ----------------------------
 * |       0-1        |2-5 |6-10|
 *  ----------------------------
 * |0000|00|00|1bytes |0-31|0-31|
 *  ----------------------------
 * |    |a |a |c      |a   |a   |
 * |    |r |r |o      |r   |r   |
 * |    |g |g |m      |g   |g   |
 * |    |1 |2 |m      |1   |2   |
 * |    |  |  |a      |    |    |
 * |    |t |t |n      |    |    |
 * |    |y |y |d      |    |    |
 * |    |p |p |       |    |    |
 * |    |e |e |       |    |    |
 *  ----------------------------
 * arg type: 
 *	00 0x0  literal
 *	01 0x01 reg
 *	10 0x02 reg is pointer
 *	11 0x03 data
 *
 * declare <name>, <ADDR/?>, (optional<SIZE/?>), (optional<"str"/N,N,N/'c','c','c'>
 *
 * 0x00 nop             ; nothing 
 * 0x01 move rdst, rsrc ; r0 = r1
 * 0x02 inc r           ; ++r;
 * 0x03 dec r           ; --r;
 * 0x04 sum rdst,rsrc   ; r0 += r1
 * 0x05 dif rdst,rsrc   ; r0 -= r1
 * 0x06 mul rdst,rsrc   ; r0 *= r1
 * 0x07 div rdst,rsrc   ; r0 /= r1
 * 0x08 mod rdst,rsrc   ; r0 %= r1
 * 0x09 or rdst,rsrc    ; r0 |= r1
 * 0x0A xor rdst,rsrc   ; r0 ^= r
 * 0x0B and rdst,rsrc   ; r0 &= r1
 * 0x0C not rdst,rsrc   ; r0 ~= r1
 * 0x0D shl rdst,rsrc   ; r0 <<= r1
 * 0x0E shr rdst,rsrc   ; r0 >>= r1
 * 0x0F rol rdst,rsrc   ; r0 = (r0<<r1) | (r0 >> 32-r1)
 * 0x10 ror rdst,rsrc   ; r0 = (r0>>r1) | (r0 << 32-r1)
 * 0x11 bor rdst,rsrc   ; r0 = r0 || r1
 * 0x12 band rdst,rsrc  ; r0 = r0 && r1
 * 0x13 bnot rdst       ; r0 = !r0
 * 0x20 jmp r           ; goto addr
 * 0x20 jmpi r          ; goto curaddr+addr
 * 0x21 ift r radr      ; if( r ) else goto curaddr+addr
 * 0x22 iff r radr      ; if( !r ) else goto curaddr+addr
 * 0x23 call r          ; r()
 * 0x24 ret             ; return
 * 0x25 push r          ; *stack-- =r
 * 0x26 pop r           ; r = *(--stack)
 * 0x30 initr r         ; ram = malloc(r)  requires as first command on data segment
 * 0x40 logi r          ; printf("%d", r)
 * 0x41 logu r          ; printf("%u", r)
 * 0x42 logc r          ; printf("%c", r)
 * 0x43 logs r          ; printf("%s", r)
 * 0x44 logln           ; putchar('\n')
 * 0x50 strcpy rdst,rsrc; strcpy(rdst, rsrc)
 * 
*/

#define CHIP32_STATE_END  0x01
#define CHIP32_STATE_FAIL 0xFF
#define CHIP32_REG_PC   32
#define CHIP32_REG_STK  33
#define CHIP32_REG_FRM  34
#define CHIP32_REG_LAST 35

#define CHIP32_CMD_NOP    0x00
#define CHIP32_CMD_MOVE   0x01
#define CHIP32_CMD_INC    0x02
#define CHIP32_CMD_DEC    0x03
#define CHIP32_CMD_SUM    0x04 
#define CHIP32_CMD_DIF    0x05
#define CHIP32_CMD_MUL    0x06
#define CHIP32_CMD_DIV    0x07
#define CHIP32_CMD_MOD    0x08
#define CHIP32_CMD_OR     0x09
#define CHIP32_CMD_XOR    0x0A
#define CHIP32_CMD_AND    0x0B
#define CHIP32_CMD_NOT    0x0C 
#define CHIP32_CMD_SHL    0x0D 
#define CHIP32_CMD_SHR    0x0E 
#define CHIP32_CMD_ROL    0x0F 
#define CHIP32_CMD_ROR    0x10 
#define CHIP32_CMD_BOR    0x11 
#define CHIP32_CMD_BAND   0x12 
#define CHIP32_CMD_BNOT   0x13 
#define CHIP32_CMD_0x14   0x14
#define CHIP32_CMD_0x1F   0x1F
#define CHIP32_CMD_JMP    0x20 
#define CHIP32_CMD_IFT    0x21 
#define CHIP32_CMD_IFF    0x22 
#define CHIP32_CMD_CALL   0x23 
#define CHIP32_CMD_RET    0x24 
#define CHIP32_CMD_PUSH   0x25 
#define CHIP32_CMD_POP    0x26
#define CHIP32_CMD_0x27   0x27
#define CHIP32_CMD_0x2F   0x2F
#define CHIP32_CMD_INITR  0x30
#define CHIP32_CMD_0x31   0x31
#define CHIP32_CMD_0x3F   0x3F
#define CHIP32_CMD_LOGI   0x40 
#define CHIP32_CMD_LOGU   0x41 
#define CHIP32_CMD_LOGC   0x42 
#define CHIP32_CMD_LOGS   0x43
#define CHIP32_CMD_LOGLN  0x44
#define CHIP32_CMD_0x45   0x45
#define CHIP32_CMD_0x4F   0x4F
#define CHIP32_CMD_STRCPY 0x50
#define CHIP32_CMD_0x51   0x51
#define CHIP32_CMD_0x5F   0x5F
#define CHIP32_CMD_0x60   0x60
#define CHIP32_CMD_0x6F   0x6F
#define CHIP32_CMD_0x70   0x70
#define CHIP32_CMD_0x7F   0x7F
#define CHIP32_CMD_0x80   0x80
#define CHIP32_CMD_0x8F   0x8F
#define CHIP32_CMD_0x90   0x90
#define CHIP32_CMD_0x9F   0x9F
#define CHIP32_CMD_0xA0   0xA0
#define CHIP32_CMD_0xAF   0xAF
#define CHIP32_CMD_0xB0   0xB0
#define CHIP32_CMD_0xBF   0xBF
#define CHIP32_CMD_0xC0   0xC0
#define CHIP32_CMD_0xCF   0xCF
#define CHIP32_CMD_0xD0   0xD0
#define CHIP32_CMD_0xDF   0xDF
#define CHIP32_CMD_0xE0   0xE0
#define CHIP32_CMD_0xEF   0xEF
#define CHIP32_CMD_0xF0   0xF0
#define CHIP32_CMD_0xFF   0xFF

#define CHIP32_TYPE_LITERAL 0
#define CHIP32_TYPE_RAM     1
#define CHIP32_TYPE_PTR     2
#define CHIP32_TYPE_DATA    3


typedef struct chip32Command chip32Command_s;
typedef struct chip32 chip32_s;

typedef ssize_t(*chipLoadData_f)(void* ctx, uint32_t addr, chip32Command_s* data, uint32_t size);
typedef void(*chipCmd_f)(chip32_s* c, chip32Command_s* cmd);

typedef struct chip32Command{
	uint8_t  flags;
	uint8_t  cmd;
	int32_t arg0;
	int32_t arg1;
}__packed chip32Command_s;

typedef struct chip32{
	int32_t* ram;
	uint32_t ramsize;

	chip32Command_s* data;
	uint32_t pagecurrent;
	uint32_t pagesize;
	void* dataCtx;

	chipLoadData_f dataLoad;

	uint32_t state;
}chip32_s;

typedef struct asmchip32 asmchip32_s;

chip32_s* chip32_new(uint32_t pagesize, chipLoadData_f ld, void* dataCtx);
void chip32_free(chip32_s* c);
int chip32_run(chip32_s* c);

asmchip32_s* chip32_compiler_new(void);
void chip32_compiler_free(asmchip32_s* ac);
void chip32_compiler_error_print(asmchip32_s* ac);
int chip32_compiler_build(asmchip32_s* ac, const char* code);
void chip32_lex_dump(asmchip32_s* ac);
void chip32_command_dump(chip32Command_s* cmd, size_t size);
chip32Command_s* chip32_commands(asmchip32_s* c, size_t* count);
int chip32_commands_save(asmchip32_s* c, FILE* f);
int chip32_command_load(chip32Command_s* cmd, FILE* f);

#endif
