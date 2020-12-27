#ifndef _GNU_SOURCE
        #define _GNU_SOURCE
#endif
#ifndef _XOPEN_SPURCE
        #define _XOPEN_SPURCE 600
#endif
#include <stdlib.h>
#include <time.h>
#include <utime.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <sos_type.h>
#include <sos_string.h>
#include <chip32.h>

#define MAX_DIR_INCLUDE 32
#define CHUNK 4096

#define die(F,args...) do{ fprintf(stderr, F "\n", ##args); exit(1); }while(0)

typedef struct codeloader{
	chip32Command_s* cmd;
	size_t size;
}codeloader_s;

void path_current(char* path){
	if( !getcwd(path, PATH_MAX) ) die("error getcwd:%s", strerror(errno));
}

char* path_current_new(void){
	char* ret = getcwd(NULL, 0);
	if( !ret ) die("error getcwd:%s", strerror(errno));
	return ret;
}

void path_home(char* path){
	char *hd;
	if( (hd = secure_getenv("HOME")) == NULL ){
		struct passwd* spwd = getpwuid(getuid());
		die("error getpwuid:%s", strerror(errno));
		strcpy(path, spwd->pw_dir);
	}
	else{
		strcpy(path, hd);
	}
}

void path_kill_back(char* path){
	size_t len = strlen(path);
	if( len == 1 ) return;
	if( path[len-1] == '/' ) path[len-1] = 0;
	char* bs = strrchr(path, '/');
	if( bs ) *bs = 0;
}

char* path_resolve(const char* path){
	char cur[PATH_MAX];
	char out[PATH_MAX];

	size_t lpath = strlen(path);
	if( lpath > PATH_MAX - 1 ) die("path %s to long", path);
	if( !str_equal(path, lpath, "~", 1) || !strncmp(path, "~/", 2) ){
		path_home(cur);
		if( lpath + strlen(cur) > PATH_MAX -1 ) die("path %s + %s to long", cur, path);
		if( path[1] && path[2] ){
			strcpy(&cur[strlen(cur)], &path[1]);
		}
	}
	else if( !str_equal(path, lpath, ".", 1) || !strncmp(path, "./", 2) ){
		path_current(cur);
		if( lpath + strlen(cur) > PATH_MAX - 1) die("path %s + %s to long", cur, path);
		if( path[1] && path[2] ){
			strcpy(&cur[strlen(cur)], &path[1]);
		}
	}
	else if( !str_equal(path, lpath, "..", 1) || !strncmp(path, "../", 2) ){
		path_current(cur);
		path_kill_back(cur);
		if( lpath + strlen(cur) > PATH_MAX - 1) die("path %s + %s to long", cur, path);
		if( path[2] && path[3] ){
			strcpy(&cur[strlen(cur)], &path[2]);
		}
	}
	else{
		strcpy(cur, path);
	}

	char* parse = cur;
	char* pout = out;
	while( *parse ){
		if( *parse == '.' ){
			if( *(parse+1) == '/' ){
				parse += 2;
				continue;
			}
			if( *(parse+1) == '.' ){
				if( *(parse+2) == 0 ){
					*pout = 0;
					path_kill_back(out);
					pout = out + strlen(out);
					parse += 2;
					continue;
				}
				if ( *(parse+2) == '/' ){
					*pout = 0;
					path_kill_back(out);
					pout = out + strlen(out);
					parse += 2;
					continue;
				}
			}
		}
		*pout++ = *parse++;
	}
	*pout = 0;
	return str_dup(out, pout-out);
}

int file_exists(const char* path){
	struct stat b;
	return stat(path, &b) ? 0 : 1;
}

char* find_file(char* dinc[MAX_DIR_INCLUDE], size_t dincount, const char* fname){
	if( file_exists(fname) ) return str_dup(fname, 0);
	for( size_t i = 0; i < dincount; ++i){
		char* ret = str_printf("%s/%s", dinc[i], fname);
		if( file_exists(ret) ) return ret;
		free(ret);
	}
	die("file %s not exists", fname);
	return NULL;
}

char* load_code(char* dinc[MAX_DIR_INCLUDE], size_t dincount, char* src){
	char* fname = find_file(dinc, dincount, src);

	FILE* f = fopen(fname, "r");
	if( !f ) die("error:%s on open file %s", strerror(errno), fname);

	size_t codeSize = CHUNK;
	size_t codeLen  = 0;
	char* code = MANY(char, codeSize);
	if( !code ) die("error:%s on allocate mem\n", strerror(errno));

	size_t nr;
	while( (nr=fread(&code[codeLen], sizeof(char), (codeSize - codeLen)-1, f)) > 0 ){
		codeLen  += nr;
		if( codeLen >= codeSize -1 ){
			codeSize += CHUNK;
			code = realloc(code, sizeof(char) * codeSize);
			if( !code ) die("error:%s on reallocate mem\n", strerror(errno));
		}
	}
	code[codeLen] = 0;
	return code;
}

