#include "c32asm.h"

__private lexcmd_s* a32_lexcmd_new(asmchip32_s* c){
	if( c->cmdcount >= c->cmdsize ){
		c->cmdsize *= 2;
		lexcmd_s* tmp = realloc(c->vcmd, sizeof(lexcmd_s) * c->cmdsize);
		if( !tmp ){
			return NULL;
		}
		c->vcmd = tmp;
	}
	return &c->vcmd[c->cmdcount++];
}

__private void a32_lexcmd_unroll_cmdmem(asmchip32_s* c){
	c->cmdcount--;
}

__private void a32_skip_space(asmchip32_s* c){
	while( *c->parse && (*c->parse == ' ' || *c->parse == '\t') ) ++c->parse;
}

__private void a32_skip_comment(asmchip32_s* c){
	while( *c->parse && *c->parse != '\n' ) ++c->parse;
}

__private int a32_endl(asmchip32_s* c){
	switch( *c->parse ){
		case 0   : return 1;
		case '\n': ++c->parse; return 1;
		case ';' : a32_skip_comment(c); if( *c->parse ) ++c->parse; return 1;
	}
	return 0;
}

__private int a32_token_quote(asmchip32_s* c, lextoken_s* tk){
	char quote = *c->parse;
	tk->start = c->parse++;
	while( *c->parse && *c->parse != quote ){
		if( *c->parse == '\\' && *(c->parse+1)  ) ++c->parse;
		++c->parse;
	}
	if( tk->start == c->parse || *c->parse != quote ){
		c->parse = tk->start;
		c->error = str_printf("missing terminator of %c", quote);
		return -1;
	}
	++c->parse;
	tk->end = c->parse;
	return 0;
}

__private int a32_token_name_next(asmchip32_s* c, lextoken_s* tk){
	if( (tolower(*c->parse) < 'a' || tolower(*c->parse) > 'z') && *c->parse != '_' ){	
		c->error = str_printf("token can't start with a number");
		return -1;
	}
	tk->start = c->parse;
	while( 
		*c->parse && 
		(
			(*c->parse >= 'A' && *c->parse <= 'Z') || 
			(*c->parse >= 'a' && *c->parse <= 'z') ||
		    (*c->parse >= '0' && *c->parse <= '9') ||
			*c->parse == '_'	
		)
	){
		++c->parse;
	}
	if( tk->start == c->parse ){
		c->error = str_printf("invalid token");
		return -1;
	}
	tk->end = c->parse;
	return 0;
}

__private int a32_token_integer(asmchip32_s* c, lextoken_s* tk){
	if( (tolower(*c->parse) < '0' || tolower(*c->parse) > '9') && *c->parse != '-' ){	
		c->error = str_printf("wrong number");
		return -1;
	}
	tk->start = c->parse;
	while( 
		*c->parse && 
		(
		    (*c->parse >= '0' && *c->parse <= '9') ||
			*c->parse == '-'	
		)
	){
		++c->parse;
	}
	if( tk->start == c->parse ){
		c->error = str_printf("invalid token");
		return -1;
	}
	tk->end = c->parse;
	return 0;
}

