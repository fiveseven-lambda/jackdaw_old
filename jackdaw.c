#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "read.h"

unsigned int samplerate = 44100;
#define channel 2
#define bitdepth 16

struct Note score[65536];
int size;
double end;

void writeout(int);

double sine(double t, double T, double f){
	return sin(2 * M_PI * f * t) * (1 - t/T);
}

int main(int argc, char *argv[]){
	char *out_filename = NULL;

	int opt;
	while((opt = getopt(argc, argv, "vho:")) != -1){
		switch(opt){
		case 'v':
			write(1, "jackdaw ver2.0\n", 15);
			return 0;
		case 'o':
			out_filename = optarg;
			break;
		case 'h':
		default:
			write(1, "usage: jackdaw [-v : version] [-h : show this help] [-o <file> : output into <file>]\n", 85);
			return 0;
		}
	}

	if(!out_filename) out_filename = "a.wav";

	if(optind >= argc){
		score[0].instrument = 0;
		score[0].velocity = 1;
		score[0].frequency = 440;
		score[0].start = 0;
		score[0].end = 3;
		size = 1;
	}else{
		void *in = fileopen(argv[optind]);
		while(readin(in));
	}
	
	int out = open(out_filename, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);
	int mapsize = end * samplerate * bitdepth / 8 * channel + 44;
	lseek(out, mapsize - 1, SEEK_SET);
	char c = 0;
	write(out, &c, 1);
	lseek(out, 0, SEEK_SET);
	signed short *map = mmap(NULL, mapsize, PROT_WRITE, MAP_SHARED, out, 0);
	((char *)map)[0] = 'R';
	((char *)map)[1] = 'I';
	((char *)map)[2] = 'F';
	((char *)map)[3] = 'F';
	((unsigned int *)map)[1] = mapsize - 8;
	((char *)map)[8] = 'W';
	((char *)map)[9] = 'A';
	((char *)map)[10] = 'V';
	((char *)map)[11] = 'E';
	((char *)map)[12] = 'f';
	((char *)map)[13] = 'm';
	((char *)map)[14] = 't';
	((char *)map)[15] = ' ';
	((unsigned int *)map)[4] = 16;
	((unsigned short *)map)[10] = 1;
	((unsigned short *)map)[11] = channel;
	((unsigned int *)map)[6] = samplerate;
	((unsigned int *)map)[7] = samplerate * bitdepth / 8 * channel;
	((unsigned short *)map)[16] = channel * bitdepth / 8;
	((unsigned short *)map)[17] = bitdepth;
	((char *)map)[36] = 'd';
	((char *)map)[37] = 'a';
	((char *)map)[38] = 't';
	((char *)map)[39] = 'a';
	((unsigned int *)map)[10] = mapsize - 44;
	memset(map + 44, 0, mapsize - 44);
	for(int i = 0; i < size; ++i){
		for(int j = score[i].start * samplerate; j < score[i].end * samplerate; ++j){
			signed short tmp = sine((double)j / samplerate - score[i].start, score[i].end - score[i].start, score[i].frequency) * score[i].velocity * (1 << 15);
			map[j * 2 + 44] += tmp;
			map[j * 2 + 1 + 44] += tmp;
		}
	}
	close(out);
	munmap(map, mapsize);
}

