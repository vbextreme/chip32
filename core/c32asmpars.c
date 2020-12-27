#include "c32asm.h"

__private chip32Command_s* a32_parse_mc_new(asmchip32_s* ac){
	if( ac->mccount >= ac->mcsize ){
		ac->mcsize *= 2;
		chip32Command_s* tmp = realloc(ac->mc, sizeof(chip32Command_s) * ac->mcsize);
		if( !tmp ){
			return NULL;
		}
		ac->mc = tmp;
	}
	return &ac->mc[ac->mccount++];

}

__private int push_move(asmchip32_s* c, uint8_t flags, uint32_t a0, uint32_t a1){
	chip32Command_s* m = a32_parse_mc_new(c);
	m->flags = flags;
	m->arg0  = a0;
	m->arg1  = a1;
	m->cmd   = CHIP32_CMD_MOVE;
	return 0;
}

__private int action_declare(asmchip32_s* c, lexcmd_s* cmd){
	size_t ic = 1;
	declared_s* dec = NEW(declared_s);

	if( cmd->tokens[ic].type != TK_NAME ){
		c->line = cmd->tokens[1].line;
		c->parse = cmd->tokens[1].start;
		c->error = str_printf("invalid declare: aspected name");
		return -1;
	}

	dec->type = DEC_TYPE_LITERAL_NUM;
	dec->name = cmd->tokens[ic].start;
	dec->size = cmd->tokens[ic].end - cmd->tokens[ic].start;
	if( ++ic >= cmd->ntoken ) goto ICERR;

	int dauto = 0;
	switch( cmd->tokens[ic].type ){
		case TK_LITERAL_NUM: dec->addr = strtoul(dec->name, NULL, 10); break;
		case TK_AUTO: dec->addr = c->avaddr; dauto = 1; break;
			
		default:
			c->line = cmd->tokens[ic].line;
			c->parse = cmd->tokens[ic].start;
			c->error = str_printf("invalid declare: aspected address or ?");
			free(dec);
		return -1;
	}
	if( ++ic >= cmd->ntoken ){
		if( dauto ) ++c->avaddr;
		goto ICEND;
	}

	ssize_t sauto = 0;
	switch( cmd->tokens[ic].type ){
		case TK_LITERAL_NUM: sauto = strtoul(cmd->tokens[ic+1].start, NULL, 10); break;
		case TK_AUTO: sauto = 0; break;
		
		default:
			c->line = cmd->tokens[ic].line;
			c->parse = cmd->tokens[ic].start;
			c->error = str_printf("invalid declare: aspected size or ?");
			free(dec);
		return -1;	
	}
	
	if( ++ic >= cmd->ntoken ){
		if( !sauto ){
			c->line = cmd->tokens[ic-1].line;
			c->parse = cmd->tokens[ic-1].start;
			c->error = str_printf("invalid declare: aspected value after auto size");
			free(dec);
			return -1;
		}
		if( dauto ) c->avaddr += sauto;
		goto ICEND;
	}

	switch( cmd->tokens[ic].type ){
		case TK_LITERAL_STR:{
			const char* s = cmd->tokens[ic].start+1;
			const char* e = strchr(s, s[-1]);
			if( !e || !*e ){
				c->line = cmd->tokens[ic].line;
				c->parse = cmd->tokens[ic].start;
				c->error = str_printf("invalid string");
				free(dec);
				return -1;
			}
			if( !sauto ) sauto = e-s;
			for( int j = 0; j < sauto; ++j ){
				push_move(c, CHIP32_TYPE_RAM, dec->addr+j, s[j]);
			}
			push_move(c, CHIP32_TYPE_RAM, dec->addr+sauto, 0);
			if( dauto ) c->avaddr += sauto+1;
			break;
		}

		case TK_LITERAL_NUM:
		case TK_LITERAL_CHAR:
			if( !sauto ) sauto = cmd->ntoken - ic;
			for( int j = 0; j < sauto; ++j ){
				switch( cmd->tokens[ic].type ){
					case TK_LITERAL_NUM:  push_move(c, CHIP32_TYPE_RAM, dec->addr+j, strtoul(cmd->tokens[ic].start, NULL, 10)); break;
					case TK_LITERAL_CHAR: push_move(c, CHIP32_TYPE_RAM, dec->addr+j, cmd->tokens[ic].start[1]); break;

					default:
						c->line = cmd->tokens[ic].line;
						c->parse = cmd->tokens[ic].start;
						c->error = str_printf("cant mix different literal type");
						free(dec);
					return -1;
				}
				if( ++ic >= cmd->ntoken ) goto ICERR;
			}
			if( dauto ) c->avaddr += sauto;
		break;
	
		default:
			c->line = cmd->tokens[ic].line;
			c->parse = cmd->tokens[ic].start;
			c->error = str_printf("invalid init value, aspected str, num or char");
			free(dec);
		return -1;
	}

ICEND:
	if( rbhash_add_unique(c->declare, dec->name, dec->size, dec) ){
		c->line = cmd->tokens[1].line;
		c->parse = cmd->tokens[1].start;
		c->error = str_printf("invalid declare: redeclaration var name '%.*s'", (int)dec->size, dec->name);
		free(dec);
		return -1;
	}

	return 0;
ICERR:
	free(dec);
	c->line = cmd->tokens[ic-1].line;
	c->parse = cmd->tokens[ic-1].start;
	c->error = str_printf("missing argument");
	return -1;
}