__private int a32_arg_next(asmchip32_s* c, lexcmd_s* cmd){
	switch(*c->parse){
		case '?':
			cmd->tokens[cmd->ntoken].line  = c->line;
			cmd->tokens[cmd->ntoken].type  = TK_AUTO;
			cmd->tokens[cmd->ntoken].start = c->parse++;
			cmd->tokens[cmd->ntoken++].end   = c->parse;
		return 0;

		case '$':
			cmd->tokens[cmd->ntoken].line  = c->line;
			cmd->tokens[cmd->ntoken].type  = TK_RAM;
			cmd->tokens[cmd->ntoken].start = c->parse++;
			cmd->tokens[cmd->ntoken++].end   = c->parse;
		return a32_arg_next(c, cmd);

		case '&':
			cmd->tokens[cmd->ntoken].line  = c->line;
			cmd->tokens[cmd->ntoken].type  = TK_ADDR;
			cmd->tokens[cmd->ntoken].start = c->parse++;
			cmd->tokens[cmd->ntoken++].end   = c->parse;
		return a32_arg_next(c, cmd);

		case '*':
			cmd->tokens[cmd->ntoken].line  = c->line;
			cmd->tokens[cmd->ntoken].type  = TK_PTR;
			cmd->tokens[cmd->ntoken].start = c->parse++;
			cmd->tokens[cmd->ntoken++].end   = c->parse;
		return a32_arg_next(c, cmd);

		case '@':
			cmd->tokens[cmd->ntoken].line  = c->line;
			cmd->tokens[cmd->ntoken].type  = TK_DATA;
			cmd->tokens[cmd->ntoken].start = c->parse++;
			cmd->tokens[cmd->ntoken++].end   = c->parse;
		return a32_arg_next(c, cmd);

		case '-':
		case '0' ... '9':
			if( a32_token_integer(c, &cmd->tokens[cmd->ntoken]) ) return -1;
			cmd->tokens[cmd->ntoken].line  = c->line;
			cmd->tokens[cmd->ntoken++].type  = TK_LITERAL_NUM;
		return 0;

		case '\'':
			if( a32_token_quote(c, &cmd->tokens[cmd->ntoken]) ) return -1;
			cmd->tokens[cmd->ntoken].line  = c->line;
			cmd->tokens[cmd->ntoken++].type  = TK_LITERAL_CHAR;
		return 0;

		case '"':
			if( a32_token_quote(c, &cmd->tokens[cmd->ntoken]) ) return -1;
			cmd->tokens[cmd->ntoken].line  = c->line;
			cmd->tokens[cmd->ntoken++].type  = TK_LITERAL_STR;
		return 0;

		case 'a' ... 'z':
		case 'A' ... 'Z':
		case '_' :
			if( a32_token_name_next(c, &cmd->tokens[cmd->ntoken]) ) return -1;
			cmd->tokens[cmd->ntoken].line  = c->line;
			cmd->tokens[cmd->ntoken++].type  = TK_NAME;
		return 0;
	}
	
	c->error = str_printf("unknow token");
	return -1;
}

__private int a32_lexer_line(asmchip32_s* c){
	a32_skip_space(c);
	if( a32_endl(c) ) return 0;

	lexcmd_s* cmd = a32_lexcmd_new(c);
	if( !cmd ){
		c->error = str_printf("eom");
		return -1;
	}
	cmd->ntoken = 0;

	while(1){
		cmd->tokens[cmd->ntoken].type = TK_NAME;
		cmd->tokens[cmd->ntoken].line = c->line;
		if( a32_token_name_next(c, &cmd->tokens[cmd->ntoken]) ){
			a32_lexcmd_unroll_cmdmem(c);
			return -1;
		}
		if( *c->parse == ':' ){
			cmd->tokens[cmd->ntoken++].type = TK_LABEL;
			++c->parse;
			a32_skip_space(c);
			continue;
		}
		if( *c->parse != ' ' && *c->parse != '\t' && *c->parse != '\n' && *c->parse != ';' ){
			c->error = str_printf("wrong separator");
			a32_lexcmd_unroll_cmdmem(c);
			return -1;
		}
		++cmd->ntoken;
		break;
	}

	while( 1 ){
		a32_skip_space(c);
		if( a32_endl(c) ) return 0;
		if( a32_arg_next(c, cmd) ){
			a32_lexcmd_unroll_cmdmem(c);
			return -1;
		}
		if( a32_endl(c) ) return 0;
		if( *c->parse != ',' ){
			c->error = str_printf("wrong seaprator");
			a32_lexcmd_unroll_cmdmem(c);
			return -1;
		}
		++c->parse;
	}

	return 0;
}

int a32_lexer(asmchip32_s* c){
	int ret = 0;
	while( *c->parse && !(ret=a32_lexer_line(c)) ){
		++c->line;
	}
	return ret;
}

