#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include "read.h"

unsigned int samplerate = 44100;
#define channel 2
#define bitdepth 16

struct Note score[10];
int size;

double tonic = 440;

void write2bit(int fd, unsigned short data){ write(fd, &data, 2); }
void write4bit(int fd, unsigned int data){ write(fd, &data, 4); }

double sine(double, double);
double (*sound[10])(double, double) = {sine};

void writeout(int);

int main(int argc, char *argv[]){
	char *out_filename = NULL;

	int opt;
	while((opt = getopt(argc, argv, "o:")) != -1){
		switch(opt){
		case 'o':
			out_filename = optarg;
			break;
		}
	}

	if(!out_filename) out_filename = "a.wav";

	if(optind >= argc){
		score[0].instrument = 0;
		score[0].velocity = 1;
		score[0].height = 1;
		score[0].start = 0;
		score[0].end = 3;
		size = 1;
	}else{
		void *in = fileopen(argv[optind]);
		while(readin(in));
	}

	int out = creat(out_filename, S_IWUSR | S_IRUSR);
	writeout(out);
	close(out);
}


void writeout(int fd){
	static signed short buffer[100000000][2];
	int buffersize = 0;

	for(int i = 0; i < size; ++i){
		int j;
		for(j = score[i].start * samplerate; j < score[i].end * samplerate; ++j){
			signed short tmp = sound[score[i].instrument]((double)j / samplerate * score[i].height, ((double)j / samplerate - score[i].start) / (score[i].end - score[i].start)) * pow(2, 15) * score[i].velocity;
			buffer[j][0] += tmp;
			buffer[j][1] += tmp;
		}
		if(buffersize < j) buffersize = j;
	}

	write(fd, "RIFF", 4);
	write4bit(fd, buffersize * bitdepth / 8 * 2 + 36);
	write(fd, "WAVEfmt ", 8);
	write4bit(fd, 16);
	write2bit(fd, 1);
	write2bit(fd, channel);
	write4bit(fd, samplerate);
	write4bit(fd, samplerate * bitdepth / 8 * channel);
	write2bit(fd, channel * bitdepth / 8);
	write2bit(fd, bitdepth);
	write(fd, "data", 4);
	write4bit(fd, buffersize * bitdepth / 8 * 2);
	write(fd, buffer, buffersize * bitdepth / 8 * 2);
}

double sine(double t, double T){
	return sin(t * tonic * 2 * M_PI) * (T < .01 ? T * 100 : 1) * (1 - T);
}