__private int parse_label(asmchip32_s* c, lexcmd_s* cmd){
	if( cmd->tokens[0].type != TK_LABEL ) return 0;

	declared_s* dec = NEW(declared_s);
	dec->type = DEC_TYPE_LABEL;
	dec->name = cmd->tokens[0].start;
	dec->size = cmd->tokens[0].end - cmd->tokens[0].start;
	dec->addr = c->cmdcount;

	if( rbhash_add_unique(c->declare, dec->name, dec->size, dec) ){
		c->line = cmd->tokens[0].line;
		c->parse = cmd->tokens[0].start;
		c->error = str_printf("invalid label: redeclaration name '%.*s'", (int)dec->size, dec->name);
		free(dec);
		return -1;
	}

	return 1;
}

__private int action_initr(asmchip32_s* c, lexcmd_s* cmd){
	ssize_t ic = parse_label(c, cmd);
	
	if( ++ic >= (ssize_t)cmd->ntoken ) goto ICERR;

	if( cmd->tokens[ic].type != TK_LITERAL_NUM ){
		c->line = cmd->tokens[ic].line;
		c->parse = cmd->tokens[ic].start;
		c->error = str_printf("invalid initr: aspected size of memory");
		return -1;
	}

	chip32Command_s* m = a32_parse_mc_new(c);
	m->flags = 0;
	m->arg0  = strtoul(cmd->tokens[ic].start, NULL, 10);
	m->arg1  = 0;
	m->cmd   = CHIP32_CMD_INITR;

	return 0;
ICERR:
	c->line = cmd->tokens[ic-1].line;
	c->parse = cmd->tokens[ic-1].start;
	c->error = str_printf("missing argument");
	return -1;
}

__private int parse_lvalue(asmchip32_s* c, lexcmd_s* cmd, ssize_t* ic, uint8_t* flags, uint32_t* arg0){
	if( *ic >= (ssize_t)cmd->ntoken ) goto ICERR;

	switch( cmd->tokens[*ic].type ){
		case TK_RAM:
			A32_ARG0F_SET(*flags, CHIP32_TYPE_RAM);
		break;
		
		case TK_PTR:
			A32_ARG0F_SET(*flags, CHIP32_TYPE_PTR);
		break;

		default:
			c->line = cmd->tokens[*ic].line;
			c->parse = cmd->tokens[*ic].start;
			c->error = str_printf("left value required $ or *");
		return -1;
	}
	if( ++(*ic) >= (ssize_t)cmd->ntoken ) goto ICERR;

	switch( cmd->tokens[*ic].type ){
		case TK_LITERAL_NUM:
			*arg0 = strtoul(cmd->tokens[*ic].start, NULL, 10);
		break;

		case TK_NAME:{
			declared_s* dec = rbhash_find(c->declare, cmd->tokens[*ic].start, cmd->tokens[*ic].end - cmd->tokens[*ic].start);
			if( !dec || dec->type == DEC_TYPE_LABEL ){
				c->line = cmd->tokens[*ic].line;
				c->parse = cmd->tokens[*ic].start;
				const char* terr = dec ? "is label" : "not exsist";
				c->error = str_printf("variable %.*s %s", (int)(cmd->tokens[*ic].end - cmd->tokens[*ic].start), cmd->tokens[*ic].start, terr);
			}
			*arg0 = dec->addr;
			break;
		}

		default:
			c->line = cmd->tokens[*ic].line;
			c->parse = cmd->tokens[*ic].start;
			c->error = str_printf("left value required literal num or var name");
		return -1;
	}
	
	return 0;
ICERR:
	c->line = cmd->tokens[*ic-1].line;
	c->parse = cmd->tokens[*ic-1].start;
	c->error = str_printf("missing argument name");
	return -1;
}

