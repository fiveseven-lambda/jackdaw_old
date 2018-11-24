#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "read.h"

#define Word_len 16

void *fileopen(char filename[]){
	return fopen(filename, "r");
}

extern unsigned int samplerate;

extern struct Note score[];
extern int size;

extern double tonic;

char readin(void *fp){
	char command[Word_len], arg[100000];
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
		while((arg[i] = getc(fp)) != a) ++i;
		arg[i] = '\0';
		if(!strcmp(command, "comment")){
		}else if(!strcmp(command, "message")){
			fputs(arg, stdout);
			if(a == '\n') putchar('\n');
		}else if(!strcmp(command, "system")){
			system(arg);
		}else if(!strcmp(command, "score")){
			char b = 0;
			double tmp;
			double stack[10] = {0};
			double cursor;
			int n = 1;
			for(int j = 0;; ++j){
				switch(arg[j]){
					case '\0':
						score[size].end = (score[size].start = cursor) + tmp;
						++size;
						return 1;
					case '[':
						stack[n++] = cursor;
						break;
					case ']':
						cursor = stack[--n];
						break;
					case '<':
						score[size].velocity = tmp;
						b = 0;
						tmp = 0;
						break;
					case '/':
						score[size].height = tmp;
						b = 0;
						tmp = 0;
						break;
					case '>':
						score[size].height /= tmp;
						b = 0;
						tmp = 0;
						break;
					case ':':
						score[size].velocity = tmp;
						tmp = b = 0;
						break;
					case '.':
						b = 1;
						break;
					case ',':
					case '\n':
						cursor = score[size].end = (score[size].start = cursor) + tmp;
						++size;
						tmp = b = 0;
						break;
					case '|':
						score[size].end = (score[size].start = cursor) + tmp;
						++size;
						cursor = stack[n - 1];
						tmp = b = 0;
						break;
					default:
						if(b){
							tmp += (arg[j] - '0') * pow(10, -(b++));
						}else{
							tmp = tmp * 10 + (arg[j] - '0');
						}
						break;
				}
			}
		}else{
			char name[Word_len];
			i = 0;
			while(isspace(name[0] = arg[i])) ++i;
			int j = 1;
			while(isalpha(name[j] = arg[i + j])) ++j;
			name[j] = '\0';
			i += j;
			while(arg[i] != '=') ++i;
			char *val = arg + i + 1;
			if(!strcmp(command, "set")){
				if(!strcmp(name, "samplerate")) sscanf(val, "%u", &samplerate);
				else if(!strcmp(name, "tonic")) sscanf(val, "%lf", &tonic);
			}
		}
		return 1;
	}
}
