#include <chip32.h>

chip32_s* chip32_new(uint32_t pagesize, chipLoadData_f ld, void* dataCtx){
	chip32_s* c = NEW(chip32_s);
	if( !c ) return NULL;
	
	c->state = 0;
	
	c->ram = NULL;
	c->ramsize = 0;
	
	c->dataLoad = ld;
	c->dataCtx  = dataCtx;
	c->pagesize = pagesize;
	c->pagecurrent = 0;
	c->data     = MANY(chip32Command_s, c->pagesize);
	if( !c->data ){
		free(c->data);
		free(c);
		return NULL;
	}
	return c;
}

void chip32_free(chip32_s* c){
	if( c->data ) free(c->data);
	if( c->ram ) free(c->ram);
	free(c);
}

#define A32_ARG1F_GET(F) (((F) >> 2) & 3)
#define A32_ARG0F_GET(F) ((F) & 3)

__private int32_t chip32_getword(chip32_s* c, uint8_t type, int32_t r){
	switch( type ){
		case CHIP32_TYPE_RAM: return c->ram[r];
		case CHIP32_TYPE_PTR: return c->ram[c->ram[r]];
		case CHIP32_TYPE_LITERAL: return r;
		case CHIP32_TYPE_DATA:{
			chip32Command_s tmp;
			if( c->dataLoad(c->dataCtx, r, &tmp, 1) != 1 ){
				c->state = CHIP32_STATE_FAIL;
			}
			return tmp.arg0;
		}
	}
	c->state = CHIP32_STATE_FAIL;
	return 0;
}

__private void chip32_setword(chip32_s* c, uint8_t type, int32_t r, int32_t v){
	switch( type ){
		case CHIP32_TYPE_RAM: c->ram[r] = v; break;
		case CHIP32_TYPE_PTR: c->ram[c->ram[r]] = v; break;
		default: case CHIP32_TYPE_LITERAL: case CHIP32_TYPE_DATA: c->state = CHIP32_STATE_FAIL; puts("setword fail"); break;
	}
}

__private void chip32_cmd_nop(__unused chip32_s* c, __unused chip32Command_s* pcmd){}

__private void chip32_cmd_move(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a1, pcmd->arg1));
}

__private void chip32_cmd_inc(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a0, pcmd->arg0) + 1);
}

__private void chip32_cmd_dec(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a0, pcmd->arg0) - 1);
}

__private void chip32_cmd_sum(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a0, pcmd->arg0) + chip32_getword(c, a1, pcmd->arg1));
}

__private void chip32_cmd_dif(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a0, pcmd->arg0) - chip32_getword(c, a1, pcmd->arg1));
}

__private void chip32_cmd_mul(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a0, pcmd->arg0) * chip32_getword(c, a1, pcmd->arg1));
}

__private void chip32_cmd_div(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a0, pcmd->arg0) / chip32_getword(c, a1, pcmd->arg1));
}

__private void chip32_cmd_mod(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a0, pcmd->arg0) % chip32_getword(c, a1, pcmd->arg1));
}

__private void chip32_cmd_or(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a0, pcmd->arg0) | chip32_getword(c, a1, pcmd->arg1));
}

__private void chip32_cmd_xor(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a0, pcmd->arg0) ^ chip32_getword(c, a1, pcmd->arg1));
}

__private void chip32_cmd_and(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a0, pcmd->arg0) & chip32_getword(c, a1, pcmd->arg1));
}

__private void chip32_cmd_not(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, ~chip32_getword(c, a0, pcmd->arg0));
}

__private void chip32_cmd_shl(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a0, pcmd->arg0) << chip32_getword(c, a1, pcmd->arg1));
}

__private void chip32_cmd_shr(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a0, pcmd->arg0) >> chip32_getword(c, a1, pcmd->arg1));
}