__private int parse_rvalue(asmchip32_s* c, lexcmd_s* cmd, ssize_t* ic, uint8_t* flags, uint32_t* arg, int idarg){
	if( *ic >= (ssize_t)cmd->ntoken ) goto ICERR;
	
	switch( cmd->tokens[*ic].type ){
		case TK_RAM:
			if( idarg ){
				A32_ARG1F_SET(*flags, CHIP32_TYPE_RAM);
			}
			else{
				A32_ARG0F_SET(*flags, CHIP32_TYPE_RAM);
			}
			if( ++(*ic) >= (ssize_t)cmd->ntoken ) goto ICERR;
		break;
		
		case TK_PTR:
			if( idarg ){
				A32_ARG1F_SET(*flags, CHIP32_TYPE_PTR);
			}
			else{
				A32_ARG0F_SET(*flags, CHIP32_TYPE_PTR);
			}
			if( ++(*ic) >= (ssize_t)cmd->ntoken ) goto ICERR;
		break;

		case TK_ADDR:
			if( idarg ){
				A32_ARG1F_SET(*flags, CHIP32_TYPE_LITERAL);
			}
			else{
				A32_ARG0F_SET(*flags, CHIP32_TYPE_LITERAL);
			}
			if( ++(*ic) >= (ssize_t)cmd->ntoken ) goto ICERR;
		break;
	
		case TK_DATA:
			if( idarg ){
				A32_ARG1F_SET(*flags, CHIP32_TYPE_DATA);
			}
			else{
				A32_ARG0F_SET(*flags, CHIP32_TYPE_DATA);
			}
			if( ++(*ic) >= (ssize_t)cmd->ntoken ) goto ICERR;
		break;

		case TK_LITERAL_NUM:
		case TK_NAME:
		break;

		default:
			c->line = cmd->tokens[*ic].line;
			c->parse = cmd->tokens[*ic].start;
			c->error = str_printf("right value required numerical/variable type");
		return -1;
	}

	switch( cmd->tokens[*ic].type ){
		case TK_LITERAL_NUM:
			*arg = strtoul(cmd->tokens[*ic].start, NULL, 10);
		break;

		case TK_NAME:{
			declared_s* dec = rbhash_find(c->declare, cmd->tokens[*ic].start, cmd->tokens[*ic].end - cmd->tokens[*ic].start);
			if( !dec ){
				c->line = cmd->tokens[*ic].line;
				c->parse = cmd->tokens[*ic].start;
				const char* terr = "not exsist";
				c->error = str_printf("variable %.*s %s", (int)(cmd->tokens[*ic].end - cmd->tokens[*ic].start), cmd->tokens[*ic].start, terr);
				return -1;
			}
			*arg = dec->addr;
			break;
		}

		default:
			c->line = cmd->tokens[*ic].line;
			c->parse = cmd->tokens[*ic].start;
			c->error = str_printf("right value required literal num or var name");
		return -1;
	}
	
	return 0;
ICERR:
	c->line = cmd->tokens[*ic-1].line;
	c->parse = cmd->tokens[*ic-1].start;
	c->error = str_printf("missing argument name");
	return -1;
}

__private int parse_noarg(asmchip32_s* c, lexcmd_s* cmd, uint32_t idcmd){
	parse_label(c, cmd);
	chip32Command_s* m = a32_parse_mc_new(c);
	m->flags = 0;
	m->arg0  = 0;
	m->arg1  = 0;
	m->cmd   = idcmd;
	return 0;
}

