#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "read.h"

void *fileopen(char filename[]){
	return fopen(filename, "r");
}

extern unsigned int samplerate;
extern struct Note score[];
extern int size;

char str[64][2][65536];
int nstr;

char readin(void *fp){
	static char command[8], arg[65536];
	while(isspace(command[0] = getc(fp)));
	if(command[0] == EOF){
		fclose(fp);
		return 0;
	}else{
		int i = 1;
		while(isalpha(command[i] = getc(fp))) ++i;
		arg[0] = command[i];
		command[i] = '\0';
		while(isspace(arg[0])) arg[0] = getc(fp);
		char a = (i = (arg[0] != '{')) ? '\n' : '}';
		while((arg[i] = getc(fp)) != a){
			if(arg[i] == '{'){
				char tmp[128];
				for(int j = 0; (tmp[j] = getc(fp)) != '}' || (tmp[j] = '\0'); ++j);
				for(int j = 0; j < nstr; ++j) if(!strcmp(tmp, str[j][0])){
					for(int k = 0; (arg[i + k] = str[j][1][k]) || !(i += k); ++k);
					break;
				}
			}else ++i;
		}
		arg[i] = '\0';
		if(!strcmp(command, "comment")){
		}else if(!strcmp(command, "message")){
			fputs(arg, stdout);
			if(a == '\n') putchar('\n');
		}else if(!strcmp(command, "system")){
			system(arg);
		}else if(!strcmp(command, "import")){
			FILE *header = fopen(arg, "r");
			while(readin(header));
		}else if(!strcmp(command, "score")){
			static unsigned int instrument = 0;
			static double tempo = 60, velocity = 1, tonic = 440;
			static double end = 0;
			double start = end, cursor = start;
			enum {nothing, velocity_specified, colon} flag;
			char notempty = 0;
			unsigned int utmp;
			double dtmp = 0, decimal = 0;
			static double rhythm[64] = {0};
			static unsigned beat = 0, repeat = 1;
			for(int j = 0;; ++j){
				switch(arg[j]){
					case '(':
						++j;
						while(isspace(arg[j])) ++j;
						int k = 0;
						while(isalpha(arg[j + k])) ++k;
						arg[j + k] = '\0';
						k += j + 1;
						int l = 0;
						while(arg[k + l] != ')') ++l;
						if(!strcmp(arg + j, "tonic")) sscanf(arg + k, "%lf", &tonic);
						else if(!strcmp(arg + j, "tempo")) sscanf(arg + k, "%lf", &tempo);
						else if(!strcmp(arg + j, "velocity")) sscanf(arg + k, "%lf", &velocity);
						else if(!strcmp(arg + j, "rhythm")){
							beat = repeat = 0;
							for(int m = 0, n = 0, o = 0; !repeat; ++o){
								while(arg[k + n] != ','){
									if(arg[k + n] == ')'){
										repeat = o + 1;
										break;
									}
									++n;
								}
								arg[k + n] = '\0';
								sscanf(arg + k + m, "%lf", rhythm + o);
								m = n+++1;
							}
						}else if(!strcmp(arg + j, "modulate")){
							unsigned utmp1, utmp2;
							sscanf(arg + k, "%u/%u", &utmp1, &utmp2);
							tonic = tonic * utmp1 / utmp2;
						}
						j = k + l;
						break;
					case '_': break;
					case '#':
						++j;
						a = (arg[j] == '#' ? '\n' : '#');
						while(arg[j] != a) ++j;
						break;
					case ',':
					case '/':
					case '|':
					case '\0':
					case '\n':
						if(notempty){
							score[size].start = cursor;
							score[size].end = cursor += (flag == colon ? dtmp : rhythm[beat++ % repeat]) * 60 / tempo;
							if(!score[size].frequency) score[size].frequency = tonic;
							if(flag != colon) score[size].velocity = velocity * (flag == velocity_specified ? dtmp : 1);
							score[size].instrument = instrument;
							if(arg[j] == '/' || arg[j] == '|' || arg[j] == '\0') if(end < cursor) end = cursor;
							if(arg[j] == '|') start = end;
							if(arg[j] == '/' || arg[j] == '|') cursor = start;
							++size;
						}
						if(arg[j] == '\0') return 1;
						notempty = 0;
						flag = nothing;
						decimal = 0;
						dtmp = 0;
						break;
					case '<':
						score[size].frequency = tonic;
						++j;
						utmp = 0;
						for(; arg[j] != '/'; ++j) if('0' <= arg[j] && arg[j] <= '9') utmp = utmp * 10 + (arg[j] - '0');
						score[size].frequency *= utmp;
						utmp = 0;
						for(; arg[j] != '>'; ++j) if('0' <= arg[j] && arg[j] <= '9') utmp = utmp * 10 + (arg[j] - '0');
						score[size].frequency /= utmp;
						dtmp = 0;
						notempty = 1;
						break;
					case ':':
						score[size].velocity = velocity * (flag == velocity_specified ? dtmp : 1);
						flag = colon;
						dtmp = 0;
						decimal = 0;
						break;
					case '.':
						decimal = 1;
						break;
					case '+':
						if(flag == colon) dtmp = (end - cursor) * tempo / 60;
						break;
					default:
						if('0' <= arg[j] && arg[j] <= '9'){
							if(decimal) dtmp += (decimal /= 10) * (arg[j] - '0');
							else dtmp = dtmp * 10 + (arg[j] - '0');
							if(flag == nothing) flag = velocity_specified;
							notempty = 1;
						}
				}
			}
		}/*else if(!strcmp(command, "debug")){
			if(!strcmp(arg, "print score")){
				for(int j = 0; j < size; ++j){
					printf("%d [%lf, %lf] vel=%lf freq=%lf\n", j, score[j].start, score[j].end, score[j].velocity, score[j].frequency);
				}
			}
		}*/else{
			char name[11];
			i = 0;
			while(isspace(name[0] = arg[i])) ++i;
			for(int j = 0; isalpha(name[j] = arg[i + j]) || ((name[j] = '\0') && (i += j)); ++j);
			while(arg[i] != '=') ++i;
			char *val = arg + i + 1;
			if(!strcmp(command, "set")){
				if(!strcmp(name, "samplerate")) sscanf(val, "%u", &samplerate);
			}else if(!strcmp(command, "define")){
				strcpy(str[nstr][0], name);
				strcpy(str[nstr][1], val);
				++nstr;
			}
		}
		return 1;
	}
}