__private void chip32_cmd_rol(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	uint32_t v = chip32_getword(c, a0, pcmd->arg0);
	uint32_t r = chip32_getword(c, a1, pcmd->arg1);
	v = (v << r) | (v >> (32 - r));
	chip32_setword(c, a0, pcmd->arg0, v);
}

__private void chip32_cmd_ror(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	uint32_t v = chip32_getword(c, a0, pcmd->arg0);
	uint32_t r = chip32_getword(c, a1, pcmd->arg1);
	v = (v << r) | (v >> (32 - r));
	chip32_setword(c, a0, pcmd->arg0, v);
}

__private void chip32_cmd_bor(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a0, pcmd->arg0) || chip32_getword(c, a1, pcmd->arg1));
}

__private void chip32_cmd_band(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, chip32_getword(c, a0, pcmd->arg0) && chip32_getword(c, a1, pcmd->arg1));
}

__private void chip32_cmd_bnot(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, !chip32_getword(c, a0, pcmd->arg0));
}

__private void chip32_cmd_jmp(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint32_t v = chip32_getword(c, a0, pcmd->arg0);
	c->ram[CHIP32_REG_PC] = v-1;
}

__private void chip32_cmd_ift(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	int32_t v = chip32_getword(c, a0, pcmd->arg0);
	if( !v ) c->ram[CHIP32_REG_PC] = chip32_getword(c, a1, pcmd->arg1) - 1;
}

__private void chip32_cmd_iff(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG1F_GET(pcmd->flags);
	int32_t v = chip32_getword(c, a0, pcmd->arg0);
	if( v ) c->ram[CHIP32_REG_PC] = chip32_getword(c, a1, pcmd->arg1) - 1;
}

__private void chip32_cmd_call(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint32_t v = chip32_getword(c, a0, pcmd->arg0);
	c->ram[c->ram[CHIP32_REG_STK]--] = c->ram[CHIP32_REG_PC];
	c->ram[c->ram[CHIP32_REG_STK]--] = c->ram[CHIP32_REG_FRM];
	c->ram[CHIP32_REG_FRM] = c->ram[CHIP32_REG_STK];
	c->ram[CHIP32_REG_PC] = v-1;
}

__private void chip32_cmd_ret(chip32_s* c, __unused chip32Command_s* pcmd){
	c->ram[CHIP32_REG_STK] = c->ram[CHIP32_REG_FRM];
	c->ram[CHIP32_REG_FRM] = c->ram[++c->ram[CHIP32_REG_STK]];
	c->ram[CHIP32_REG_PC] = c->ram[++c->ram[CHIP32_REG_STK]];
}

__private void chip32_cmd_push(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	c->ram[c->ram[CHIP32_REG_STK]--] = chip32_getword(c, a0, pcmd->arg0);
}

__private void chip32_cmd_pop(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	chip32_setword(c, a0, pcmd->arg0, c->ram[++c->ram[CHIP32_REG_STK]]);
}

__private void chip32_cmd_initr(chip32_s* c, chip32Command_s* cmd){
	if( c->ram ){
		c->state = CHIP32_STATE_FAIL;
		return;
	}
	c->ramsize = cmd->arg0;
	if( cmd->arg0 == 0 ){
		c->state = CHIP32_STATE_FAIL;
		return;
	}
	c->ram = MANY(int32_t, c->ramsize);
	if( !c->ram ){
		c->state = CHIP32_STATE_FAIL;
		return;
	}
	c->ram[CHIP32_REG_PC]  = 1;
	c->ram[CHIP32_REG_STK] = c->ramsize-1;
	c->ram[CHIP32_REG_FRM] = c->ramsize-1;
}

__private void chip32_cmd_logi(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	int32_t v = chip32_getword(c, a0, pcmd->arg0);
	fprintf(stderr, "%d", v);
}

__private void chip32_cmd_logu(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint32_t v = chip32_getword(c, a0, pcmd->arg0);
	fprintf(stderr, "%u", v);
}

__private void chip32_cmd_logc(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	int32_t v = chip32_getword(c, a0, pcmd->arg0);
	fprintf(stderr, "%c", v);
}