__private int parse_onearg(asmchip32_s* c, lexcmd_s* cmd, uint32_t idcmd){
	ssize_t ic = parse_label(c, cmd);
	uint8_t flags = 0;
	uint32_t arg0;

	++ic;
	if( parse_rvalue(c, cmd, &ic, &flags, &arg0, 0) ) return -1;

	chip32Command_s* m = a32_parse_mc_new(c);
	m->flags = flags;
	m->arg0  = arg0;
	m->arg1  = 0;
	m->cmd   = idcmd;

	return 0;
}

__private int parse_twoarg(asmchip32_s* c, lexcmd_s* cmd, uint32_t idcmd){
	ssize_t ic = parse_label(c, cmd);
	uint8_t flags = 0;
	uint32_t arg0;
	uint32_t arg1;

	++ic;
	if( parse_lvalue(c, cmd, &ic, &flags, &arg0) ) return -1;
	if( ++ic >= (ssize_t)cmd->ntoken ) goto ICERR;
	if( parse_rvalue(c, cmd, &ic, &flags, &arg1, 1) ) return -1;

	chip32Command_s* m = a32_parse_mc_new(c);
	m->flags = flags;
	m->arg0  = arg0;
	m->arg1  = arg1;
	m->cmd   = idcmd;
	return 0;
ICERR:
	c->line = cmd->tokens[ic-1].line;
	c->parse = cmd->tokens[ic-1].start;
	c->error = str_printf("missing argument name");
	return -1;
}

__private int action_nop(asmchip32_s* c, lexcmd_s* cmd){
	return parse_noarg(c, cmd, CHIP32_CMD_NOP);
}

__private int action_move(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_MOVE);
}

__private int action_inc(asmchip32_s* c, lexcmd_s* cmd){
	return parse_onearg(c, cmd, CHIP32_CMD_INC);
}

__private int action_dec(asmchip32_s* c, lexcmd_s* cmd){
	return parse_onearg(c, cmd, CHIP32_CMD_DEC);
}

__private int action_sum(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_SUM);
}

__private int action_dif(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_DIF);
}

__private int action_mul(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_MUL);
}

__private int action_div(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_DIV);
}

__private int action_mod(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_MOD);
}

__private int action_or(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_OR);
}

__private int action_xor(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_XOR);
}

__private int action_and(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_AND);
}

__private int action_not(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_NOT);
}

__private int action_shl(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_SHL);
}

__private int action_shr(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_SHR);
}

__private int action_rol(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_ROL);
}

__private int action_ror(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_ROR);
}

__private int action_bor(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_BOR);
}

__private int action_band(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_BAND);
}

__private int action_bnot(asmchip32_s* c, lexcmd_s* cmd){
	return parse_onearg(c, cmd, CHIP32_CMD_BNOT);
}

__private int action_jmp(asmchip32_s* c, lexcmd_s* cmd){
	return parse_onearg(c, cmd, CHIP32_CMD_JMP);
}

__private int action_jmpi(asmchip32_s* c, lexcmd_s* cmd){
	ssize_t ic = parse_label(c, cmd);
	uint8_t flags = 0;
	uint32_t arg0;

	++ic;
	if( parse_rvalue(c, cmd, &ic, &flags, &arg0, 0) ) return -1;

	chip32Command_s* m = a32_parse_mc_new(c);
	m->flags = flags;
	m->arg0  = c->mccount + arg0;
	m->arg1  = 0;
	m->cmd   = CHIP32_CMD_JMP;
	return 0;
}

__private int action_ift(asmchip32_s* c, lexcmd_s* cmd){
	ssize_t ic = parse_label(c, cmd);
	uint8_t flags = 0;
	uint32_t arg0;
	uint32_t arg1;

	++ic;
	if( parse_lvalue(c, cmd, &ic, &flags, &arg0) ) return -1;
	if( ++ic >= (ssize_t)cmd->ntoken ) goto ICERR;
	if( parse_rvalue(c, cmd, &ic, &flags, &arg1, 1) ) return -1;

	chip32Command_s* m = a32_parse_mc_new(c);
	m->flags = flags;
	m->arg0  = arg0;
	m->arg1  = c->mccount + arg1;
	m->cmd   = CHIP32_CMD_IFT;
	return 0;
ICERR:
	c->line = cmd->tokens[ic-1].line;
	c->parse = cmd->tokens[ic-1].start;
	c->error = str_printf("missing argument name");
	return -1;
}

