#include "c32asm.h"

__private void rbhash_token_free(__unused uint32_t hash, __unused const char* name, void* a){
	free(a);
}

asmchip32_s* chip32_compiler_new(void){
	asmchip32_s* ac = NEW(asmchip32_s);
	ac->cmdcount = 0;
	ac->cmdsize = A32_START_CMD_SIZE;
	ac->code = NULL;
	ac->parse = NULL;
	ac->error = NULL;
	ac->line = 1;
	ac->mcsize = A32_START_MC_SIZE;
	ac->mccount = 0;
	ac->avaddr = A32_START_ADDR;

	ac->vcmd = MANY(lexcmd_s, ac->cmdsize);
	if( !ac->vcmd ) return NULL;
	
	ac->mc = MANY(chip32Command_s, ac->mcsize);
	if( !ac->mc ){
		free(ac->vcmd);
		return NULL;
	}	

	ac->actions = rbhash_new(A32_RBH_SIZE, A32_RBH_MIN, A32_RBH_KEY, (rbhash_f)hash_fasthash, NULL);
	if( !ac->actions ){
		free(ac->vcmd);
		free(ac->mc);
		return NULL;
	}
	ac->declare = rbhash_new(A32_RBH_SIZE, A32_RBH_MIN, A32_RBH_KEY, (rbhash_f)hash_fasthash, rbhash_token_free);
	if( !ac->declare ){
		free(ac->vcmd);
		free(ac->mc);
		rbhash_free(ac->actions);
		return NULL;
	}
	
	a32_parser_init(ac);

	return ac;
}

void chip32_compiler_free(asmchip32_s* ac){
	if( ac->error ) free(ac->error);
	free(ac->vcmd);
	free(ac->mc);
	rbhash_free(ac->actions);
	rbhash_free(ac->declare);
	free(ac);
}

void chip32_compiler_error_print(asmchip32_s* ac){
	if( !ac->error ) return;
	fprintf(stderr, 
		"error: %s at line:%lu\n",
		ac->error,
		ac->line
	);

	//printf("<PARSE>%s</PARSE>\n", ac->parse);
	const char* endline = ac->parse;
	while( *endline && *endline != '\n' ) ++endline;
	//printf("<ENDLINE>%.*s<ENDLINE>\n", (int)(endline-ac->parse), ac->parse);


	const char* startline = ac->parse;
	while( startline > ac->code && *startline != '\n' ){
		--startline;
	}
	if( *startline == '\n' ) ++ startline;

	if( startline - endline > 100 ){
		startline = ac->parse - 50;
		endline = ac->parse + 50;
	}

	fprintf(stderr,
		"%.*s\n", 
		(int)(endline - startline), startline
	);

	while( startline < ac->parse ){
		++startline;
		putchar(' ');
	}

	puts("^");
}

int chip32_compiler_build(asmchip32_s* ac, const char* code){
	ac->code = ac->parse = code;
	if( a32_lexer(ac) ) return -1;
	if( a32_parse(ac) ) return -1;
	return 0;
}

void chip32_lex_dump(asmchip32_s* ac){
	printf("lex dumps:%lu\n", ac->cmdcount);

	for( size_t i = 0; i < ac->cmdcount; ++i ){
		printf("%5lu|", i);
		for( size_t k = 0; k < ac->vcmd[i].ntoken; ++k ){
			printf("<%d|%.*s>", 
				ac->vcmd[i].tokens[k].type,
				(int)(ac->vcmd[i].tokens[k].end - ac->vcmd[i].tokens[k].start),
				ac->vcmd[i].tokens[k].start
			);
		}
		putchar('\n');
	}
}

chip32Command_s* chip32_commands(asmchip32_s* c, size_t* count){
	*count = c->mccount;
	return c->mc;
}

int chip32_commands_save(asmchip32_s* c, FILE* f){
	for( size_t i = 0; i < c->mccount; ++i ){
		if( fwrite(&c->mc[i].flags, sizeof(uint8_t), 1, f) != 1 ) return -1;
		if( fwrite(&c->mc[i].cmd,   sizeof(uint8_t), 1, f) != 1 ) return -1;
		if( fwrite(&c->mc[i].arg0,  sizeof(int32_t), 1, f) != 1 ) return -1;
		if( fwrite(&c->mc[i].arg1,  sizeof(int32_t), 1, f) != 1 ) return -1;
	}
	return 0;
}

int chip32_command_load(chip32Command_s* cmd, FILE* f){
	if( fread(&cmd->flags, sizeof(uint8_t), 1, f) != 1 ) return -1;
	if( fread(&cmd->cmd,  sizeof(uint8_t),  1, f) != 1 ) return -1;
	if( fread(&cmd->arg0, sizeof(uint32_t), 1, f) != 1 ) return -1;
	if( fread(&cmd->arg1, sizeof(uint32_t), 1, f) != 1 ) return -1;
	return 0;
}