__private void chip32_cmd_logs(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	int32_t v;
   	while( (v=chip32_getword(c, a0, pcmd->arg0++)) ){
		fprintf(stderr, "%c", v);
	}
}

__private void chip32_cmd_logln(__unused chip32_s* c, __unused chip32Command_s* pcmd){
	fprintf(stderr, "%c", '\n');
}

__private void chip32_cmd_strcpy(chip32_s* c, chip32Command_s* pcmd){
	uint8_t a0 = A32_ARG0F_GET(pcmd->flags);
	uint8_t a1 = A32_ARG0F_GET(pcmd->flags);
	uint32_t s = 0;
	
	do{
		s = chip32_getword(c, a1, pcmd->arg1++);
		chip32_setword(c, a0, pcmd->arg0++, s);
	}while(s);
}

__private chipCmd_f commands[] = {
	[CHIP32_CMD_NOP  ] = chip32_cmd_nop,
	[CHIP32_CMD_MOVE ] = chip32_cmd_move,
	[CHIP32_CMD_INC  ] = chip32_cmd_inc,
	[CHIP32_CMD_DEC  ] = chip32_cmd_dec,
	[CHIP32_CMD_SUM  ] = chip32_cmd_sum,
	[CHIP32_CMD_DIF  ] = chip32_cmd_dif,
	[CHIP32_CMD_MUL  ] = chip32_cmd_mul,
	[CHIP32_CMD_DIV  ] = chip32_cmd_div,
	[CHIP32_CMD_MOD  ] = chip32_cmd_mod,
	[CHIP32_CMD_OR   ] = chip32_cmd_or,
	[CHIP32_CMD_XOR  ] = chip32_cmd_xor,
	[CHIP32_CMD_AND  ] = chip32_cmd_and,
	[CHIP32_CMD_NOT  ] = chip32_cmd_not,
	[CHIP32_CMD_SHL  ] = chip32_cmd_shl,
	[CHIP32_CMD_SHR  ] = chip32_cmd_shr,
	[CHIP32_CMD_ROL  ] = chip32_cmd_rol,
	[CHIP32_CMD_ROR  ] = chip32_cmd_ror,
	[CHIP32_CMD_BOR  ] = chip32_cmd_bor,
	[CHIP32_CMD_BAND ] = chip32_cmd_band,
	[CHIP32_CMD_BNOT ] = chip32_cmd_bnot,
	[CHIP32_CMD_0x14 ... CHIP32_CMD_0x1F] = chip32_cmd_nop,

	[CHIP32_CMD_JMP  ] = chip32_cmd_jmp,
	[CHIP32_CMD_IFT  ] = chip32_cmd_ift,
	[CHIP32_CMD_IFF  ] = chip32_cmd_iff,
	[CHIP32_CMD_CALL ] = chip32_cmd_call,
	[CHIP32_CMD_RET  ] = chip32_cmd_ret,
	[CHIP32_CMD_PUSH ] = chip32_cmd_push,
	[CHIP32_CMD_POP  ] = chip32_cmd_pop,
	[CHIP32_CMD_0x27 ... CHIP32_CMD_0x2F] = chip32_cmd_nop,
	
	[CHIP32_CMD_INITR] = chip32_cmd_initr,
	[CHIP32_CMD_0x31 ... CHIP32_CMD_0x3F] = chip32_cmd_nop,
	
	[CHIP32_CMD_LOGI ] = chip32_cmd_logi,
	[CHIP32_CMD_LOGU ] = chip32_cmd_logu,
	[CHIP32_CMD_LOGC ] = chip32_cmd_logc,
	[CHIP32_CMD_LOGS ] = chip32_cmd_logs,
	[CHIP32_CMD_LOGLN ] = chip32_cmd_logln,
	[CHIP32_CMD_0x45 ... CHIP32_CMD_0x4F ] = chip32_cmd_nop,
	
	[CHIP32_CMD_STRCPY ] = chip32_cmd_strcpy,
	[CHIP32_CMD_0x51 ... CHIP32_CMD_0x5F ] = chip32_cmd_nop,
	
	[CHIP32_CMD_0x60 ... CHIP32_CMD_0x6F ] = chip32_cmd_nop,
	
	[CHIP32_CMD_0x70 ... CHIP32_CMD_0x7F ] = chip32_cmd_nop,
	
	[CHIP32_CMD_0x80 ... CHIP32_CMD_0x8F ] = chip32_cmd_nop,
	
	[CHIP32_CMD_0x90 ... CHIP32_CMD_0x9F ] = chip32_cmd_nop,
	
	[CHIP32_CMD_0xA0 ... CHIP32_CMD_0xAF ] = chip32_cmd_nop,
	
	[CHIP32_CMD_0xB0 ... CHIP32_CMD_0xBF ] = chip32_cmd_nop,
	
	[CHIP32_CMD_0xC0 ... CHIP32_CMD_0xCF ] = chip32_cmd_nop,
	
	[CHIP32_CMD_0xD0 ... CHIP32_CMD_0xDF ] = chip32_cmd_nop,
	
	[CHIP32_CMD_0xE0 ... CHIP32_CMD_0xEF ] = chip32_cmd_nop,
	
	[CHIP32_CMD_0xF0 ... CHIP32_CMD_0xFF ] = chip32_cmd_nop
};