void preprocessing_code(char* dinc[MAX_DIR_INCLUDE], size_t dincount, char** code){
	char* prest = *code;
   	while( (prest=strchr(prest, '#')) ){
		if( strcmp(prest, "#include") ) die("unsupported preprocess %.*s", 30, prest);
		char* incfile = prest + 8;
		while( *incfile && *incfile != '"' ) ++incfile;
		if( *incfile != '"' ) die("invalid include %.*s", 30, prest);
		++incfile;
		char* endf = incfile;
		while( *endf && *endf != '"' ) ++endf;
		if( *endf != '"' ) die("invalid include %.*s", 30, prest);
		*endf = 0;
		char* inscode = load_code(dinc, dincount, incfile);
		*prest = 0;
		prest = str_printf("%s%s%s", *code, inscode, endf+1);
		free(*code);
		*code = prest;
	}
}

__private ssize_t page_load(void* ctx, uint32_t addr, chip32Command_s* data, uint32_t size){
	FILE* f = ctx;
	if( fseek(f, addr * sizeof(chip32Command_s), SEEK_SET) ) die("error:%s on seekinf page", strerror(errno));

	size_t n;
	for(n = 0; n < size && !chip32_command_load(&data[n], f); ++n);
	for(size_t i = n; i < size; ++i){
		data[i].cmd = 0;
	}
	return n;
}

int main(int count, char** argv){
	char* dinclude[MAX_DIR_INCLUDE];
	size_t countdi = 0;
	char* src = NULL;
	char* code = NULL;
	char* out = NULL;
	int verbose = 0;
	int run = 0;

	int iarg = 1;
	while( iarg < count ){
		if( !strcmp(argv[iarg], "-i") ){
			if( ++iarg >= count ) goto WRONGA;
			if( countdi >= MAX_DIR_INCLUDE ){
				fputs("too much dir include\n", stderr);
				exit(1);
			}
			dinclude[countdi++] = path_resolve(argv[iarg]);
		}
		else if( !strcmp(argv[iarg], "-f") ){
			if( ++iarg >= count ) goto WRONGA;
			if( src ){
				fputs("already set file\n", stderr);
				exit(1);
			}
			src = path_resolve(argv[iarg]);
		}
		else if( !strcmp(argv[iarg], "-o") ){
			if( ++iarg >= count ) goto WRONGA;
			if( out ){
				fputs("already set output\n", stderr);
				exit(1);
			}
			out = path_resolve(argv[iarg]);
		}
		else if( !strcmp(argv[iarg], "-r") ){
			run = 1;
		}
		else if( !strcmp(argv[iarg], "-v") ){
			verbose = 1;
		}
		else {
			fputs(
				"usage:\n"
				"\t-i <dir>   /* add include directory                   */\n"
				"\t-f <file>  /* source code file                        */\n"
				"\t-o <file>  /* output filename  or builded file to run */\n"
				"\t-r         /* run -o file                             */\n"
				"\t-v         /* verbose mode                            */\n"
				"\t-h         /* display this                            */\n",
				stderr
			);
			exit(1);
		}
		++iarg;
	}
	
	if( !out ) die("required output file");
	out = path_resolve(out);

	if( src ){
		code = load_code(dinclude, countdi, src);
		if( verbose ) fprintf(stderr, "<CODE>%s</CODE>\n", code);
		asmchip32_s* ac = chip32_compiler_new();
		if( chip32_compiler_build(ac, code) ){
			chip32_compiler_error_print(ac);
			chip32_compiler_free(ac);
			return 1;
		}
		if( verbose ){
			chip32_lex_dump(ac);
			size_t sz;
			chip32Command_s* cmds = chip32_commands(ac, &sz);
			chip32_command_dump(cmds, sz);
		}
		FILE* fout = fopen(out, "w");
		if( verbose ) fprintf(stderr, "save command: %s\n", out);
		if( !fout ) die("error:%s on open file %s", strerror(errno), out);
		if( chip32_commands_save(ac, fout) ) die("error: on save commands");
		
		for( size_t i = 0; i < countdi; ++i ){
			free(dinclude[i]);
		}
		free(src);
		free(code);
		fclose(fout);
		chip32_compiler_free(ac);
	}
	
	if( !run || !out ) return 0;

	FILE* fout = fopen(out, "r");
	if( !fout ) die("error:%s on open file %s", strerror(errno), out);

	chip32_s* chip = chip32_new(512, page_load, fout);
	if( chip32_run(chip) ){
		die("mc32 fail");
	}

	free(out);
	chip32_free(chip);
	fclose(fout);
	return 0;
WRONGA:
	fputs("wrong numbers of arguments\n",stderr);
	return 1;
}