__private int action_iff(asmchip32_s* c, lexcmd_s* cmd){
	ssize_t ic = parse_label(c, cmd);
	uint8_t flags = 0;
	uint32_t arg0;
	uint32_t arg1;

	++ic;
	if( parse_lvalue(c, cmd, &ic, &flags, &arg0) ) return -1;
	if( ++ic >= (ssize_t)cmd->ntoken ) goto ICERR;
	if( parse_rvalue(c, cmd, &ic, &flags, &arg1, 1) ) return -1;

	chip32Command_s* m = a32_parse_mc_new(c);
	m->flags = flags;
	m->arg0  = arg0;
	m->arg1  = c->mccount + arg1;
	m->cmd   = CHIP32_CMD_IFF;
	return 0;
ICERR:
	c->line = cmd->tokens[ic-1].line;
	c->parse = cmd->tokens[ic-1].start;
	c->error = str_printf("missing argument name");
	return -1;
}

__private int action_call(asmchip32_s* c, lexcmd_s* cmd){
	return parse_onearg(c, cmd, CHIP32_CMD_CALL);
}

__private int action_ret(asmchip32_s* c, lexcmd_s* cmd){
	return parse_noarg(c, cmd, CHIP32_CMD_RET);
}

__private int action_push(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_PUSH);
}

__private int action_pop(asmchip32_s* c, lexcmd_s* cmd){
	return parse_onearg(c, cmd, CHIP32_CMD_POP);
}

__private int action_logi(asmchip32_s* c, lexcmd_s* cmd){
	return parse_onearg(c, cmd, CHIP32_CMD_LOGI);
}

__private int action_logu(asmchip32_s* c, lexcmd_s* cmd){
	return parse_onearg(c, cmd, CHIP32_CMD_LOGU);
}

__private int action_logc(asmchip32_s* c, lexcmd_s* cmd){
	return parse_onearg(c, cmd, CHIP32_CMD_LOGC);
}

__private int action_logs(asmchip32_s* c, lexcmd_s* cmd){
	return parse_onearg(c, cmd, CHIP32_CMD_LOGS);
}

__private int action_logln(asmchip32_s* c, lexcmd_s* cmd){
	return parse_noarg(c, cmd, CHIP32_CMD_LOGLN);
}

__private int action_strcpy(asmchip32_s* c, lexcmd_s* cmd){
	return parse_twoarg(c, cmd, CHIP32_CMD_STRCPY);
}

int a32_parser_init(asmchip32_s* c){
#define declare_action(NAME) rbhash_add(c->actions, #NAME, strlen(#NAME), action_##NAME)
	declare_action(declare);
	declare_action(initr);
	declare_action(nop);
	declare_action(move);
	declare_action(inc);
	declare_action(dec);
	declare_action(sum);
	declare_action(dif);
	declare_action(mul);
	declare_action(div);
	declare_action(mod);
	declare_action(or);
	declare_action(xor);
	declare_action(and);
	declare_action(not);
	declare_action(shl);
	declare_action(shr);
	declare_action(rol);
	declare_action(ror);
	declare_action(bor);
	declare_action(band);
	declare_action(bnot);
	declare_action(jmp);
	declare_action(jmpi);
	declare_action(ift);
	declare_action(iff);
	declare_action(call);
	declare_action(ret);
	declare_action(push);
	declare_action(pop);
	declare_action(logi);
	declare_action(logu);
	declare_action(logc);
	declare_action(logs);
	declare_action(logln);
	declare_action(strcpy);

	return 0;
}

int a32_parse(asmchip32_s* c){
	for( size_t i = 0; i < c->cmdcount; ++i){
		size_t iname = c->vcmd[i].tokens[0].type == TK_LABEL && c->vcmd[i].ntoken > 0 ? 1 : 0;
		action_f fn = rbhash_find(c->actions, c->vcmd[i].tokens[iname].start,c->vcmd[i].tokens[iname].end - c->vcmd[i].tokens[iname].start);
		if( !fn ){
			c->line = c->vcmd[i].tokens[iname].line;
			c->parse = c->vcmd[i].tokens[iname].start;
			c->error = str_printf("unknow command '%.*s'", 
					(int)(c->vcmd[i].tokens[iname].end - c->vcmd[i].tokens[iname].start), c->vcmd[i].tokens[iname].start
			);
			return -1;
		}
		if( fn(c, &c->vcmd[i]) ) return -1;	
	}
	return 0;
}