int chip32_run(chip32_s* c){
	ssize_t ncmds;
	if( (ncmds=c->dataLoad(c->dataCtx, 0, c->data, c->pagesize)) < 0 ){
		c->state = CHIP32_STATE_FAIL;
		puts("fail load 0");
		return -1;
	}
	c->pagecurrent = 0;
	if( c->data[0].cmd != CHIP32_CMD_INITR ){
		c->state = CHIP32_STATE_FAIL;
		puts("fail initr");
		return -1;
	}
	chip32_cmd_initr(c, c->data);
	if( c->state == CHIP32_STATE_FAIL ) return -1;

	while( c->state != CHIP32_STATE_END && c->state != CHIP32_STATE_FAIL ){
		size_t pc = c->ram[CHIP32_REG_PC];
		uint32_t page = pc / c->pagesize;
		if( page != c->pagecurrent ){
			printf("vm: reload page %u current:%u size:%u\n", page, c->pagecurrent, c->pagesize); 
			c->pagecurrent = page;
		   	if( (ncmds=c->dataLoad(c->dataCtx, page, c->data, c->pagesize)) < 0){
				c->state = CHIP32_STATE_FAIL;
				break;
			}
		}
		if( (ssize_t)pc % (ssize_t)c->pagesize >= ncmds ){
			//printf("ncmds:%lu pc:%lu ps:%u mod:%lu\n", ncmds, pc, c->pagesize, pc%c->pagesize); 
			break;
		}
		commands[c->data[pc].cmd](c, &c->data[pc]);
		++c->ram[CHIP32_REG_PC];
	}

	return c->state == CHIP32_STATE_FAIL ? -1 : 0;	
}

void chip32_command_dump(chip32Command_s* cmd, size_t size){
	__private struct sdump{
		char* name;
		int narg;
	}dump[]	= {
		[CHIP32_CMD_NOP ] = {"nop", 0},
		[CHIP32_CMD_MOVE] = {"move",2},
		[CHIP32_CMD_INC ] = {"inc", 1},
		[CHIP32_CMD_DEC ] = {"dec", 1},
		[CHIP32_CMD_SUM ] = {"sum", 2},
		[CHIP32_CMD_DIF ] = {"dif", 2},
		[CHIP32_CMD_MUL ] = {"mul", 2},
		[CHIP32_CMD_DIV ] = {"div", 2},
		[CHIP32_CMD_MOD ] = {"mod", 2},
		[CHIP32_CMD_OR  ] = {"or",  2},
		[CHIP32_CMD_XOR ] = {"xor", 2},
		[CHIP32_CMD_AND ] = {"and", 2},
		[CHIP32_CMD_NOT ] = {"not", 1},
		[CHIP32_CMD_SHL ] = {"shl", 2},
		[CHIP32_CMD_SHR ] = {"shr", 2},
		[CHIP32_CMD_ROL ] = {"rol", 2},
		[CHIP32_CMD_ROR ] = {"ror", 2},
		[CHIP32_CMD_BOR ] = {"bor", 2},
		[CHIP32_CMD_BAND] = {"band",2},
		[CHIP32_CMD_BNOT] = {"bnot",2},
		[CHIP32_CMD_0x14 ... CHIP32_CMD_0x1F] = {"nop", 0},

		[CHIP32_CMD_JMP ] = {"jmp", 1},
		[CHIP32_CMD_IFT ] = {"ift", 2},
		[CHIP32_CMD_IFF ] = {"iff", 2},
		[CHIP32_CMD_CALL] = {"call",1},
		[CHIP32_CMD_RET ] = {"ret", 1},
		[CHIP32_CMD_PUSH] = {"push",1},
		[CHIP32_CMD_POP ] = {"pop", 1},
		[CHIP32_CMD_0x27 ... CHIP32_CMD_0x2F] = {"nop",0},
	
		[CHIP32_CMD_INITR] = {"initr", 1},
		[CHIP32_CMD_0x31 ... CHIP32_CMD_0x3F] = {"nop",0},
	
		[CHIP32_CMD_LOGI  ] = {"logi", 1},
		[CHIP32_CMD_LOGU  ] = {"logu", 1},
		[CHIP32_CMD_LOGC  ] = {"logc", 1},
		[CHIP32_CMD_LOGS  ] = {"logs", 1},
		[CHIP32_CMD_LOGLN ] = {"logln",0},
		[CHIP32_CMD_0x45 ... CHIP32_CMD_0x4F ] = {"nop",0},
	
		[CHIP32_CMD_STRCPY] = {"strcpy", 2},
		[CHIP32_CMD_0x51 ... CHIP32_CMD_0x5F ] = {"nop",0},
	
		[CHIP32_CMD_0x60 ... CHIP32_CMD_0x6F ] = {"nop",0},
		[CHIP32_CMD_0x70 ... CHIP32_CMD_0x7F ] = {"nop",0},
		[CHIP32_CMD_0x80 ... CHIP32_CMD_0x8F ] = {"nop",0},
		[CHIP32_CMD_0x90 ... CHIP32_CMD_0x9F ] = {"nop",0},
		[CHIP32_CMD_0xA0 ... CHIP32_CMD_0xAF ] = {"nop",0},
		[CHIP32_CMD_0xB0 ... CHIP32_CMD_0xBF ] = {"nop",0},
		[CHIP32_CMD_0xC0 ... CHIP32_CMD_0xCF ] = {"nop",0},
		[CHIP32_CMD_0xD0 ... CHIP32_CMD_0xDF ] = {"nop",0},
		[CHIP32_CMD_0xE0 ... CHIP32_CMD_0xEF ] = {"nop",0},
		[CHIP32_CMD_0xF0 ... CHIP32_CMD_0xFF ] = {"nop",0}
	};
	__private char* argname[] ={
		"",
		"$",
		"*",
		"@"
	};
	__private char* sep[] = { "", "," };

	printf("commands dump: %lu\n", size);
	for( size_t i = 0; i < size; ++i){
		printf("%5lu| %s", i, dump[cmd[i].cmd].name);
		uint8_t flags = cmd[i].flags;
		int32_t arg[2] = { cmd[i].arg0, cmd[i].arg1 };
		for(int k = 0; k < dump[cmd[i].cmd].narg; ++k){
			uint8_t ta = flags & 3;
			flags >>= 2;
			printf("%s %s%d", sep[k], argname[ta], arg[k]);
		}
		putchar('\n');
	}
}


















